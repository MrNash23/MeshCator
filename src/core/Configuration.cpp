#include "App.h"
#include "RadioPresets.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
namespace mc {
std::vector<SettingField> settingFields(int category) {
  using F=SettingField;
  switch(category) {
  case 2:return {F::Preset,F::Frequency,F::Bandwidth,F::SF,F::CR,F::Power};
  case 3:return {F::Name,F::Mode,F::GPS,F::ShareLocation};
  case 4:return {F::Bluetooth,F::BluetoothName,F::BluetoothPIN};
  case 5:return {F::Brightness,F::Timeout};
  case 8:return {F::NotificationMute,F::NotificationMode,F::NotificationVolume,F::NotificationWake};
  case 6:return {F::MapSource,F::Contacts};
  default:return {};
  }
}
const char *settingLabel(SettingField f) {
  static const char *names[]={"Sprache","Farbe","Region / Profil","Frequenz MHz","Bandbreite kHz","Spreading Factor","Coding Rate","Sendeleistung dBm","Betriebsmodus","Gerätename","GPS","Standort teilen","Bluetooth","Bluetooth-Name","Kopplungs-PIN","Helligkeit %","Display-Timeout s","Kartenquelle","Kontakte anzeigen","Einrichtung abschließen","Version","Gesprächsfilter","Alle stummschalten","Standardhinweis","Lautstärke %","Display bei Eingang"};
  return names[static_cast<int>(f)];
}
std::vector<std::string> settingOptions(SettingField f) {
  using F=SettingField;
  switch(f){
  case F::NotificationMode:return {"Stumm","Ton","Vibration","Ton + Vibration"};
  case F::NotificationVolume:return {"20","40","60","80","100"};
  case F::Language:return {"English","Français","Español","Deutsch"};
  case F::Theme:return {"Farbkarten","Helle Baender","Warm Orange","Neon Purple"};
  case F::Preset:{std::vector<std::string> v; for(auto &p:radioPresets)v.push_back(p.name); return v;}
 case F::ChatFilter:return {"Alle","Kanäle","Direktnachrichten"};
  case F::Mode:return {"Companion","Companion + forwarding"};
  case F::GPS:case F::ShareLocation:case F::Bluetooth:case F::Contacts:case F::NotificationMute:case F::NotificationWake:return {"Aus","Ein"};
  case F::Bandwidth:return {"7.8","10.4","15.6","20.8","31.25","41.7","62.5","125","250","500"};
  case F::SF:return {"5","6","7","8","9","10","11","12"};
  case F::CR:return {"5","6","7","8"};
  case F::Brightness:return {"20","40","60","80","100"};
  case F::Timeout:return {"30","60","90","120","300","600"};
  case F::MapSource:return {"Integriert","SD-Karte","Automatisch"};
  default:return {};
  }
}
static std::string num(double v){std::ostringstream s;s<<std::setprecision(9)<<v;return s.str();}
std::string settingValue(const Settings&s,SettingField f){
 using F=SettingField;
 switch(f){
 case F::NotificationMute:return s.notificationMute?"Ein":"Aus";
 case F::NotificationWake:return s.notificationWake?"Ein":"Aus";
 case F::NotificationMode:return settingOptions(f)[s.notificationMode];
 case F::NotificationVolume:return num(s.notificationVolume);
 case F::ChatFilter:return settingOptions(f)[s.chatFilter];
 case F::Language:return languageName(s.language);
 case F::Theme:return settingOptions(f)[static_cast<int>(s.theme)];
 case F::Preset:return s.preset>=0&&s.preset<radioPresetCount?radioPresets[s.preset].name:"Benutzerdefiniert";
 case F::Frequency:return num(s.frequency);case F::Bandwidth:return num(s.bandwidth);
 case F::SF:return num(s.sf);case F::CR:return num(s.cr);case F::Power:return num(s.power);
 case F::Mode:return settingOptions(f)[s.repeat?1:0];case F::Name:return s.name;
 case F::GPS:return s.gps?"Ein":"Aus";case F::ShareLocation:return s.shareLocation?"Ein":"Aus";
 case F::Bluetooth:return s.bluetooth?"Ein":"Aus";case F::BluetoothName:return s.bluetoothName;
 case F::BluetoothPIN:return s.bluetoothPIN;case F::Brightness:return num(s.brightness);case F::Timeout:return num(s.timeout);
 case F::MapSource:return settingOptions(f)[s.mapSource];case F::Contacts:return s.showContacts?"Ein":"Aus";
 case F::Setup:return s.setupComplete?"Bestätigt":"Senden gesperrt";case F::Version:return "MeshCator 0.3.8-dev";
 }
 return {};
}
bool setSetting(Settings&s,SettingField f,const std::string&v,std::string&error){
 using F=SettingField; auto options=settingOptions(f);auto it=std::find(options.begin(),options.end(),v); int idx=it==options.end()?-1:int(it-options.begin());
 auto fail=[&](){error="Ungültiger Wert";return false;};
 if(!options.empty()&&idx<0)return fail();
 double n=0;
 if(f==F::Frequency||f==F::Power){std::istringstream in(v);char extra;if(!(in>>n)||in>>extra||!std::isfinite(n))return fail();}
 switch(f){
 case F::NotificationMute:s.notificationMute=idx==1;break;
 case F::NotificationWake:s.notificationWake=idx==1;break;
 case F::NotificationMode:s.notificationMode=idx;break;
 case F::NotificationVolume:s.notificationVolume=std::stoi(v);break;
 case F::ChatFilter:s.chatFilter=idx;break;
 case F::Language:s.language=static_cast<Language>(idx);break;
 case F::Theme:s.theme=static_cast<Theme>(idx);break;
 case F::Preset:{auto &p=radioPresets[idx];s.preset=idx;s.frequency=p.frequency;s.bandwidth=p.bandwidth;s.sf=p.sf;s.cr=p.cr;s.hashBytes=p.hashBytes;break;}
 case F::Frequency:if(n<150||n>960)return fail();s.frequency=n;s.preset=-1;break;
 case F::Bandwidth:s.bandwidth=std::stod(v);s.preset=-1;break;
 case F::SF:s.sf=std::stoi(v);s.preset=-1;break;case F::CR:s.cr=std::stoi(v);s.preset=-1;break;
 case F::Power:if(n< -9||n>22||n!=std::floor(n))return fail();s.power=int(n);break;
 case F::Mode:s.repeat=idx==1;break;
 case F::Name:case F::BluetoothName:
  if(v.empty()||v.size()>31||!validText(v))return fail();
  if(f==F::Name)s.name=v;else s.bluetoothName=v;break;
 case F::GPS:s.gps=idx==1;break;case F::ShareLocation:s.shareLocation=idx==1;break;case F::Bluetooth:s.bluetooth=idx==1;break;
 case F::BluetoothPIN:if(v.size()!=6||v[0]=='0'||v.find_first_not_of("0123456789")!=std::string::npos)return fail();s.bluetoothPIN=v;break;
 case F::Brightness:s.brightness=std::stoi(v);break;case F::Timeout:s.timeout=std::stoi(v);break;
 case F::MapSource:s.mapSource=idx;break;case F::Contacts:s.showContacts=idx==1;break;
 case F::Setup:s.setupComplete=true;break;case F::Version:break;
 }
 return true;
}
std::string encodeSettings(const Settings&s){
 std::ostringstream o;o<<"MC2\n"<<int(s.theme)<<' '<<int(s.language)<<' '<<int(s.helpVersion)<<' '<<s.setupComplete<<' '<<s.preset<<'\n';
 o<<std::setprecision(12)<<s.frequency<<' '<<s.bandwidth<<' '<<s.sf<<' '<<s.cr<<' '<<s.power<<' '<<s.hashBytes<<'\n';
 o<<s.repeat<<' '<<s.gps<<' '<<s.shareLocation<<' '<<s.bluetooth<<' '<<s.brightness<<' '<<s.timeout<<' '<<s.mapSource<<' '<<s.showContacts<<'\n';
 o<<s.name<<'\n'<<s.bluetoothName<<'\n'<<s.bluetoothPIN<<'\n';
 o<<s.waypoints.size()<<'\n';for(auto &w:s.waypoints)o<<std::quoted(w.name)<<' '<<w.position.lat<<' '<<w.position.lon<<'\n';o<<s.chatFilter<<'\n';
 o<<"N1 "<<s.notificationMute<<' '<<s.notificationMode<<' '<<s.notificationVolume<<' '<<s.notificationWake<<' '<<s.notificationRules.size()<<'\n';
 for(auto &r:s.notificationRules)o<<std::quoted(r.first)<<' '<<r.second<<'\n';return o.str();
}
bool decodeSettings(const std::string&text,Settings &out){
 std::istringstream in(text);std::string tag;std::getline(in,tag);if(tag!="MC2")return false;
 Settings s;int theme,language,help;
 if(!(in>>theme>>language>>help>>s.setupComplete>>s.preset>>s.frequency>>s.bandwidth>>s.sf>>s.cr>>s.power>>s.hashBytes>>s.repeat>>s.gps>>s.shareLocation>>s.bluetooth>>s.brightness>>s.timeout>>s.mapSource>>s.showContacts))return false;
 in.ignore(1024,'\n');std::getline(in,s.name);std::getline(in,s.bluetoothName);std::getline(in,s.bluetoothPIN);
 if(theme<0||theme>3||language<0||language>3||help<0||help>255||s.mapSource<0||s.mapSource>2||s.preset< -1||s.preset>=radioPresetCount||s.hashBytes<1||s.hashBytes>3)return false;
 s.theme=static_cast<Theme>(theme);s.language=static_cast<Language>(language);s.helpVersion=help;
 Settings validated=s;std::string error;
 for(auto f:{SettingField::Frequency,SettingField::Bandwidth,SettingField::SF,SettingField::CR,SettingField::Power,SettingField::Name,SettingField::BluetoothName,SettingField::BluetoothPIN,SettingField::Brightness,SettingField::Timeout})if(!setSetting(validated,f,settingValue(s,f),error))return false;
 size_t count=0;
 if(in>>count){if(count>16)return false;for(size_t i=0;i<count;++i){Waypoint w;if(!(in>>std::quoted(w.name)>>w.position.lat>>w.position.lon)||w.name.empty()||w.name.size()>31||!validText(w.name)||!std::isfinite(w.position.lat)||!std::isfinite(w.position.lon)||std::abs(w.position.lat)>90||std::abs(w.position.lon)>180)return false;w.position.valid=true;s.waypoints.push_back(w);}}
 int filter=0;if(in>>filter){if(filter<0||filter>2)return false;s.chatFilter=filter;}
 std::string extension;
 if(in>>extension){
  size_t rules;
  if(extension!="N1"||!(in>>s.notificationMute>>s.notificationMode>>s.notificationVolume>>s.notificationWake>>rules)||rules>32||s.notificationMode<0||s.notificationMode>3||s.notificationVolume<20||s.notificationVolume>100||s.notificationVolume%20)return false;
  for(size_t i=0;i<rules;++i){std::string key;int mode;if(!(in>>std::quoted(key)>>mode)||key.empty()||key.size()>96||!validText(key)||mode<0||mode>3||s.notificationRules.count(key))return false;s.notificationRules[key]=mode;}
  if(in>>extension)return false;
 }
 out=s;return true;
}
}
