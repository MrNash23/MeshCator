#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace mc {
// Contact path encoding: upper two bits = hash width minus one, lower six = hops.
inline bool parseNodePath(const std::string& text,uint8_t& encoded,std::vector<uint8_t>& bytes){
 unsigned width=1;size_t start=0;
 if(text.size()>1&&text[1]==':'){if(text[0]<'1'||text[0]>'3')return false;width=text[0]-'0';start=2;}
 size_t digits=text.size()-start;
 if(digits%(2*width)||digits/2>64||digits/(2*width)>63)return false;
 bytes.clear();
 auto nibble=[](char c)->int{if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;};
 for(size_t i=start;i<text.size();i+=2){int a=nibble(text[i]),b=nibble(text[i+1]);if(a<0||b<0)return false;bytes.push_back((a<<4)|b);}
 encoded=bytes.empty()?0:((width-1)<<6)|(bytes.size()/width);return true;
}
inline std::string formatNodePath(uint8_t encoded,const uint8_t* bytes){
 if(encoded==255)return {};
 unsigned width=(encoded>>6)+1,count=encoded&63;if(width>3||count*width>64)return {};
 std::string s=std::to_string(width)+":";const char*d="0123456789abcdef";
 for(unsigned i=0;i<count*width;++i){s+=d[bytes[i]>>4];s+=d[bytes[i]&15];}return s;
}
// A round trip visits the destination repeater then the reverse outgoing route.
// MeshCore's trace wire format supports power-of-two hash widths only.
inline bool nodeTracePath(uint8_t encoded,const uint8_t* path,const uint8_t* peer,bool repeater,std::vector<uint8_t>& route,uint8_t& flags){
 if(encoded==255)return false;unsigned width=(encoded>>6)+1,count=encoded&63;
 if(width>2||count*width>64||(count*2+(repeater?1:0))*width>=64||(!count&&!repeater))return false;
 route.clear();if(count)route.assign(path,path+count*width);
 if(repeater)route.insert(route.end(),peer,peer+width);
 for(unsigned i=count;i>0;--i)route.insert(route.end(),path+(i-1)*width,path+i*width);
 flags=width-1;return true;
}
}
