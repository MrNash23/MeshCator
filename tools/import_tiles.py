#!/usr/bin/env python3
"""Convert your own XYZ tiles to MeshCator's bounded offline BMP format.
Requires Pillow. This script does not fetch any map data.
Usage: python tools/import_tiles.py source_xyz /Volumes/SD/meshcator/tiles
"""
import argparse
from pathlib import Path
from PIL import Image
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('source',type=Path)
parser.add_argument('destination',type=Path)
args=parser.parse_args()
count=0
for path in args.source.glob('*/*/*'):
 if not path.is_file() or path.suffix.lower() not in {'.png','.jpg','.jpeg','.webp','.bmp'}:continue
 parts=path.relative_to(args.source).parts
 try:z,x,y=int(parts[0]),int(parts[1]),int(Path(parts[2]).stem)
 except ValueError:continue
 if not (0<=z<=18 and 0<=x<2**z and 0<=y<2**z):raise ValueError('Invalid XYZ coordinate: '+str(path))
 with Image.open(path) as tile:
  if tile.size!=(256,256):raise ValueError('Expected 256x256: '+str(path))
  dest=args.destination/str(z)/str(x)/(str(y)+'.bmp')
  dest.parent.mkdir(parents=True,exist_ok=True)
  if dest.exists():raise FileExistsError('Will not overwrite existing tile: '+str(dest))
  temp=dest.with_suffix('.tmp')
  tile.convert('RGB').save(temp,format='BMP')
  temp.replace(dest)
  count+=1
print(f'Converted {count} tiles. Copy the tiles directory to SD:/meshcator/tiles/.')
