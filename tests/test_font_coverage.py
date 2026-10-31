"""Audit real generated glyphs in every UI font, not just requested converter ranges."""
import re
import json
import unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
class Coverage(unittest.TestCase):
 def test_languages_and_catalog(self):
  catalog=(ROOT/'src/core/I18n.cpp').read_text()
  texts=re.findall(r'"((?:[^"\\]|\\.)*)"',catalog)
  required={ord(c) for text in texts for c in text if ord(c)>=32}
  required.update(map(ord,'ÄÖÜäöüßẞÀÂÆÇÉÈÊËÎÏÔŒÙÛÜŸàâæçéèêëîïôœùûüÿÁÉÍÓÚÜÑáéíóúüñ¿¡’'))
  for size in (10,12,14,16,18,24,28,36):
   with self.subTest(size=size):
    source=(ROOT/f'src/ui/fonts/meshcator_font_{size}.c').read_text()
    glyphs={int(code,16) for code in re.findall(r'U\+([0-9A-Fa-f]+)',source)}
    self.assertFalse(required-glyphs,[(chr(c),hex(c)) for c in sorted(required-glyphs)])
    self.assertIn('.fallback = &meshcator_emoji_font',source)
 def test_emoji_glyphs(self):
  manifest=json.loads((ROOT/'assets/fonts/noto-emoji/manifest.json').read_text())
  source=(ROOT/'src/ui/fonts/meshcator_emoji_font.c').read_text()
  glyphs={int(code,16) for code in re.findall(r'U\+([0-9A-Fa-f]+)',source)}
  self.assertFalse(set(map(ord,manifest['symbols']))-glyphs)
  picker=(ROOT/'src/core/Emoji.h').read_text().split('emojis[]={',1)[1].split('};',1)[0]
  required=set(map(ord,''.join(re.findall(r'"([^"]*)"',picker))))-{0xfe0f}
  self.assertFalse(required-glyphs)
  self.assertEqual(len(re.findall(r'"([^"]*)"',picker)),160)
  self.assertIn('.get_glyph_dsc = emoji_glyph',source)
 def test_german_spellings(self):
  source=(ROOT/'src/core/I18n.cpp').read_text()
  for word in ('Gespraeche','Kanaele','gehoert','Schliessen','Oeffnen','Auswaehlen','Menue','Baender','ungueltige','uebergeben'):
   self.assertNotIn(word,source)
if __name__=='__main__':unittest.main()
