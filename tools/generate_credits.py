#!/usr/bin/env python3
"""Generate the on-device attribution pages from the shipped component inventory."""
import json
from pathlib import Path
root=Path(__file__).resolve().parents[1]
entries=json.loads((root/'LICENSES/components.json').read_text())
intro='MeshCator 0.3.25-dev\nMeshCator contributors / MIT\nSource and full license texts:\ngithub.com/MrNash23/MeshCoder\nLICENSES/ + THIRD_PARTY.md\n'
# One component per five-line page. Avoid splitting a name across pages.
text=intro
for e in entries:
 if e['scope']=='source only':continue
 lines=[e['name'],e['version'],e['author'],e['license'],'']
 # The app wraps at 42 columns; wrap this input and pad each component to pages.
 import textwrap
 lines=[part for line in lines for part in (textwrap.wrap(line,42,break_long_words=False) or [''])]
 lines+=['']*((-len(lines))%5)
 text+='\n'.join(lines)+'\n'
text+='SDK component notices and sources:\nLICENSES/ESP-IDF-COPYRIGHT.rst\nArduino / MicroNMEA: LGPL\nESP radio binaries: Apache-2.0\nNo blanket MIT license for deps.'
out='#pragma once\nnamespace mc {\ninline constexpr const char creditsText[]=\n'+''.join(json.dumps(line+'\n',ensure_ascii=False)+'\n' for line in text.splitlines())+';\n}\n'
(root/'src/core/Credits.h').write_text(out)
