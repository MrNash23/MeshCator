#pragma once
#include <cstdint>
namespace mc {
// Defer the short action until release so holding Back never deletes text.
class BackGesture {
  bool down_ = false, fired_ = false;
  uint32_t since_ = 0;
public:
  static constexpr uint32_t HoldMs = 800;
  enum class Action { None, Short, Back };
  Action update(bool down, uint32_t now) {
    if (down && !down_) { since_ = now; fired_ = false; }
    Action action = Action::None;
    if ((down || down_) && !fired_ && uint32_t(now - since_) >= HoldMs) {
      fired_ = true;
      action = Action::Back;
    } else if (!down && down_ && !fired_ && uint32_t(now - since_) >= 30) {
      action = Action::Short;
    }
    down_ = down;
    return action;
  }
};
}
