"""Generate additive color studies from the rounded 480 × 222 display SVGs."""
from pathlib import Path
import json, re, html
ROOT=Path(__file__).resolve().parents[1]
base={'bg':'#101819','surface':'#172426','focus':'#253C3C','line':'#355052','text':'#EFF4E8','muted':'#AEC2BD','accent':'#D2E86C','notice':'#E6C572','secondary':'#7CB9B0','readout':'#D9E8D5','subtle':'#557F7B','green':'#9ACCAB'}
themes={
 'warm-orange':{'name':'Warm Orange','description':'Kohle, Kupfer und warmes Orange. Türkis setzt einen kühlen Gegenakzent.', 'colors':dict(zip(base,['#19130F','#261D17','#453025','#69503F','#FFF2E6','#CDB6A3','#FFAA5C','#F7CD71','#79C9C0','#FFE0BA','#A87E5C','#96D7A2']))},
 'neon-purple':{'name':'Neon Purple','description':'Tiefes Violett mit leuchtendem Purple. Eis-Cyan und Pink akzentuieren sparsam.', 'colors':dict(zip(base,['#110E1B','#1D172C','#342348','#584270','#F6EFFF','#BEB0D1','#C58AFF','#FF8BD3','#77DCE8','#E2C9FF','#8872A6','#85E3B0']))}
}
for key,t in themes.items():
 t['colors'].update({'info':t['colors']['secondary'],'warning':'#F7CD71' if key=='warm-orange' else '#FFD166','danger':'#FF8580' if key=='warm-orange' else '#FF859B','success':t['colors']['green']})
def luminance(h):
 v=[int(h[i:i+2],16)/255 for i in (1,3,5)];v=[x/12.92 if x<=.04045 else ((x+.055)/1.055)**2.4 for x in v];return sum(a*b for a,b in zip(v,[.2126,.7152,.0722]))
def ratio(a,b):
 a,b=sorted([luminance(a),luminance(b)]);return (b+.05)/(a+.05)
