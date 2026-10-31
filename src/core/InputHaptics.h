#pragma once
#include "App.h"
#include <tuple>
namespace mc {
enum class InputPulse { None, Tick, Confirm };
// Only selection state, deliberately excluding chat history and map zoom.
inline auto menuCursor(const App& a) {
 return std::make_tuple(a.selected,a.menuSelected,a.contextSelected,a.settingChoice,
  a.mapMenuSelected,a.waypointSelected,a.notificationChoice,a.utilityChoice,
  a.channelKind,a.helpPage);
}
inline InputPulse inputPulse(Key key,bool selectionChanged,bool locked) {
 if(locked)return InputPulse::None;
 if(key==Key::Confirm||key==Key::Enter)return InputPulse::Confirm;
 if((key==Key::Next||key==Key::Previous)&&selectionChanged)return InputPulse::Tick;
 return InputPulse::None;
}
class InputHapticGate {
 uint32_t last_=0,alert_=0;bool played_=false,alertPlaying_=false;
public:
 void notification(uint32_t now){alert_=now;alertPlaying_=true;}
 bool allow(InputPulse pulse,uint32_t now){
  if(pulse==InputPulse::None)return false;
  // Do not replace the existing 1000-ms message alert with a navigation tick.
  if(alertPlaying_){if(uint32_t(now-alert_)<1100)return false;alertPlaying_=false;}
  if(pulse==InputPulse::Tick&&played_&&uint32_t(now-last_)<40)return false;
  played_=true;last_=now;return true;
 }
};
}
