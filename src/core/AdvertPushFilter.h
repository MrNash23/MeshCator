#pragma once
#include <array>
#include <cstdint>
#include <cstring>
namespace mc {
// Only suppress repeat discovery notifications, never contact-list replies or messages.
class AdvertPushFilter {
 struct Entry {std::array<uint8_t,32> key{};uint32_t stamp=0,seen=0;bool valid=false;};
 std::array<Entry,64> entries_{};
 unsigned next_=0;
 static bool discovery(const uint8_t*frame,size_t len){return len>=136&&frame[0]==0x8a;}
 static uint32_t stamp(const uint8_t*f){return uint32_t(f[132])|(uint32_t(f[133])<<8)|(uint32_t(f[134])<<16)|(uint32_t(f[135])<<24);}
public:
 void clear(){for(auto&e:entries_)e.valid=false;next_=0;}
 bool duplicate(const uint8_t*frame,size_t len,uint32_t now)const{
  if(!discovery(frame,len))return false;
  for(const auto&e:entries_)if(e.valid&&e.stamp==stamp(frame)&&uint32_t(now-e.seen)<60000&&std::memcmp(e.key.data(),frame+1,32)==0)return true;
  return false;
 }
 // Remember only after the frame was accepted into the BLE send queue.
 void accepted(const uint8_t*frame,size_t len,uint32_t now){
  if(!discovery(frame,len))return;
  auto &e=entries_[next_++%entries_.size()];std::memcpy(e.key.data(),frame+1,32);e.stamp=stamp(frame);e.seen=now;e.valid=true;
 }
};
}
