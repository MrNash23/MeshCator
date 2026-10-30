#pragma once
#include <cstdint>
namespace mc {
// Defer the short action until release so holding Back never deletes text.
class BackGesture {
  bool down_ = false, fired_ = false;
  uint32_t since_ = 0;
public:
  enum class Action { None, Short, Home };
  Action update(bool down, uint32_t now) {
    if (down && !down_) { since_ = now; fired_ = false; }
    Action action = Action::None;
    if ((down || down_) && !fired_ && uint32_t(now - since_) > 2000) {
      fired_ = true;
      action = Action::Home;
    } else if (!down && down_ && !fired_ && uint32_t(now - since_) >= 30) {
      action = Action::Short;
    }
    down_ = down;
    return action;
  }
};
}
