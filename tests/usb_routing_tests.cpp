#include "helpers/MultiSerialInterface.h"
#include <vector>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<'\n';return 1;}}while(false)
int calls=0;
extern "C" size_t meshcator_usb_request(const uint8_t*,size_t,uint8_t*out){++calls;out[0]=0x7e;out[1]='M';out[2]='C';out[3]=0;out[4]=42;return 5;}
struct Port:BaseSerialInterface{
 int writes=0;bool enabled=false;std::vector<uint8_t> input,output;
 void enable()override{enabled=true;}void disable()override{enabled=false;}bool isEnabled()const override{return enabled;}bool isConnected()const override{return true;}bool isWriteBusy()const override{return false;}
 size_t writeFrame(const uint8_t*s,size_t n)override{++writes;output.assign(s,s+n);return n;}
 size_t checkRecvFrame(uint8_t*d)override{size_t n=input.size();std::copy(input.begin(),input.end(),d);input.clear();return n;}
};
int main(){Port usb,ble;MultiSerialInterface mux;mux.addInterface(InterfaceType::USB,&usb);mux.addInterface(InterfaceType::Bluetooth,&ble);CHECK(mux.addInterface(InterfaceType::Bluetooth,&ble));CHECK(mux.addInterface(InterfaceType::Bluetooth,&ble));mux.enable();uint8_t normal[]={0x8a,0};mux.writeFrame(normal,2);CHECK(ble.writes==1&&usb.writes==1);ble.output.clear();usb.output.clear();uint8_t buf[176];usb.input={0x7e,'M','C',1};CHECK(mux.checkRecvFrame(buf)==0&&calls==1&&usb.output.size()==5&&ble.output.empty());usb.output.clear();ble.input={0x7e,'M','C',1};CHECK(mux.checkRecvFrame(buf)==0&&calls==1&&ble.output[3]==1&&usb.output.empty());usb.input={22,3};CHECK(mux.checkRecvFrame(buf)==2&&buf[0]==22);std::cout<<"USB maintenance isolated from BLE; standard protocol preserved\n";}
