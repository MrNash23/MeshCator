#pragma once
#include "WorldTiles.h"
#include <vector>
#include <cstddef>
#include <memory>
#include <new>
#ifdef ARDUINO
#include <esp32s3/rom/miniz.h>
#else
#include <zlib.h>
#endif
namespace mc {
inline bool loadWorldTile(int z,int x,int y,std::vector<uint8_t>&pixels){
 if(z<0||z>4||x<0||y<0||x>=(1<<z)||y>=(1<<z))return false;
 size_t index=((1u<<(2*z))-1)/3+y*(1<<z)+x;
 auto tile=worldTiles[index];if(size_t(tile.offset)+tile.length>sizeof(worldTileBytes))return false;
 pixels.resize(8192);
#ifdef ARDUINO
 // The convenience ROM wrapper keeps the large inflater on the task stack.
 // Allocate it on the heap instead; the Arduino loop stack is limited.
 std::unique_ptr<tinfl_decompressor> decoder(new(std::nothrow) tinfl_decompressor);
 if(!decoder){pixels.clear();return false;}
 tinfl_init(decoder.get());
 size_t input=tile.length,output=pixels.size();
 auto result=tinfl_decompress(decoder.get(),worldTileBytes+tile.offset,&input,pixels.data(),pixels.data(),&output,TINFL_FLAG_PARSE_ZLIB_HEADER|TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
 if(result!=TINFL_STATUS_DONE||output!=8192||input!=tile.length){pixels.clear();return false;}
#else
 uLongf length=pixels.size();
 if(uncompress(pixels.data(),&length,worldTileBytes+tile.offset,tile.length)!=Z_OK||length!=8192){pixels.clear();return false;}
#endif
 return true;
}
inline bool worldLand(const std::vector<uint8_t>&tile,int x,int y){
 size_t i=y*256+x;return tile.size()==8192 && ((tile[i/8]>>(7-(i%8)))&1);
}
}
