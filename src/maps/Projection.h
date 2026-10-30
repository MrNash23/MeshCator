#pragma once
#include <algorithm>
#include <cmath>
namespace mc {
inline double mapX(double lon, double world) { return (lon + 180.) / 360. * world; }
inline double mapY(double lat, double world) {
  lat = std::clamp(lat, -85.05112878, 85.05112878) * 3.141592653589793 / 180.;
  return (1. - std::asinh(std::tan(lat)) / 3.141592653589793) * .5 * world;
}
inline double wrapMapX(double x, double world) { return x - std::floor((x + world/2) / world) * world; }
inline bool clipMapLine(double &x0,double &y0,double &x1,double &y1,double w,double h) {
  auto code=[&](double x,double y){return (x<0?1:x>w?2:0)|(y<0?4:y>h?8:0);};
  for(int i=0;i<12;++i) {
    int a=code(x0,y0),b=code(x1,y1);
    if(!(a|b)) return true;
    if(a&b) return false;
    int c=a?a:b; double x=0,y=0;
    if(c&8){x=x0+(x1-x0)*(h-y0)/(y1-y0);y=h;}
    else if(c&4){x=x0+(x1-x0)*(0-y0)/(y1-y0);y=0;}
    else if(c&2){y=y0+(y1-y0)*(w-x0)/(x1-x0);x=w;}
    else {y=y0+(y1-y0)*(0-x0)/(x1-x0);x=0;}
    if(c==a){x0=x;y0=y;}else{x1=x;y1=y;}
  }
  return false;
}
}
