#include "core/MapHome.h"
#include <cassert>
struct TestBackend:mc::Backend {
 int calls=0;
 bool send(const mc::Chat&,const std::string&)override{return true;}
 bool advert(bool)override{return true;}
 bool saveSettings(const mc::Settings&)override{return true;}
 bool mapHome(mc::Position&p,int&z)override{++calls;return mc::decodeMapHome("MCMAP1 52.52 13.405 12",p,z);}
};
int main(){
 mc::Position p;int z=0;
 assert(mc::decodeMapHome("MCMAP1 52.52 13.405 12\n",p,z));assert(p.valid&&z==12);
 for(auto s:{"", "MCMAP1 nan 13 12", "MCMAP1 91 13 12", "MCMAP1 52 181 12", "MCMAP1 52 13 19", "MCMAP1 52 13 12 junk"})assert(!mc::decodeMapHome(s,p,z));
 TestBackend b;mc::App a(b);a.prepareMap();assert(b.calls==0);
 a.settings.mapSource=2;a.prepareMap();assert(a.mapZoom==12&&a.offlineMapCenter.valid&&!a.mapCenter.valid&&!a.status.position.valid);
 a.mapZoom=7;a.prepareMap();assert(a.mapZoom==7&&b.calls==1);
 TestBackend b2;mc::App a2(b2);a2.settings.mapSource=1;a2.mapCenter={51,14,true};a2.mapZoom=10;a2.prepareMap();assert(a2.mapCenter.lat==51&&a2.mapZoom==10);
 TestBackend b3;mc::App a3(b3);a3.screen=mc::Screen::Map;a3.settings.mapSource=2;
 assert(a3.type("a"));double left=a3.mapCenter.lon;assert(left<13.405);assert(a3.type("s"));assert(std::abs(a3.mapCenter.lon-13.405)<1e-8);
 double lat=a3.mapCenter.lat;assert(a3.type("w")&&a3.mapCenter.lat>lat);assert(a3.type("z")&&std::abs(a3.mapCenter.lat-lat)<1e-8);
 a3.mapFullscreen=true;assert(a3.type("S")&&a3.mapCenter.lon>13.405);
 a3.mapMenuOpen=true;auto before=a3.mapCenter;assert(!a3.type("a")&&a3.mapCenter.lon==before.lon);a3.mapMenuOpen=false;
 a3.waypointNaming=true;assert(a3.type("a")&&a3.waypointName=="a"&&a3.mapCenter.lon==before.lon);a3.waypointNaming=false;
 a3.mapCenter={85.05,179.99,true};a3.mapZoom=2;assert(a3.type("s")&&a3.mapCenter.lon<0);assert(a3.type("w")&&a3.mapCenter.lat<=85.05112879);
}
