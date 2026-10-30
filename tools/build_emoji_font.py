#!/usr/bin/env python3
"""Own monochrome symbols (no third-party emoji artwork), LVGL fallback font."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
codes=[0x2764,0x2705,0x274c,0x26a0,0x1f642,0x1f602,0x1f44d,0x1f44e,0x1f64f,0x1f4cd,0x1f6a9,0x1f50b,0xfe0f]
bitmaps=[];descs=['{0,0,0,0,0,0}'];cmaps=[];offset=0
for idx,code in enumerate(sorted(codes),1):
 im=Image.new('1',(18,18));d=ImageDraw.Draw(im)
 if code in [0x1f642,0x1f602]:
  d.ellipse((1,1,16,16),outline=1,width=2);d.point((6,6),1);d.point((11,6),1);d.arc((4,6,13,13),0,180,fill=1,width=2)
  if code==0x1f602:d.line((0,8,2,11),fill=1,width=2);d.line((15,8,17,11),fill=1,width=2)
 elif code==0x2764:d.polygon([(9,16),(1,7),(1,4),(4,1),(7,1),(9,4),(11,1),(14,1),(17,4),(17,7)],fill=1)
 elif code==0x2705:d.rectangle((0,0,17,17),outline=1);d.line([(3,9),(7,13),(15,4)],fill=1,width=3)
 elif code==0x274c:d.line((3,3,14,14),fill=1,width=3);d.line((14,3,3,14),fill=1,width=3)
 elif code==0x26a0:d.polygon([(9,1),(17,16),(1,16)],outline=1);d.line((9,6,9,10),fill=1,width=2);d.rectangle((8,13,9,14),fill=1)
 elif code in [0x1f44d,0x1f44e]:
  d.polygon([(6,8),(10,2),(12,2),(12,7),(16,7),(15,15),(6,15)],outline=1);d.rectangle((1,8,4,15),fill=1)
  if code==0x1f44e:im=im.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
 elif code==0x1f64f:d.polygon([(8,2),(8,13),(3,16),(2,13)],outline=1);d.polygon([(10,2),(10,13),(15,16),(16,13)],outline=1)
 elif code==0x1f4cd:d.ellipse((3,1,14,12),outline=1,width=2);d.polygon([(4,9),(9,17),(14,9)],outline=1);d.ellipse((7,4,10,7),fill=1)
 elif code==0x1f6a9:d.line((3,1,3,17),fill=1,width=2);d.polygon([(5,1),(16,5),(5,9)],fill=1)
 elif code==0x1f50b:d.rectangle((4,3,14,16),outline=1,width=2);d.rectangle((7,0,11,2),fill=1);d.rectangle((7,7,11,13),fill=1)
 raw=[];bits=list(im.getdata())
 for i in range(0,len(bits),8):raw.append(sum((int(v)>0)<<(7-j) for j,v in enumerate(bits[i:i+8])))
 bitmaps+=raw;descs.append('{%d,%d,18,18,0,-3}'%(offset,0 if code==0xfe0f else 20*16));offset+=len(raw);cmaps.append('{.range_start=%d,.range_length=1,.glyph_id_start=%d,.type=LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY}'%(code,idx))
s='#include <lvgl.h>\nstatic const uint8_t bits[]={'+','.join(map(str,bitmaps))+'};\nstatic const lv_font_fmt_txt_glyph_dsc_t glyphs[]={'+','.join(descs)+'};\nstatic const lv_font_fmt_txt_cmap_t cmaps[]={'+','.join(cmaps)+'};\nstatic lv_font_fmt_txt_dsc_t dsc={.glyph_bitmap=bits,.glyph_dsc=glyphs,.cmaps=cmaps,.cmap_num=13,.bpp=1};\nconst lv_font_t meshcator_emoji_font={.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,.line_height=22,.base_line=4,.dsc=&dsc};\n'
(root/'src/ui/fonts/meshcator_emoji_font.c').write_text(s)
for p in (root/'src/ui/fonts').glob('meshcator_font_*.c'):
 s=p.read_text();s=s.replace('.fallback = NULL,','.fallback = &meshcator_emoji_font,')
 if 'LV_FONT_DECLARE(meshcator_emoji_font);' not in s:s=s.replace('const lv_font_t meshcator_font_', 'const lv_font_t meshcator_font_',1);pos=s.index('/*-----------------\n *  PUBLIC FONT') if '/*-----------------\n *  PUBLIC FONT' in s else s.index('#if LVGL_VERSION_MAJOR >= 8');s=s[:pos]+'LV_FONT_DECLARE(meshcator_emoji_font);\n'+s[pos:]
 p.write_text(s)
