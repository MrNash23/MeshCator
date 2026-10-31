#pragma once
namespace mc {
// BQ25896 REG0B: VBUS=7 is OTG output, not external USB power.
inline bool usbPowered(int reg){return reg>=0&&(reg&4)&&((reg>>5)&7)!=7;}
inline bool batteryCharging(int reg){int state=(reg>>3)&3;return usbPowered(reg)&&(state==1||state==2);}
}
