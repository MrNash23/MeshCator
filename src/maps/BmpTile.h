#pragma once
#include <cstdint>
#include <vector>
namespace mc {
// Bounded decoder: XYZ tiles are 256x256, 24-bit uncompressed Windows BMP.
// Reader(offset,buffer,length) returns false on missing/truncated data.
template<class Reader> bool readBmpTile(Reader read,std::vector<uint16_t>&out){
 uint8_t header[54];if(!read(0,header,sizeof header))return false;
 auto u16=[&](int p){return uint16_t(header[p]|header[p+1]<<8);};
 auto u32=[&](int p){return uint32_t(header[p])|uint32_t(header[p+1])<<8|uint32_t(header[p+2])<<16|uint32_t(header[p+3])<<24;};
 int32_t height=int32_t(u32(22));uint32_t start=u32(10);
 if(u16(0)!=0x4d42||u32(14)<40||u32(18)!=256||(height!=256&&height!=-256)||u16(26)!=1||u16(28)!=24||u32(30)!=0||start<54||start>4096)return false;
 out.resize(256*256);uint8_t row[768];
 for(int stored=0;stored<256;++stored){int y=height>0?255-stored:stored;if(!read(start+stored*768,row,sizeof row)){out.clear();return false;}
  for(int x=0;x<256;++x){auto b=row+3*x;out[y*256+x]=uint16_t((b[2]>>3)<<11|(b[1]>>2)<<5|(b[0]>>3));}}
 return true;
}
}
