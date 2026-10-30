#include "maps/WorldTileDecoder.h"
#include "maps/Projection.h"
#include "maps/AdminMap.h"
#include <string>
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
bool land(double lat,double lon){std::vector<uint8_t> tile;int x=mc::mapX(lon,4096),y=mc::mapY(lat,4096);return mc::loadWorldTile(4,x/256,y/256,tile)&&mc::worldLand(tile,x%256,y%256);}
int main(){
 std::vector<uint8_t> tile;int count=0;
 for(int z=0;z<=4;++z)for(int y=0;y<(1<<z);++y)for(int x=0;x<(1<<z);++x){CHECK(mc::loadWorldTile(z,x,y,tile));CHECK(tile.size()==8192);++count;}
 CHECK(count==341);CHECK(!mc::loadWorldTile(-1,0,0,tile));CHECK(!mc::loadWorldTile(5,0,0,tile));CHECK(!mc::loadWorldTile(4,16,0,tile));
 CHECK(land(52.52,13.405));CHECK(land(-25,135));CHECK(!land(0,-30));CHECK(!land(0,179));CHECK(!land(0,-179));CHECK(!land(47.7,-87.5));
 auto country=mc::adminAt(52.52,13.405,0),region=mc::adminAt(52.52,13.405,1);
 CHECK(country&&std::string(country->names[3])=="Deutschland");CHECK(region&&std::string(region->names[0])=="Berlin");
 CHECK(mc::adminAt(48.137,11.575,1));CHECK(!mc::adminAt(0,-30,0));
 for(auto a:mc::adminAreas){CHECK(a.count>0);CHECK(a.first+a.count<=sizeof(mc::adminRings)/sizeof(mc::adminRings[0]));for(auto name:a.names)CHECK(name&&*name);}
 for(auto r:mc::adminRings)CHECK(r.count>=4&&r.first+r.count<=sizeof(mc::adminPoints)/sizeof(mc::adminPoints[0]));
 std::cout<<"341 tiles decoded; land, ocean, dateline and lake samples passed\n";
}
