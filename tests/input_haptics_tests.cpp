#include "core/InputHaptics.h"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"Failed: "<<#x<<'\n';return 1;}}while(false)
struct TestBackend:mc::Backend{bool send(const mc::Chat&,const std::string&)override{return true;}bool advert(bool)override{return true;}bool saveSettings(const mc::Settings&)override{return true;}};
int main(){
 using namespace mc;TestBackend b;App a(b);a.settings.setupComplete=true;
 auto before=menuCursor(a);a.key(Key::Next);CHECK(inputPulse(Key::Next,before!=menuCursor(a),false)==InputPulse::Tick);
 a.screen=Screen::Map;before=menuCursor(a);a.key(Key::Next);CHECK(inputPulse(Key::Next,before!=menuCursor(a),false)==InputPulse::None);
 a.screen=Screen::Settings;a.settingsCategory=-1;a.selected=0;a.key(Key::Confirm);
 before=menuCursor(a);a.key(Key::Next);CHECK(inputPulse(Key::Next,before!=menuCursor(a),false)==InputPulse::Tick);
 a.utilityOpen(Utility::Emoji);before=menuCursor(a);a.key(Key::Next);CHECK(before!=menuCursor(a));
 CHECK(inputPulse(Key::Next,false,false)==InputPulse::None);
 CHECK(inputPulse(Key::Enter,false,false)==InputPulse::Confirm);
 CHECK(inputPulse(Key::Confirm,false,false)==InputPulse::Confirm);
 CHECK(inputPulse(Key::Confirm,true,true)==InputPulse::None);
 CHECK(inputPulse(Key::Delete,true,false)==InputPulse::None);
 InputHapticGate gate;
 CHECK(gate.allow(InputPulse::Tick,0));CHECK(!gate.allow(InputPulse::Tick,39));CHECK(gate.allow(InputPulse::Tick,40));
 CHECK(gate.allow(InputPulse::Confirm,41));
 gate.notification(50);CHECK(!gate.allow(InputPulse::Confirm,1149));CHECK(gate.allow(InputPulse::Tick,1150));
 gate.notification(0xfffffff0);CHECK(!gate.allow(InputPulse::Confirm,20));CHECK(gate.allow(InputPulse::Confirm,1100));
 std::cout<<"Menu changes, map exclusion, confirmation, lock, rate limit and notification priority passed\n";
}