roles=[('accent','Fokus / Auswahl'),('notice','Neue Nachricht'),('info','Information'),('success','Erfolg / Verbunden'),('warning','Warnung'),('danger','Fehler')]
cards=[]
for key,t in themes.items():
 c=t['colors'];out=ROOT/'assets/display'/key;out.mkdir(exist_ok=True)
 mapping={base[k].upper():c[k] for k in base}
 for p in (ROOT/'assets/display').glob('*.svg'):
  s=re.sub(r'#[0-9a-fA-F]{6}',lambda m:mapping.get(m[0].upper(),m[0]),p.read_text());(out/p.name).write_text(s)
 # Palette is also an independent editable SVG artifact.
 svg=['<svg xmlns="http://www.w3.org/2000/svg" width="720" height="420" viewBox="0 0 720 420">',f'<rect width="720" height="420" rx="20" fill="{c["bg"]}"/>',f'<g font-family="Arial,sans-serif" fill="{c["text"]}"><text x="28" y="42" font-size="24">MeshCator / {t["name"]}</text><text x="28" y="68" font-size="13" fill="{c["muted"]}">Auszeichnungsfarben · Hex-Werte · semantische Rollen</text>']
 for i,(role,label) in enumerate(roles):
  x=28+(i%3)*230;y=92+(i//3)*114
  svg.extend([f'<rect x="{x}" y="{y}" width="204" height="42" rx="10" fill="{c[role]}"/>',f'<text x="{x}" y="{y+65}" font-size="14">{label}</text>',f'<text x="{x}" y="{y+85}" font-size="13" fill="{c["muted"]}">{c[role]}</text>'])
 for i,role in enumerate(['bg','surface','focus','text','muted']):
  x=28+i*138;svg.extend([f'<rect x="{x}" y="337" width="120" height="24" rx="6" fill="{c[role]}" stroke="{c["line"]}"/>',f'<text x="{x}" y="381" font-size="12">{role}</text>',f'<text x="{x}" y="399" font-size="12" fill="{c["muted"]}">{c[role]}</text>'])
 svg.append('</g></svg>');(out/'palette.svg').write_text(''.join(svg))
 checks={role:round(min(ratio(c[role],c[b]) for b in ['bg','surface','focus']),2) for role in ['text','muted',*[r for r,_ in roles]]}
 assert min(checks.values())>=4.5,(key,checks)
 t['text_contrast_min_over_bg_surface_focus']=checks
 swatches=''.join(f'<div class="swatch"><i style="background:{c[r]}"></i><strong>{label}</strong><code>{c[r]}</code></div>' for r,label in roles)
 screens=''.join(f'<figure><figcaption>{html.escape(p.stem[3:].replace("-"," ").title())}</figcaption><img class="screen" width="480" height="222" src="../../assets/display/{key}/{p.name}?v=5" alt="{t["name"]} {p.stem}"></figure>' for p in sorted(out.glob('0*.svg')))
 cards.append(f'<section id="{key}" style="--bg:{c["bg"]};--surface:{c["surface"]};--text:{c["text"]};--muted:{c["muted"]};--accent:{c["accent"]}"><header><small>MESHCATOR / FARBSTUDIE</small><h2>{t["name"]}</h2><p>{t["description"]}</p></header><div class="swatches">{swatches}</div><p class="note">Flächen: {c["bg"]} / {c["surface"]} · Fokusfläche: {c["focus"]} · Text: {c["text"]}</p><a href="../../assets/display/{key}/palette.svg">Farbpalette als SVG öffnen</a><div class="screens">{screens}</div></section>')
(ROOT/'docs/design/color-palettes.json').write_text(json.dumps(themes,indent=2,ensure_ascii=False))
page='''<!doctype html><html lang="de"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MeshCator · Warm Orange & Neon Purple</title><style>*{box-sizing:border-box}body{margin:0;background:#101418;color:#eef2ee;font:16px Arial,sans-serif}main{max-width:1200px;margin:auto;padding:32px 20px}h1{font-size:34px;margin:12px 0}p{line-height:1.6;color:#b8c1c4}nav{display:flex;flex-wrap:wrap;gap:10px;margin:24px 0}button,nav a{border:1px solid #586168;border-radius:12px;background:#20292d;color:#eef2ee;padding:12px;text-decoration:none;cursor:pointer}section{background:var(--bg);color:var(--text);border-radius:24px;padding:24px;margin:24px 0}section p{color:var(--muted)}section a{color:var(--accent)}small{color:var(--accent);letter-spacing:2px;font-size:10px}h2{font-size:30px;margin:12px 0}.swatches{display:grid;grid-template-columns:repeat(auto-fit,minmax(130px,1fr));gap:16px}.swatch{display:flex;flex-direction:column;gap:8px;font-size:12px}.swatch i{height:32px;border-radius:9px}.swatch code{color:var(--muted)}.note{font-size:12px}.screens{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:24px;margin-top:28px}figure{margin:0;min-width:0;overflow:auto}figcaption{font-size:12px;color:var(--muted);margin-bottom:10px}.screen{display:block;max-width:100%;height:auto;border-radius:8px}.native .screen{max-width:none}.native .screens{grid-template-columns:1fr}footer{font-size:13px;color:#b8c1c4}</style><main><small>MESHCATOR / DREI FARBWELTEN</small><h1>Gleiche Form. Neuer Charakter.</h1><p>Zwei zusätzliche Farbvarianten für die gerundeten 480 × 222 Display-Entwürfe. Die bisherige Petrol/Limette-Version bleibt erhalten.</p><nav><button onclick="document.querySelectorAll('section').forEach(s=>s.hidden=false)">Beide vergleichen</button><button onclick="document.querySelectorAll('section').forEach(s=>s.hidden=s.id!=='warm-orange')">Warm Orange</button><button onclick="document.querySelectorAll('section').forEach(s=>s.hidden=s.id!=='neon-purple')">Neon Purple</button><button onclick="document.body.classList.toggle('native')">1:1 umschalten</button><a href="preview.html?v=4">Petrol / Limette</a></nav>'''+''.join(cards)+'''<footer>Die Auszeichnungsfarben erreichen als Text auf den drei definierten dunklen Flächen mindestens 4,5:1 Kontrast. Farben werden durch Text und Symbole ergänzt. Neon wird durch Farbsättigung erzeugt, ohne Glow, damit kleine Schrift scharf bleibt. Alle Daten sind Beispiele. Hardware-Test ausstehend. <a href="color-palettes.json">Design-Tokens als JSON</a></footer></main></html>'''
(ROOT/'docs/design/color-variants.html').write_text(page)
print('14 Display-SVGs, 2 Paletten-SVGs, JSON-Tokens und Vergleichsgalerie erstellt; Textkontraste >= 4.5:1.')
