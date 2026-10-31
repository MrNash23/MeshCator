#pragma once
#include <string>
namespace mc {
inline constexpr const char* emojis[]={"🙂","😀","😃","😄","😁","😆","😂","🤣","😊","😉","😍","🥰","😘","😎","🤓","🤔","😐","🙄","😏","😴","😢","😭","😅","😇","😬","😱","😡","🤬","🤯","🥳","🥺","🤗","👍","👎","👌","✌️","🤞","🤟","🤘","🤙","👋","🤚","✋","👏","🙌","👐","🤝","🙏","💪","👀","❤️","🧡","💛","💚","💙","💜","🖤","🤍","🤎","💔","💕","💯","✅","❌","⚠️","❓","❗","🆘","📍","🚩","🏁","🔋","🔌","📡","🛰️","📻","📱","💻","🌐","🧭","🗺️","🏠","⛺","🏕️","⛰️","🏔️","🌲","🌳","🌴","🌵","🌻","☀️","🌙","⭐","🌟","✨","🔥","⚡","💧","🌧️","❄️","🌈","☁️","🌍","🌎","🌏","🚀","✈️","🚁","🚗","🚙","🚲","🚶","🏃","🐶","🐱","🦊","🐺","🐻","🐼","🐸","🦉","🐝","🦋","💀","👻","🤖","👽","🎉","🎊","🎈","🎁","🎂","🍰","☕","🍺","🍻","🍷","🍕","🍔","🔔","🔕","🔊","🔇","🔒","🔓","🔑","🛠️","🔧","⚙️","💡","📷","🎵","🎶","🏆","🥇","🎯","💬","📢","📶"};
inline constexpr int emojiCount=sizeof(emojis)/sizeof(emojis[0]);
// LVGL has no flag ligature shaping: retain a readable country code on screen.
// Stored and transmitted text is never modified.
inline std::string emojiDisplayText(const std::string& text) {
  std::string out;
  for(size_t i=0;i<text.size();) {
    auto regional=[&](size_t at)->char {
      if(at+4>text.size())return 0;
      const auto *p=reinterpret_cast<const unsigned char*>(text.data()+at);
      if(p[0]!=0xf0||p[1]!=0x9f||p[2]!=0x87||p[3]<0xa6||p[3]>0xbf)return 0;
      return 'A'+p[3]-0xa6;
    };
    if(char first=regional(i)) {
      out+='[';out+=first;i+=4;
      if(char second=regional(i)){out+=second;i+=4;}
      out+=']';
    } else out+=text[i++];
  }
  return out;
}
}
