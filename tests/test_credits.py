import json,re,subprocess,sys,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class Credits(unittest.TestCase):
 def test_notices_and_direct_dependencies(self):
  entries=json.loads((ROOT/'LICENSES/components.json').read_text())
  names={e['name'] for e in entries}
  for e in entries:
   with self.subTest(component=e['name']):
    self.assertTrue(e['author'] and e['license'])
    if e['notice']:
     self.assertGreater((ROOT/'LICENSES'/e['notice']).stat().st_size,100)
    else:
     self.assertEqual(e['license'],'Public Domain')
  text=(ROOT/'platformio.ini').read_text().split('lib_deps =',1)[1].split('; Device-specific',1)[0]
  for line in text.splitlines():
   line=line.strip()
   if not line:continue
   name='RadioLib' if 'github.com/jgromes/RadioLib' in line else line.split('/',1)[1].split(' @')[0]
   self.assertIn(name,names)
 def test_generated_credits_current(self):
  path=ROOT/'src/core/Credits.h';old=path.read_bytes()
  subprocess.run([sys.executable,str(ROOT/'tools/generate_credits.py')],check=True)
  self.assertEqual(old,path.read_bytes())
if __name__=='__main__':unittest.main()
