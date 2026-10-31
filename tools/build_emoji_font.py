#!/usr/bin/env python3
"""Build the licensed Noto Emoji subset; preserve received UTF-8 unchanged."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
def link_fonts():
 for p in (root/'src/ui/fonts').glob('meshcator_font_*.c'):
  s=p.read_text();s=s.replace('.fallback = NULL,','.fallback = &meshcator_emoji_font,')
  if 'LV_FONT_DECLARE(meshcator_emoji_font);' not in s:s=s.replace('const lv_font_t meshcator_font_', 'const lv_font_t meshcator_font_',1);pos=s.index('/*-----------------\n *  PUBLIC FONT') if '/*-----------------\n *  PUBLIC FONT' in s else s.index('#if LVGL_VERSION_MAJOR >= 8');s=s[:pos]+'LV_FONT_DECLARE(meshcator_emoji_font);\n'+s[pos:]
  p.write_text(s)

import sys
if '--link-only' in sys.argv:
 link_fonts()
 sys.exit(0)
import hashlib,json,os,subprocess
manifest=json.loads((root/'assets/fonts/noto-emoji/manifest.json').read_text())
source=root/'assets/fonts/noto-emoji/NotoEmoji.ttf'
assert hashlib.sha256(source.read_bytes()).hexdigest()==manifest['sha256']
output=root/'src/ui/fonts/meshcator_emoji_font.c'
subprocess.run([os.environ.get('FONT_CONVERTER','lv_font_conv'),'--font',str(source),
 '--size',str(manifest['size']),'--bpp',str(manifest['bpp']),'--format','lvgl',
 '--symbols',manifest['symbols'],'--no-kerning','--lv-font-name','meshcator_emoji_font','-o',str(output)],check=True)
s=output.read_text()
# LVGL does not shape color/ZWJ sequences. Render their supported base symbols,
# and make selectors, joiners, skin-tone modifiers and flag tags zero-width.
# This only affects rendering; received/sent text retains every code point.
wrapper="""
static bool emoji_glyph(const lv_font_t *font, lv_font_glyph_dsc_t *out,
                        uint32_t cp, uint32_t next) {
    if(cp==0x200d || cp==0xfe0e || cp==0xfe0f || (cp>=0x1f3fb && cp<=0x1f3ff) ||
       (cp>=0xe0020 && cp<=0xe007f)) {
        out->adv_w=0; out->box_w=0; out->box_h=0; out->ofs_x=0; out->ofs_y=0;
        out->format=LV_FONT_GLYPH_FORMAT_A1; out->is_placeholder=false; out->gid.index=0;
        return true;
    }
    return lv_font_get_glyph_dsc_fmt_txt(font,out,cp,next);
}
"""
s=s.replace('const lv_font_t meshcator_emoji_font = {',wrapper+'\nconst lv_font_t meshcator_emoji_font = {')
s=s.replace('.get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,','.get_glyph_dsc = emoji_glyph,')
output.write_text(s.rstrip()+"\n")
link_fonts()
