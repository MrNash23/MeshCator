#pragma once
#include "App.h"
#include <cstdio>
#include <cmath>
namespace mc {
inline bool decodeMapHome(const char* text,Position& pos,int& zoom){
 double lat,lon;int z;char extra;
 if(std::sscanf(text,"MCMAP1 %lf %lf %d %c",&lat,&lon,&z,&extra)!=3 || !std::isfinite(lat)||!std::isfinite(lon)||lat < -85.05112878||lat > 85.05112878||lon < -180||lon > 180||z<0||z>18)return false;
 pos={lat,lon,true};zoom=z;return true;
}
}
