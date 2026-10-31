// MeshCator adapter for the pinned MIT SX1262 port. All callbacks run on
// the companion loop task; LVGL is never called from an ISR or radio callback.
#include "MyMesh.h"
#include "TPagerLVGL.h"
#include "TPagerST7796Display.h"
#include "UITask.h"
#include "core/App.h"
#include "core/PowerState.h"
#include "core/MessageLog.h"
#include "core/NodePath.h"
#include "core/BackGesture.h"
#include "core/InputHaptics.h"
#include "core/MapHome.h"
#include "ui/View.h"
#include <Preferences.h>
#include <SD.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Adafruit_DRV2605.h>
#include "maps/BmpTile.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iomanip>
extern LGFX_TPager tpager_lgfx_panel;
extern "C" {
void variant_loop();
bool meshcator_haptic_power(bool);
void ui_apply_radio_changes();
bool meshcator_configure_ble(bool, const char *, uint32_t);
int tpager_battery_percent();
int tpager_charge_status();
int ui_get_sat_count();
bool ui_get_gps_valid();
int ui_get_noise_floor();
void ui_get_self_loc(double *, double *);
uint32_t ui_get_now_epoch();
bool ui_send_group_text(int, const char *);
bool ui_send_dm(const uint8_t *, const char *);
bool ui_send_self_advert(bool);
int ui_get_dm_msg_count(const uint8_t *);
struct UiDmMsg {
  char text[120];
  uint8_t from_me;
  uint32_t age_s;
};
bool ui_get_dm_msg(const uint8_t *, int, UiDmMsg *);
}
namespace {
std::string hex(const uint8_t *key) {
  static const char *a = "0123456789abcdef";
  std::string s;
  for (int i = 0; i < 32; ++i) {
    s += a[key[i] >> 4];
    s += a[key[i] & 15];
  }
  return s;
}
bool unhex(const std::string &s, uint8_t *key) {
  if (s.size() != 64)
    return false;
  auto n = [](char c) {
    return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
  };
  for (int i = 0; i < 32; ++i) {
    int a = n(s[i * 2]), b = n(s[i * 2 + 1]);
    if (a < 0 || b < 0)
      return false;
    key[i] = (a << 4) | b;
  }
  return true;
}
std::string channelNotificationKey(const ChannelDetails&channel){
 uint8_t digest[32];mesh::Utils::sha256(digest,32,channel.channel.secret,32);return "channel-key:"+hex(digest);
}
extern mc::App app;
class PagerBackend : public mc::Backend {
  Adafruit_DRV2605 haptic_;
  bool hapticReady_=false;
  uint32_t hapticAttempt_=0;
  mc::InputHapticGate inputHapticGate_;
  uint32_t lastAlert_=0,lastAck_=0,lastTimeout_=0,lastSentTime_=0;
public:
  void beginAlerts(){
    hapticAttempt_=millis();hapticReady_=false;
    if(!meshcator_haptic_power(true))return;
    delay(10);
    hapticReady_=haptic_.begin(&Wire);
    if(hapticReady_){haptic_.useERM();haptic_.selectLibrary(1);haptic_.setMode(DRV2605_MODE_INTTRIG);haptic_.stop();}
  }
  void inputFeedback(mc::InputPulse pulse) {
    if(!hapticReady_&&uint32_t(millis()-hapticAttempt_)>=5000)beginAlerts();
    if(!hapticReady_||!inputHapticGate_.allow(pulse,millis()))return;
    haptic_.stop();haptic_.setMode(DRV2605_MODE_INTTRIG);
    // Built-in full-strength effects: strong click / distinct double click.
    haptic_.setWaveform(0,pulse==mc::InputPulse::Confirm?10:17);
    haptic_.setWaveform(1,0);haptic_.go();
  }
  std::string hapticDiagnostic(bool test){
    if(test){beginAlerts();if(hapticReady_){haptic_.setWaveform(0,15);haptic_.setWaveform(1,0);haptic_.go();}}
    char text[160];
    Wire.beginTransmission(0x5a);bool present=Wire.endTransmission()==0;
    snprintf(text,sizeof text,"ready=%u ack=%u status=%02x mode=%02x library=%02x go=%02x rated=%02x clamp=%02x feedback=%02x control3=%02x",
      hapticReady_,present,present?haptic_.readRegister8(0):0,present?haptic_.readRegister8(1):0,present?haptic_.readRegister8(3):0,present?haptic_.readRegister8(12):0,present?haptic_.readRegister8(22):0,present?haptic_.readRegister8(23):0,present?haptic_.readRegister8(26):0,present?haptic_.readRegister8(29):0);
    return text;
  }
  bool displayAwake() const override {return !PowerManager::instance().isScreenOff();}
  void notifyReceived(bool sound,bool vibrate,bool wake,bool channel) override {
    if(wake)PowerManager::instance().onActivity();
    auto now=millis();if(lastAlert_&&uint32_t(now-lastAlert_)<3000)return;lastAlert_=now;
    if(sound)AudioManager::instance().notify(channel?SoundEvent::MSG_CHANNEL:SoundEvent::MSG_DM);
    if(vibrate&&hapticReady_){inputHapticGate_.notification(now);haptic_.setWaveform(0,16);haptic_.setWaveform(1,0);haptic_.go();}
  }
  std::string channelKey() override {uint8_t bytes[16];esp_fill_random(bytes,sizeof bytes);static const char digits[]="0123456789abcdef";std::string value;for(auto b:bytes){value+=digits[b>>4];value+=digits[b&15];}return value;}
  std::string createChannel(const std::string &name,const std::string &key,bool privateChannel) override {
    if(name.empty()||name.size()>30||!mc::validText(name))return {};
    std::string title=privateChannel?name:name[0]=='#'?name:"#"+name;
    ChannelDetails existing;int slot=-1;
    for(int i=0;i<MAX_GROUP_CHANNELS;++i)if(the_mesh.getChannel(i,existing)){
      if(title==existing.name){std::string n,k;if(!channelInfo("channel:"+std::to_string(i),n,k))return {};std::string expected=key;if(!privateChannel){uint8_t digest[32]={};mesh::Utils::sha256(digest,16,reinterpret_cast<const uint8_t*>(title.data()),title.size());expected=hex(digest).substr(0,32);}if(k==expected)return "channel:"+std::to_string(i);return {};}
      if(!existing.name[0]&&slot<0)slot=i;
    }
    if(slot<0)return {};
    ChannelDetails channel{};std::strncpy(channel.name,title.c_str(),31);
    if(privateChannel){
      if((key.size()!=32&&key.size()!=64)||key.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)return {};
      for(size_t i=0;i<key.size()/2;++i)channel.channel.secret[i]=std::strtoul(key.substr(i*2,2).c_str(),nullptr,16);
    }else mesh::Utils::sha256(channel.channel.secret,16,reinterpret_cast<const uint8_t*>(title.data()),title.size());
    if(!the_mesh.setChannel(slot,channel))return {};
    the_mesh.uiSaveChannels();return "channel:"+std::to_string(slot);
  }
  bool channelInfo(const std::string&id,std::string&name,std::string&key) override {
    if(id.rfind("channel:",0)!=0)return false;ChannelDetails c;if(!the_mesh.getChannel(std::atoi(id.c_str()+8),c)||!c.name[0])return false;name=c.name;key=hex(c.channel.secret);if(key.substr(32)==std::string(32,'0'))key.resize(32);return true;
  }
  bool updateChannel(const std::string&id,const std::string&name,const std::string&key,bool priv) override {
    std::string old,k;if(!channelInfo(id,old,k)||name.empty()||name.size()>30||!mc::validText(name))return false;
    ChannelDetails c{};auto title=priv?name:name[0]=='#'?name:"#"+name;std::strncpy(c.name,title.c_str(),31);
    if(priv){if((key.size()!=32&&key.size()!=64)||key.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)return false;for(size_t i=0;i<key.size()/2;++i)c.channel.secret[i]=std::strtoul(key.substr(i*2,2).c_str(),nullptr,16);}else mesh::Utils::sha256(c.channel.secret,16,reinterpret_cast<const uint8_t*>(title.data()),title.size());
    if(!the_mesh.setChannel(std::atoi(id.c_str()+8),c))return false;the_mesh.uiSaveChannels();return true;
  }
  bool removeEntry(const std::string&id,bool channel) override {
    if(channel){if(id.rfind("channel:",0)!=0)return false;ChannelDetails blank{};if(!the_mesh.setChannel(std::atoi(id.c_str()+8),blank))return false;the_mesh.uiSaveChannels();return true;}
    uint8_t key[32];if(!unhex(id,key))return false;auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return false;the_mesh.uiClearAcks();if(!the_mesh.removeContact(*c))return false;the_mesh.uiSaveContacts();return true;
  }
  bool renameContact(const std::string&id,const std::string&name) override {
    uint8_t key[32];if(!unhex(id,key)||!mc::validText(name)||name.size()>31)return false;auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return false;std::strncpy(c->name,name.c_str(),sizeof(c->name)-1);c->name[sizeof(c->name)-1]=0;the_mesh.uiSaveContacts();return true;
  }
  bool shareContact(const std::string&id) override {uint8_t key[32];if(!unhex(id,key))return false;auto c=the_mesh.lookupContactByPubKey(key,32);return c&&the_mesh.shareContactZeroHop(*c);}
  std::string exportEntry(const std::string&id,bool channel) override {
    if(channel){std::string n,k;if(!channelInfo(id,n,k))return {};std::ostringstream o;o<<"MC-CHANNEL1 "<<std::quoted(n)<<' '<<k;return o.str();}
    uint8_t key[32],packet[256];if(!unhex(id,key))return {};auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return {};auto len=the_mesh.exportContact(*c,packet);if(!len){std::ostringstream o;o<<"MC-NODE1 "<<id<<' '<<std::quoted(c->name)<<' '<<unsigned(c->type)<<' '<<unsigned(c->flags)<<' '<<c->gps_lat<<' '<<c->gps_lon;return o.str();}std::string out="MC-CONTACT1 ";const char *digits="0123456789abcdef";for(unsigned i=0;i<len;++i){out+=digits[packet[i]>>4];out+=digits[packet[i]&15];}return out;
  }
  bool importEntry(const std::string&s) override {
    std::istringstream in(s);std::string tag,n,k,extra;in>>tag;
    if(tag=="MC-CHANNEL1"){if(!(in>>std::quoted(n)>>k)||in>>extra)return false;return !createChannel(n,k,true).empty();}
    if(tag=="MC-NODE1"){unsigned type,flags;int32_t lat,lon;uint8_t key[32];if(!(in>>k>>std::quoted(n)>>type>>flags>>lat>>lon)||in>>extra||!unhex(k,key)||!mc::validText(n)||n.size()>31||type<1||type>4||flags>255||lat< -90000000||lat>90000000||lon< -180000000||lon>180000000)return false;if(!the_mesh.uiAddContact(key,n.c_str(),type,flags&1,lat,lon))return false;auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return false;std::strncpy(c->name,n.c_str(),31);c->name[31]=0;c->type=type;c->flags=flags;c->gps_lat=lat;c->gps_lon=lon;the_mesh.uiSaveContacts();return true;}
    if(tag!="MC-CONTACT1"||!(in>>k)||in>>extra||k.size()>510||k.size()<194||k.size()%2||k.find_first_not_of("0123456789abcdefABCDEF")!=std::string::npos)return false;
    uint8_t bytes[255];for(size_t i=0;i<k.size()/2;++i)bytes[i]=std::strtoul(k.substr(i*2,2).c_str(),nullptr,16);if(!the_mesh.importContact(bytes,k.size()/2))return false;the_mesh.uiSaveContacts();return true;
  }
  bool setPath(const std::string&id,const std::string&path) override {
    uint8_t key[32],encoded;std::vector<uint8_t> bytes;if(!unhex(id,key)||!mc::parseNodePath(path,encoded,bytes))return false;
    auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return false;
    std::copy(bytes.begin(),bytes.end(),c->out_path);c->out_path_len=encoded;the_mesh.uiSaveContacts();return true;
  }
  int telemetryPolicy()const override {auto p=the_mesh.getNodePrefs();return p->telemetry_mode_base|(p->telemetry_mode_loc<<2)|(p->telemetry_mode_env<<4);}
  bool useContactPermissions() override {auto p=the_mesh.getNodePrefs();p->telemetry_mode_base=p->telemetry_mode_loc=p->telemetry_mode_env=TELEM_MODE_ALLOW_FLAGS;the_mesh.savePrefs();return true;}
  bool setPermissions(const std::string&id,uint8_t bits) override {
    uint8_t key[32];if(!unhex(id,key)||bits>7)return false;auto c=the_mesh.lookupContactByPubKey(key,32);if(!c)return false;
    c->flags=(c->flags&~14)|(bits<<1);the_mesh.uiSaveContacts();return true;
  }
  bool remote(const std::string&id,int op,const std::string&text) override {
    uint8_t key[32];if(!unhex(id,key))return false;
    if(op>=4)return the_mesh.uiNodeQuery(key,op);
    if(op==0)return the_mesh.uiSendLogin(key,text.c_str());
    if(op==1)return the_mesh.uiSendBinaryReq(key,7,nullptr,0,0);
    if(op==2)return the_mesh.uiSendBinaryReq(key,1,nullptr,0,2);
    auto c=the_mesh.lookupContactByPubKey(key,32);uint32_t timeout=0;if(!c)return false;int r=the_mesh.sendCommandData(*c,ui_get_now_epoch(),0,text.c_str(),timeout);return r==MSG_SEND_SENT_DIRECT||r==MSG_SEND_SENT_FLOOD;
  }
  bool mapHome(mc::Position& pos,int& zoom) override {
    if(!sd_init())return false;
    auto f=SD.open("/meshcator/map-home.txt",FILE_READ);
    if(!f||f.size()==0||f.size()>96)return false;
    char text[97]={};size_t n=f.size();if(f.read(reinterpret_cast<uint8_t*>(text),n)!=n)return false;
    return mc::decodeMapHome(text,pos,zoom);
  }
  bool readTile(int z,int x,int y,std::vector<uint16_t>&pixels) override {
    // Map access must mount SD independently of message logging or USB maintenance.
    if(!sd_init())return false;
    std::string path="/meshcator/tiles/"+std::to_string(z)+"/"+std::to_string(x)+"/"+std::to_string(y)+".bmp";
    auto f=SD.open(path.c_str(),FILE_READ);if(!f)return false;
    return mc::readBmpTile([&](uint32_t off,uint8_t*data,size_t len){return (f.position()==off||f.seek(off))&&f.read(data,len)==len;},pixels);
  }
  mc::SendResult sendTracked(const mc::Chat &c,const std::string&s) override {lastAck_=lastTimeout_=0;bool ok=send(c,s);return {ok,lastAck_,lastTimeout_};}
  bool appendMessageLog(const mc::Chat& c,const mc::Message& m) override {
    // No card: the internal history remains available. A mounted but unwritable card reports failure.
    if(!sd_init())return true;
    auto f=SD.open("/meshcator/messages.tsv",FILE_APPEND);if(!f)return false;
    auto record=mc::messageLogRecord(c,m);
    bool ok=f.write(reinterpret_cast<const uint8_t*>(record.data()),record.size())==record.size();f.flush();f.close();return ok;
  }
  bool writeArchive(const std::string&s) override {
    auto f=SPIFFS.open("/mc-history.tmp",FILE_WRITE);if(!f)return false;
    bool ok=f.write(reinterpret_cast<const uint8_t*>(s.data()),s.size())==s.size();f.flush();f.close();if(!ok)return false;
    if(SPIFFS.exists("/mc-history")){SPIFFS.remove("/mc-history.bak");if(!SPIFFS.rename("/mc-history","/mc-history.bak"))return false;}
    return SPIFFS.rename("/mc-history.tmp","/mc-history");
  }
  std::string readArchive() override {
    bool found=false;for(auto path:{"/mc-history","/mc-history.bak"}){auto f=SPIFFS.open(path,FILE_READ);if(!f)continue;found=true;if(f.size()>mc::MaxArchiveBytes||f.size()==0)continue;std::string s(f.size(),'\0'),body;bool ok=f.read(reinterpret_cast<uint8_t*>(&s[0]),s.size())==s.size();f.close();if(ok&&mc::unsealArchive(s,body))return s;}return found?"CORRUPT":"";
  }
  bool send(const mc::Chat &c, const std::string &text) override {
    if (c.channel) {
      // BaseChatMesh otherwise silently truncates the sender-prefixed text.
      if (text.size() + std::strlen(the_mesh.getNodeName()) + 2 > MAX_TEXT_LEN)
        return false;
      if (c.id.rfind("channel:", 0) != 0)
        return false;
      int index = std::atoi(c.id.c_str() + 8);
      ChannelDetails current;if(!the_mesh.getChannel(index,current)||!current.name[0])return false;
      return ui_send_group_text(index, text.c_str());
    }
    uint8_t key[32];
    if (!unhex(c.id, key))
      return false;
    auto contact = the_mesh.lookupContactByPubKey(key, 32);
    if (!contact)
      return false;
    uint32_t ack = 0, timeout = 0;
    const int result =
        the_mesh.sendMessage(*contact, lastSentTime_=std::max(ui_get_now_epoch(),lastSentTime_+1), 0, text.c_str(), ack, timeout);
    if(result==MSG_SEND_SENT_DIRECT||result==MSG_SEND_SENT_FLOOD){lastAck_=ack;lastTimeout_=std::max(uint32_t(1000),timeout);the_mesh.uiTrackAck(contact,ack);}
    // The upstream UI helper treats zero (MSG_SEND_FAILED) as success.
    // Use the protocol return codes directly instead.
    return result == MSG_SEND_SENT_DIRECT || result == MSG_SEND_SENT_FLOOD;
  }
  bool advert(bool flood) override {
    return ui_send_self_advert(flood);
  }
  bool toggleFavorite(const std::string &id) override {
    uint8_t key[32];
    if (!unhex(id, key) || !the_mesh.uiToggleFavorite(key)) return false;
    the_mesh.uiSaveContacts();
    return true;
  }
  bool resetPath(const std::string &id) override {
    uint8_t key[32];
    if (!unhex(id, key)) return false;
    auto contact = the_mesh.lookupContactByPubKey(key, 32);
    if (!contact) return false;
    contact->out_path_len = OUT_PATH_UNKNOWN;
    the_mesh.uiSaveContacts();
    return true;
  }
  bool apply(const mc::Settings &s) {
    AudioManager::instance().setEnabled(!s.notificationMute);
    AudioManager::instance().setVolume(s.notificationVolume);
    if(s.notificationMute&&hapticReady_)haptic_.stop();
    auto p = the_mesh.getNodePrefs();
    bool radioChanged=p->freq!=float(s.frequency)||p->bw!=float(s.bandwidth)||p->sf!=s.sf||p->cr!=s.cr||p->tx_power_dbm!=s.power;
    bool gpsChanged=bool(p->gps_enabled)!=s.gps;
    uint32_t pin=std::strtoul(s.bluetoothPIN.c_str(),nullptr,10);
    bool prefsChanged=radioChanged||gpsChanged||p->path_hash_mode!=s.hashBytes-1||p->isRepeatEn()!=s.repeat||p->advert_loc_policy!=(s.shareLocation?1:0)||p->ble_pin!=pin||s.name!=p->node_name;
    p->freq=s.frequency; p->bw=s.bandwidth; p->sf=s.sf; p->cr=s.cr;
    p->tx_power_dbm=s.power; p->path_hash_mode=s.hashBytes-1;
    p->setRepeatEn(s.repeat); p->gps_enabled=s.gps;
    p->advert_loc_policy=s.shareLocation?1:0; p->ble_pin=pin;
    std::strncpy(p->node_name,s.name.c_str(),sizeof(p->node_name)-1);p->node_name[sizeof(p->node_name)-1]=0;
    if(gpsChanged)the_mesh.applyGpsPrefs();
    if(radioChanged)ui_apply_radio_changes();else if(prefsChanged)the_mesh.savePrefs();
    if(PowerManager::instance().isGpsEnabled()!=s.gps)PowerManager::instance().setGpsEnabled(s.gps);
    PowerManager::instance().setTargetBrightness(s.brightness*255/100);
    PowerManager::instance().setScreenTimeout(s.timeout);
    return meshcator_configure_ble(s.bluetooth,s.bluetoothName.c_str(),p->ble_pin);
  }
  bool saveSettings(const mc::Settings &s) override {
    auto previous=app.settings;
    Preferences p;if(!p.begin("meshcator",false))return false;
    if(!apply(s)){apply(previous);p.end();return false;}
    auto encoded=mc::encodeSettings(s);bool ok=p.putString("config",encoded.c_str())==encoded.size();
    if(ok){p.putUChar("language",static_cast<uint8_t>(s.language));p.putUChar("help_ver",s.helpVersion);p.putUChar("theme",static_cast<uint8_t>(s.theme));}
    p.end();if(!ok)apply(previous);return ok;
  }
};
PagerBackend backend;
mc::App app(backend);
mc::View view(app);
bool ready = false;
bool displayReady = false;
uint32_t splashStarted = 0;
uint32_t refreshed = 0;
volatile int lastEncoder = 3, ticks = 0;
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;
DRAM_ATTR int8_t quad[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
void IRAM_ATTR encoderEdge() {
  portENTER_CRITICAL_ISR(&encoderMux);
  int now = (digitalRead(PIN_ENCODER_A) << 1) | digitalRead(PIN_ENCODER_B);
  ticks += quad[(lastEncoder << 2) | now];
  lastEncoder = now;
  portEXIT_CRITICAL_ISR(&encoderMux);
}
mc::BackGesture keyboardBack;
bool keyboardBackDown = false;
bool sideDown = false, sideLongHandled = false;
uint32_t sidePressed = 0;
bool down = false, longHandled = false;
uint32_t pressed = 0;
// MeshCore owns radio identity/preferences, including updates made over USB/BLE.
void readMeshSettings(mc::Settings &s) {
  auto p=the_mesh.getNodePrefs();
  if(float(s.frequency)!=p->freq||float(s.bandwidth)!=p->bw||s.sf!=p->sf||s.cr!=p->cr)s.preset=-1;
  s.name=p->node_name;s.frequency=p->freq;s.bandwidth=p->bw;s.sf=p->sf;s.cr=p->cr;s.power=p->tx_power_dbm;
  s.hashBytes=p->path_hash_mode+1;s.repeat=p->isRepeatEn();s.gps=p->gps_enabled;s.shareLocation=p->advert_loc_policy!=0;
  char pin[7];snprintf(pin,sizeof pin,"%06u",unsigned((p->ble_pin?p->ble_pin:the_mesh.getBLEPin())%1000000));s.bluetoothPIN=pin;
}
void syncModel() {
  readMeshSettings(app.settings);
  if(PowerManager::instance().isGpsEnabled()!=app.settings.gps)PowerManager::instance().setGpsEnabled(app.settings.gps);
  meshcator_configure_ble(app.settings.bluetooth,app.settings.bluetoothName.c_str(),std::strtoul(app.settings.bluetoothPIN.c_str(),nullptr,10));
  app.status.battery = tpager_battery_percent();
  int charge=tpager_charge_status();app.status.usbPowered=mc::usbPowered(charge);app.status.charging=mc::batteryCharging(charge);
  app.status.sats = ui_get_sat_count();
  app.status.fix = ui_get_gps_valid();
  app.status.epoch = ui_get_now_epoch();
  ui_get_self_loc(&app.status.position.lat, &app.status.position.lon);
  app.status.position.valid = app.status.fix;
  app.nodes.clear();
  for (uint32_t i = 0; i < MAX_CONTACTS; ++i) {
    ContactInfo c;
    if (!the_mesh.getContactByIdx(i, c))
      break;
    if (!c.name[0])
      continue;
    app.nodes.push_back({hex(c.id.pub_key),
                         c.name,
                         {c.gps_lat / 1e6, c.gps_lon / 1e6, c.gps_lat != 0 || c.gps_lon != 0},
                         0,
                         false,
                         c.out_path_len == 0xFF ? -1 : c.out_path_len&63,
                         (c.flags & 1) != 0});
    auto &node=app.nodes.back();node.type=c.type;node.lastAdvert=c.last_advert_timestamp;node.permissions=(c.flags>>1)&7;
    node.path=mc::formatNodePath(c.out_path_len,c.out_path);
  }
  for (int i = 0; i < MAX_GROUP_CHANNELS; ++i) {
    ChannelDetails c;
    if (!the_mesh.getChannel(i, c) || !c.name[0]){app.bindChannel("channel:"+std::to_string(i),"","");continue;}
    auto id = "channel:" + std::to_string(i);
    app.bindChannel(id,c.name,channelNotificationKey(c));
  }
  app.sample(ui_get_noise_floor());
  app.changed();
}
void key(mc::Key k) {
  PowerManager::instance().onActivity();
  app.notice.clear();
  const auto before=mc::menuCursor(app);
  const bool locked=app.screen==mc::Screen::Lock;
  app.key(k);
  backend.inputFeedback(mc::inputPulse(k,before!=mc::menuCursor(app),locked));
}
void handleKeyboardBack(uint32_t now) {
  auto action = keyboardBack.update(keyboardBackDown, now);
  if (action == mc::BackGesture::Action::Back) key(mc::Key::Back);
  else if (action == mc::BackGesture::Action::Short) key(mc::Key::Delete);
}
} // namespace
extern "C" void ui_back_key(bool pressed) {
  if (!ready) return;
  keyboardBackDown = pressed;
  PowerManager::instance().onActivity();
  handleKeyboardBack(millis());
}
extern "C" void meshcator_show_splash() {
  Preferences prefs;
  if (prefs.begin("meshcator", true)) {
    auto language = prefs.getUChar("language", 0);
    if (language < 4) app.settings.language = static_cast<mc::Language>(language);
    prefs.end();
  }
  displayReady = tpager_lvgl_begin(&tpager_lgfx_panel);
  if (displayReady) {
    view.splash();
    lv_refr_now(nullptr);
    splashStarted = millis();
  }
}
extern "C" void ui_input_char(char c) {
  if (!ready)
    return;
  PowerManager::instance().onActivity();
  if (app.helpOpen || app.menuOpen || app.contextOpen) {
    if (c == '\r' || c == '\n') key(mc::Key::Enter);
    else if (c == '\b' || c == 27) key(mc::Key::Back);
    return;
  }
  if (c == '\r' || c == '\n')
    key(mc::Key::Enter);
  else if (c == 27)
    key(mc::Key::Back);
  else if (c == '\b' || c == 127) {
    key(mc::Key::Delete);
  } else if (c == '\t' || (c == ' ' && app.utility==mc::Utility::None && app.screen != mc::Screen::Conversation && !app.settingsEditing && !app.waypointNaming && !app.channelCreating))
    key(mc::Key::Menu);
  else
    app.type(std::string(1, c));

}
extern "C" void ui_on_channel_message(int index, uint32_t timestamp, const char *text) {
  ChannelDetails c;
  if (!the_mesh.getChannel(index, c))
    return;
  auto id="channel:"+std::to_string(index);
  if(!app.bindChannel(id,c.name,channelNotificationKey(c)))return;
  app.receive(id, c.name, text, true, timestamp);
}
extern "C" void meshcator_telemetry(const uint8_t*key,const uint8_t*data,unsigned len){
  std::ostringstream out;out<<"Telemetry\n";unsigned i=0;
  while(i+2<=len){unsigned channel=data[i++],type=data[i++];unsigned size=type==104?1:type==136?9:2;
    if(type!=103&&type!=104&&type!=115&&type!=116&&type!=136){out<<"Type "<<type<<" (raw): ";for(;i<len;++i){const char*d="0123456789abcdef";out<<d[data[i]>>4]<<d[data[i]&15];}break;}
    if(i+size>len){out<<"Invalid payload";break;}
    out<<channel<<": ";int v=size>=2?(data[i]<<8)|data[i+1]:data[i];
    if(type==103)out<<int16_t(v)/10.0<<" C";
    if(type==104)out<<v/2.0<<" %";
    if(type==115)out<<v/10.0<<" hPa";
    if(type==116)out<<v/100.0<<" V";
    if(type==136){for(unsigned j=0;j<9;j+=3){int32_t n=(data[i+j]<<16)|(data[i+j+1]<<8)|data[i+j+2];if(n&0x800000)n|=0xff000000;out<<n/(j==6?100.0:10000.0)<<" ";}}
    out<<"\n";i+=size;
  }
  app.remoteReply(hex(key),out.str());
}
extern "C" void meshcator_remote_reply(const uint8_t*key,const char*text){app.remoteReply(hex(key),text?text:"No response");}
extern "C" void ui_on_rptr_login_result(const uint8_t*key,bool ok){app.remoteReply(hex(key),mc::translate(app.settings.language,ok?"Anmeldung erfolgreich":"Anmeldung fehlgeschlagen"));}
extern "C" void ui_on_rptr_vars(const uint8_t*key,const char*text){meshcator_remote_reply(key,text);}
extern "C" void ui_on_rptr_stats(const uint8_t*key,const char*text){meshcator_remote_reply(key,text);}
extern "C" void meshcator_ack(const uint8_t *key,uint32_t ack){app.acknowledged(hex(key),ack);}
extern "C" void ui_meshcator_received_dm(const uint8_t *keyBytes, uint32_t timestamp, const char *text) {
  auto id = hex(keyBytes), name = id.substr(0, 8);
  for (auto &c : app.nodes)
    if (c.id == id) {
      name = c.name;
      break;
    }
  app.receive(id, name, text, false, timestamp);
}
void UITask::begin(DisplayDriver *d, SensorManager *s, NodePrefs *p) {
  _display = d;
  _sensors = s;
  _prefs = p;
  if (d)
    d->turnOn();
  Preferences prefs;
  if (prefs.begin("meshcator", true)) {
    if (prefs.getUInt("schema", 1) == 1) {
      app.settings.helpVersion = prefs.getUChar("help_ver", 0);
      auto language = prefs.getUChar("language", 0);
      if (language < 4) app.settings.language = static_cast<mc::Language>(language);
      auto theme = prefs.getUChar("theme", 0);
      if (theme < 4)
        app.settings.theme = static_cast<mc::Theme>(theme);
    }
    auto encoded=prefs.getString("config", "");
    if(!mc::decodeSettings(encoded.c_str(),app.settings)) {
      app.settings.name=p->node_name;
      app.settings.frequency=p->freq;app.settings.bandwidth=p->bw;
      app.settings.sf=p->sf;app.settings.cr=p->cr;app.settings.power=p->tx_power_dbm;
      app.settings.gps=p->gps_enabled;app.settings.shareLocation=p->advert_loc_policy!=0;
      app.settings.repeat=p->isRepeatEn();app.settings.hashBytes=p->path_hash_mode+1;
    }
    prefs.end();
  }
  readMeshSettings(app.settings);
  PowerManager::instance().setGpsEnabled(app.settings.gps); // also enforce disabled at boot
  backend.beginAlerts();
  backend.apply(app.settings);
  pinMode(PIN_USER_BTN, INPUT_PULLUP);
  pinMode(PIN_SIDE_BTN, INPUT_PULLUP);
  lastEncoder = (digitalRead(PIN_ENCODER_A) << 1) | digitalRead(PIN_ENCODER_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), encoderEdge, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), encoderEdge, CHANGE);
  ready = displayReady || tpager_lvgl_begin(&tpager_lgfx_panel);
  if (ready) {
    app.restoreArchive();
    syncModel();
    app.startIntroduction();
    if (millis() - splashStarted >= 1200) view.render();
  }
  Serial.printf("[MeshCator] LVGL UI %s; free heap=%u, PSRAM=%u\n",
                ready ? "ready" : "FAILED", (unsigned)ESP.getFreeHeap(),
                (unsigned)ESP.getFreePsram());
}
void UITask::loop() {
  if (!ready)
    return;
  app.tick(millis());
  variant_loop();
  PowerManager::instance().loop();
  AudioManager::instance().loop();
  portENTER_CRITICAL(&encoderMux);
  int steps = ticks / 4;
  ticks -= steps * 4;
  portEXIT_CRITICAL(&encoderMux);
  while (steps > 0) { key(mc::Key::Previous); --steps; }
  while (steps < 0) { key(mc::Key::Next); ++steps; }
  bool b = digitalRead(PIN_USER_BTN) == LOW;
  uint32_t time = millis();
  handleKeyboardBack(time);
  if (b && !down) {
    pressed = time;
    longHandled = false;
  }
  if (b && !longHandled && time - pressed > 650) {
    key(mc::Key::Menu);
    longHandled = true;
  }
  if (!b && down && !longHandled && time - pressed > 30)
    key(mc::Key::Confirm);
  down = b;
  bool side = digitalRead(PIN_SIDE_BTN) == LOW;
  if (side && !sideDown) { sidePressed = time; sideLongHandled = false; }
  if (side && !sideLongHandled && time - sidePressed > 2000) {
    key(mc::Key::Back);
    sideLongHandled = true;
  }
  if (!side && sideDown && !sideLongHandled && time - sidePressed >= 30)
    key(mc::Key::Back);
  sideDown = side;
  static bool wasAwake = true;
  bool awake=backend.displayAwake();
  if ((awake && !wasAwake) || time - refreshed >= (awake ? 1000u : 10000u)) {
    refreshed = time;
    syncModel();
  }
  if(awake){
    if(!wasAwake)app.changed();
    if (millis() - splashStarted >= 1200) view.render();
    lv_timer_handler();
  }else delay(1); // yield CPU; radio/BLE/USB and input still serviced every loop
  wasAwake=awake;
}
void UITask::showPreview(const char *from, const char *) {
  app.notice = mc::translate(app.settings.language, "Nachricht: ") + from;
  app.changed();
}
void UITask::trackLastMsg(const char *, const char *, uint8_t) {}
bool UITask::isChannelMuted(const char *name) {
  auto chat=std::find_if(app.chats.begin(),app.chats.end(),[&](const mc::Chat&c){return c.channel&&c.name==name;});
  return app.settings.notificationMute||(chat!=app.chats.end()&&app.notificationMode(chat->id)==0);
}

