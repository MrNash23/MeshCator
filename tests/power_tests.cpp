#include "PowerManager.h"
#include "core/PowerState.h"
#include <vector>
#include <utility>
#include <iostream>
uint32_t testMillis=0;unsigned backlightWrites=0;
std::vector<std::pair<int,bool>> outputs;std::vector<bool> gps;
extern "C" bool meshcator_power_output(uint8_t pin,bool on){outputs.push_back({pin,on});return true;}
extern "C" void tpager_gps_power(bool on){gps.push_back(on);}
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<"\n";return 1;}}while(false)
int main(){
 auto& pm=PowerManager::instance();pm.begin();CHECK(outputs.size()==1&&outputs[0].first==5&&!outputs[0].second);
 pm.setGpsEnabled(false);CHECK(!gps.back());testMillis=600000;pm.loop();CHECK(gps.size()==1);
 pm.setGpsEnabled(true);CHECK(gps.back());testMillis+=29999;pm.loop();CHECK(gps.back());++testMillis;pm.loop();CHECK(!gps.back());testMillis+=270000;pm.loop();CHECK(gps.back());
 pm.setTargetBrightness(200);pm.setScreenTimeout(90);pm.onActivity();testMillis+=45000;pm.loop();auto writes=backlightWrites;pm.loop();CHECK(backlightWrites==writes);pm.onActivity();CHECK(backlightWrites==writes+1);
 testMillis+=90000;pm.loop();CHECK(pm.isScreenOff());pm.onActivity();CHECK(!pm.isScreenOff());
 CHECK(!mc::usbPowered(-1)&&!mc::batteryCharging(-1));CHECK(!mc::usbPowered(0));CHECK(mc::usbPowered(0x24)&&!mc::batteryCharging(0x24));CHECK(mc::batteryCharging(0x2c)&&mc::batteryCharging(0x34));CHECK(mc::usbPowered(0x3c)&&!mc::batteryCharging(0x3c));CHECK(!mc::usbPowered(0xe4));
}
