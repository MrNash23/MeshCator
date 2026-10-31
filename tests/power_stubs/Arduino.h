#pragma once
#include <cstdint>
extern uint32_t testMillis;
extern unsigned backlightWrites;
inline uint32_t millis(){return testMillis;}
inline void ledcSetup(int,int,int){}
inline void ledcAttachPin(int,int){}
inline void ledcWrite(int,int){++backlightWrites;}