extern "C" bool meshcator_tx_enabled() { return app.settings.setupComplete; }

// Private maintenance extension. MultiSerialInterface accepts it only from USB
// and writes the reply ONLY to that physical interface, never to BLE.
extern "C" size_t meshcator_usb_request(const uint8_t *in,size_t len,uint8_t*out){
 static std::string resource,buffer,guard;static File file;static size_t offset=0;static uint32_t hash=2166136261u,last=0;static bool writing=false;
 out[0]=0x7e;out[1]='M';out[2]='C';out[3]=1;
 auto reset=[&](){file.close();buffer.clear();resource.clear();offset=0;writing=false;};
 if(millis()-last>30000)reset();last=millis();if(len<4)return 4;
 auto validSD=[](const std::string&r){return r.rfind("sd:",0)==0&&r.size()>3&&r.size()<120&&r.find("..") == std::string::npos&&r[3]!='/'&&r.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/_-.: ")==std::string::npos;};
 auto path=[&](){return std::string("/meshcator/")+resource.substr(3);};
 auto u32=[](const uint8_t*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);};
 auto ok=[&](){out[3]=0;return size_t(4);};
 if(in[3]==7||in[3]==8){auto info=backend.hapticDiagnostic(in[3]==8);std::memcpy(out+4,info.data(),info.size());out[3]=0;return 4+info.size();}
 if(in[3]==0){std::string info="MeshCator maintenance v1: settings history catalog entry sd:<path>";std::memcpy(out+4,info.data(),info.size());out[3]=0;return 4+info.size();}
 if(in[3]==1||in[3]==3){
  reset();resource.assign(reinterpret_cast<const char*>(in+4),len-4);if(resource.find('\0')!=std::string::npos)return 4;writing=in[3]==3;hash=2166136261u;guard=app.archive();
  bool sd=validSD(resource);
  if(resource!="settings"&&resource!="history"&&resource!="catalog"&&resource!="entry"&&!sd){reset();return 4;}
  if(sd){if(!sd_init())return 4;auto target=path();if(writing){size_t p=target.find('/',1);while(p!=std::string::npos){SD.mkdir(target.substr(0,p).c_str());p=target.find('/',p+1);}SD.remove((target+".upload").c_str());}file=SD.open((target+(writing?".upload":"")).c_str(),writing?FILE_WRITE:FILE_READ);if(!file){reset();return 4;}}
  else if(!writing){
   if(resource=="settings"){readMeshSettings(app.settings);buffer=mc::encodeSettings(app.settings);}
   else if(resource=="history"){app.saveArchive();buffer=app.archive();}
   else if(resource=="catalog"){for(auto&c:app.chats)if(c.channel){auto e=backend.exportEntry(c.id,true);if(!e.empty())buffer+=e+'\n';}for(auto&n:app.nodes){auto e=backend.exportEntry(n.id,false);if(!e.empty())buffer+=e+'\n';}}
   else return 4;
  }else if(resource=="catalog")return 4;
  uint32_t total=sd?file.size():buffer.size();for(int i=0;i<4;++i)out[4+i]=total>>(8*i);out[3]=0;return 8;
 }
 if(resource.empty())return 4;
 if(in[3]==2&&!writing&&len==8){size_t at=u32(in+4);size_t total=file?file.size():buffer.size();if(at>total)return 4;size_t n=std::min(size_t(160),total-at);if(file){if(!file.seek(at)||file.read(out+4,n)!=n)return 4;}else std::memcpy(out+4,buffer.data()+at,n);out[3]=0;return 4+n;}
 if(in[3]==4&&writing&&len>=8){if(u32(in+4)!=offset||offset+len-8>(file?4*1024*1024:mc::MaxArchiveBytes))return 4;size_t n=len-8;if(file){if(file.write(in+8,n)!=n)return 4;}else buffer.append(reinterpret_cast<const char*>(in+8),n);for(size_t i=8;i<len;++i)hash=(hash^in[i])*16777619u;offset+=n;return ok();}
 if(in[3]==5&&writing&&len==8){
  if(u32(in+4)!=hash)return 4;
  if(file){file.flush();file.close();auto target=path();if(SD.exists(target.c_str())){SD.remove((target+".bak").c_str());if(!SD.rename(target.c_str(),(target+".bak").c_str()))return 4;}if(!SD.rename((target+".upload").c_str(),target.c_str()))return 4;}
  else if(resource=="settings"){mc::Settings next;if(!mc::decodeSettings(buffer,next)||!backend.saveSettings(next))return 4;app.settings=next;app.changed();}
  else if(resource=="history"){if(app.archive()!=guard){out[3]=2;return 4;}mc::App candidate(backend);if(!candidate.restore(buffer)||!backend.writeArchive(buffer))return 4;app.key(mc::Key::Home);app.restore(buffer);}
  else if(resource=="entry"){if(!backend.importEntry(buffer))return 4;syncModel();}
  else return 4;
  reset();return ok();
 }
 if(in[3]==6){reset();return ok();}
 return 4;
}
