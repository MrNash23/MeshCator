#!/usr/bin/env python3
"""Render Public Domain Natural Earth geometry to compact semantic XYZ tiles.
Requires Pillow. No third-party rendered tiles or online map service is used.
Palette is applied on the device, so all four MeshCator themes share one atlas.
"""
import json, math, hashlib, zlib
from pathlib import Path
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parents[1]
SOURCES=['ne_50m_land.geojson','ne_50m_lakes.geojson']
def polygons(name):
 for f in json.loads((ROOT/'assets/maps'/name).read_text())['features']:
  g=f['geometry']
  if g['type']=='Polygon':yield g['coordinates']
  elif g['type']=='MultiPolygon':yield from g['coordinates']
def project(point,size):
 lon,lat=point[:2];lat=math.radians(max(-85.05112878,min(85.05112878,lat)))
 return ((lon+180)*size/360,(1-math.asinh(math.tan(lat))/math.pi)*size/2)
def pack(raw):
 packed=Image.frombytes("L",(256,256),raw).point(lambda x:255*x).convert("1").tobytes()
 return zlib.compress(packed,9)

land=list(polygons(SOURCES[0]));lakes=list(polygons(SOURCES[1]));atlas=bytearray();entries=[]
for z in range(5):
 size=256*(1<<z);im=Image.new('L',(size,size),0);draw=ImageDraw.Draw(im)
 for poly in land:
  draw.polygon([project(p,size) for p in poly[0]],fill=1)
  for hole in poly[1:]:draw.polygon([project(p,size) for p in hole],fill=0)
 for poly in lakes:
  draw.polygon([project(p,size) for p in poly[0]],fill=0)
  for hole in poly[1:]:draw.polygon([project(p,size) for p in hole],fill=1)
 for y in range(1<<z):
  for x in range(1<<z):
   raw=im.crop((x*256,y*256,(x+1)*256,(y+1)*256)).tobytes();data=pack(raw)
   entries.append((len(atlas),len(data)));atlas.extend(data)
 print('Zoom',z,'atlas bytes',len(atlas),flush=True)
out='// Generated from Natural Earth public-domain land/lakes. tools/build_map_tiles.py\n#pragma once\n#include <cstdint>\nnamespace mc {\nstruct WorldTile { uint32_t offset; uint16_t length; };\ninline constexpr WorldTile worldTiles[]={\n'
out+=''.join('{%d,%d},\n'%p for p in entries)+'};\ninline constexpr uint8_t worldTileBytes[]={\n'
out+=''.join(','.join(str(v) for v in atlas[i:i+32])+',\n' for i in range(0,len(atlas),32))+'};\n}\n'
(ROOT/'src/maps/WorldTiles.h').write_text(out)
manifest={'license':'Public Domain','terms':'https://www.naturalearthdata.com/about/terms-of-use/','zoom_levels':[0,1,2,3,4],'tile_count':len(entries),'encoding':'zlib-compressed 1-bit semantic mask, MSB-first (0 water, 1 land)','encoded_bytes':len(atlas),'sources':{n:hashlib.sha256((ROOT/'assets/maps'/n).read_bytes()).hexdigest() for n in SOURCES}}
(ROOT/'assets/maps/tiles-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
