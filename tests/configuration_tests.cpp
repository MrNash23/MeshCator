#include "core/App.h"
#include "core/RadioPresets.h"
#include "maps/Projection.h"
#include "maps/BmpTile.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <cstdlib>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
struct Backend:mc::Backend{
 bool accept=true;int sends=0;std::string lastKey;
 std::string channelKey()override{return "0123456789abcdef0123456789abcdef";}
 std::string createChannel(const std::string&,const std::string&key,bool)override{lastKey=key;return accept?"channel:1":"";}
 bool send(const mc::Chat&,const std::string&)override{++sends;return true;}
 bool advert(bool)override{return true;}
 bool saveSettings(const mc::Settings&)override{return accept;}
};
int main(){
 mc::Settings s,restored;std::string error;
 for(auto&p:mc::radioPresets){CHECK(mc::setSetting(s,mc::SettingField::Preset,p.name,error));CHECK(s.sf==p.sf&&s.hashBytes==p.hashBytes);CHECK(mc::decodeSettings(mc::encodeSettings(s),restored));}
 CHECK(!mc::setSetting(s,mc::SettingField::Frequency,"nan",error));
 CHECK(!mc::setSetting(s,mc::SettingField::Power,"23",error));
 CHECK(!mc::setSetting(s,mc::SettingField::Power,"14.1",error));
 CHECK(!mc::setSetting(s,mc::SettingField::BluetoothPIN,"12345",error));
 CHECK(!mc::setSetting(s,mc::SettingField::BluetoothPIN,"000012",error));
 CHECK(mc::setSetting(s,mc::SettingField::BluetoothPIN,"123456",error));
 s.waypoints={{"Café \"Nord\"",{52.5,13.4,true}},{"Equator",{0,0,true}}};s.bluetooth=true;s.setupComplete=true;
 CHECK(mc::decodeSettings(mc::encodeSettings(s),restored));
 CHECK(restored.bluetoothPIN=="123456"&&restored.waypoints.size()==2&&restored.waypoints[0].name==s.waypoints[0].name&&restored.waypoints[1].position.valid);
 CHECK(!mc::decodeSettings("MC2\n99 0",restored));
 Backend backend;mc::App app(backend);
 app.receive("test","Test","hello",false);app.open(0);app.draft="pending";CHECK(!app.send()&&backend.sends==0&&app.draft=="pending");
 app.startIntroduction();CHECK(app.setupWizard&&app.editField==mc::SettingField::Language);
 for(int i=0;i<5;++i){app.key(mc::Key::Confirm);CHECK(!app.settings.setupComplete);}
 CHECK(app.editField==mc::SettingField::Setup);
 backend.accept=false;app.key(mc::Key::Confirm);CHECK(!app.settings.setupComplete&&app.settingsEditing);
 backend.accept=true;app.key(mc::Key::Confirm);CHECK(app.settings.setupComplete&&app.helpOpen);
 app.key(mc::Key::Home);app.screen=mc::Screen::Map;app.key(mc::Key::Enter);app.mapMenuSelected=3;app.key(mc::Key::Confirm);CHECK(!app.waypointNaming);
 app.status.position={0,0,true};app.key(mc::Key::Enter);app.mapMenuSelected=3;app.key(mc::Key::Confirm);CHECK(app.waypointNaming);
 app.type("Home");app.status.position={1,1,true};backend.accept=false;app.key(mc::Key::Confirm);CHECK(app.waypointNaming&&app.settings.waypoints.empty());
 backend.accept=true;app.key(mc::Key::Confirm);CHECK(app.settings.waypoints.size()==1&&app.settings.waypoints[0].position.lat==0);
 app.key(mc::Key::Enter);app.mapMenuSelected=4;app.key(mc::Key::Confirm);app.key(mc::Key::Confirm);CHECK(app.navigationTarget==0&&app.mapCenter.valid);
 app.screen=mc::Screen::Settings;app.beginChannel();app.key(mc::Key::Next);app.key(mc::Key::Enter);app.type("Friends");app.key(mc::Key::Enter);CHECK(app.channelStage==2&&app.channelSecret.size()==32);
 app.type("invalid");app.key(mc::Key::Enter);CHECK(app.channelStage==2);
 app.channelSecret="0123456789abcdef0123456789abcdef";app.key(mc::Key::Enter);backend.accept=false;app.key(mc::Key::Enter);CHECK(app.channelCreating);
 backend.accept=true;app.key(mc::Key::Enter);CHECK(!app.channelCreating&&app.channelSecret.empty()&&backend.lastKey.size()==32);
 app.beginChannel();app.channelSecret="private";app.key(mc::Key::Menu);CHECK(!app.channelCreating&&app.channelSecret.empty());app.key(mc::Key::Back);
 app.settings.chatFilter=1;CHECK(app.visibleChats().size()==1&&app.chats[app.visibleChats()[0]].channel);
 app.settings.chatFilter=2;CHECK(app.visibleChats().size()==1&&!app.chats[app.visibleChats()[0]].channel);
 CHECK(std::abs(mc::mapY(0,256)-128)<1e-9);
 CHECK(std::abs(mc::wrapMapX(mc::mapX(-179,256)-mc::mapX(179,256),256)-256.*2/360)<1e-8);
 double x0=-100,y0=10,x1=100,y1=10;CHECK(mc::clipMapLine(x0,y0,x1,y1,50,50)&&x0==0&&x1==50);
 x0=-10;y0=-20;x1=80;y1=-20;CHECK(!mc::clipMapLine(x0,y0,x1,y1,50,50));
 std::vector<uint8_t> bmp(54+256*768);auto put=[&](int p,uint32_t v){for(int i=0;i<4;++i)bmp[p+i]=(v>>(8*i))&255;};
 bmp[0]='B';bmp[1]='M';put(10,54);put(14,40);put(18,256);put(22,256);bmp[26]=1;bmp[28]=24;bmp[54+2]=255;
 int reads=0;std::vector<uint16_t> pixels;auto read=[&](uint32_t off,uint8_t*out,size_t n){++reads;if(off+n>bmp.size())return false;std::memcpy(out,bmp.data()+off,n);return true;};
 CHECK(mc::readBmpTile(read,pixels)&&pixels[255*256]==0xf800&&pixels[0]==0);
 CHECK(reads==33);
 put(22,uint32_t(-256));CHECK(mc::readBmpTile(read,pixels)&&pixels[0]==0xf800);
 bmp.resize(100);CHECK(!mc::readBmpTile(read,pixels));
 std::cout<<"Configuration, setup, waypoint, projection and tile checks passed\n";
}
