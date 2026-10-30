#include "core/NodePath.h"
#include "core/UtilityText.h"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<"FAIL "<<__LINE__<<'\n';return 1;}}while(false)
int main(){uint8_t encoded,flags;std::vector<uint8_t> bytes,route;
 CHECK(mc::parseNodePath("",encoded,bytes)&&encoded==0);
 CHECK(mc::parseNodePath("2:aabbccdd",encoded,bytes)&&encoded==66);
 CHECK(mc::formatNodePath(encoded,bytes.data())=="2:aabbccdd");
 uint8_t peer[]={0x12,0x34,0x56,0x78};
 CHECK(mc::nodeTracePath(encoded,bytes.data(),peer,true,route,flags));
 CHECK(flags==1&&route==std::vector<uint8_t>({0xaa,0xbb,0xcc,0xdd,0x12,0x34,0xcc,0xdd,0xaa,0xbb}));
 CHECK(!mc::parseNodePath("2:aabbcc",encoded,bytes));CHECK(!mc::parseNodePath("aa zz",encoded,bytes));CHECK(!mc::parseNodePath("4:aabbccdd",encoded,bytes));
 CHECK(!mc::parseNodePath(std::string(128,'a'),encoded,bytes));
 CHECK(mc::parseNodePath("2:"+std::string(128,'a'),encoded,bytes)&&encoded==96);
 CHECK(!mc::nodeTracePath(encoded,bytes.data(),peer,true,route,flags));
 CHECK(mc::parseNodePath("3:aabbcc",encoded,bytes)&&encoded==129);
 CHECK(!mc::nodeTracePath(encoded,bytes.data(),peer,true,route,flags));
 CHECK(!mc::nodeTracePath(255,peer,peer,true,route,flags));
 CHECK(mc::nodeTracePath(0,peer,peer,true,route,flags)&&route.size()==1);
 auto pages=mc::utilityPages("one\ntwo\nthree\nfour\nfive\nsix");CHECK(pages.size()==2&&pages[1]=="six");
 CHECK(mc::utilityPages(std::string(211,'a')).size()==2);
 std::cout<<"Path widths, bounds, invalid input and trace round trips passed\n";
}
