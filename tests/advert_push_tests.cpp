#include "core/AdvertPushFilter.h"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<'\n';return 1;}}while(false)
int main(){mc::AdvertPushFilter f;uint8_t frame[148]={};frame[0]=0x8a;frame[1]=1;frame[132]=42;
 CHECK(!f.duplicate(frame,sizeof(frame),10)); // A full send queue must not consume discovery.
 CHECK(!f.duplicate(frame,sizeof(frame),11));f.accepted(frame,sizeof(frame),11);
 CHECK(f.duplicate(frame,sizeof(frame),12));CHECK(f.duplicate(frame,sizeof(frame),13));
 frame[132]++;CHECK(!f.duplicate(frame,sizeof(frame),14));frame[132]--;
 frame[32]=1;CHECK(!f.duplicate(frame,sizeof(frame),14));frame[32]=0;
 CHECK(!f.duplicate(frame,100,14));frame[0]=3;CHECK(!f.duplicate(frame,sizeof(frame),14));frame[0]=0x8a;
 CHECK(!f.duplicate(frame,sizeof(frame),60011));f.clear();CHECK(!f.duplicate(frame,sizeof(frame),20));
 f.accepted(frame,sizeof(frame),0xfffffff0);CHECK(f.duplicate(frame,sizeof(frame),20));
 std::cout<<"Repeated discoveries filtered; updated adverts, different full keys, reconnects and other frames retained\n";
}
