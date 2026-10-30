from pathlib import Path
from html import escape
from math import sin, cos, radians
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets/display'
C={'bg':'#101819','surface':'#172426','focus':'#253C3C','line':'#355052','text':'#EFF4E8','muted':'#AEC2BD','accent':'#D2E86C','notice':'#E6C572','secondary':'#7CB9B0','readout':'#D9E8D5','subtle':'#557F7B','green':'#9ACCAB'}
def rect(x,y,w,h,c,extra=''):return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{C.get(c,c)}" {extra}/>'
def line(x,y,a,b,c='line'):return f'<path d="M{x} {y}L{a} {b}" fill="none" stroke="{C.get(c,c)}"/>'
def text(x,y,s,size=14,c='text',mono=False):return f'<text x="{x}" y="{y}" fill="{C.get(c,c)}" font-size="{size}" font-family="{ "Menlo,monospace" if mono else "Inter,Arial,sans-serif"}">{escape(s)}</text>'
def label(x,y,s,c='bg',size=10):return f'<text x="{x}" y="{y}" fill="{C.get(c,c)}" font-size="{size}" font-weight="700" font-family="Inter,Arial,sans-serif">{escape(s)}</text>'
def circle(x,y,r,c='secondary'):return f'<circle cx="{x}" cy="{y}" r="{r}" fill="none" stroke="{C.get(c,c)}"/>'
def pill(x,y,w,h,s,c='secondary',size=10):
 return rect(x,y,w,h,'surface',f'rx="{min(9,h/2)}"')+rect(x+3,y+5,2,max(5,h-10),c,'rx="1"')+label(x+9,y+h/2+size*.35,s,'text',size)
def frame(mode,hints):
 s=rect(0,0,480,222,'bg')+rect(0,0,480,26,'surface')
 s+=label(8,17,'MC /','accent',11)
 tabs=[('Chats',51,49),('Kontakte',104,71),('Scan',179,47),('Karte',230,49),('Tools',283,48)]
 for m,x,w in tabs:
  if m==mode:s+=rect(x,3,w,20,'focus','rx="7"')+rect(x+5,24,w-10,2,'accent')
  s+=label(x+6,17,m,'accent' if m==mode else 'muted',10)
 s+=text(351,17,'14:32',10,'readout',True)+text(405,17,'GPS',9,'secondary')+text(445,17,'78%',10,'text')
 s+='<path d="M8 193Q8 201 16 201H464Q472 201 472 193" fill="none" stroke="#355052"/>'+text(28,217,hints,10,'muted')
 return s

def save(name,s):
 (OUT/f'{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="480" height="222" viewBox="0 0 480 222" role="img"><title>MeshCator – {name} – Design 04 – Beispieldaten</title>{s}</svg>')

def section(x,y,w,title,c='secondary'):
 return rect(x,y,w,17,'surface','rx="8"')+rect(x+3,y+4,2,9,c,'rx="1"')+label(x+9,y+12,title,'text')

s=frame('Chats','Enter Öffnen   Enter lang Aktionen   / Suche   Space Modus')+line(180,36,180,190)
for i,(name,msg) in enumerate([('#berlin','Mia: bin am Tor …'),('Public','Jonas: ok, danke'),('Tracy','Du: bis gleich ✓')]):
 y=36+i*49
 s+=section(28,y,143,name.upper(),'accent' if i==0 else 'secondary')+text(34,y+34,msg,12,'text' if i==0 else 'muted')
 if i==0:s+=pill(149,y+1,21,15,'2','notice',9)
s+=text(192,48,'#berlin',18)+pill(367,34,105,18,'3 HOPS · 78%','readout')+rect(192,57,280,3,'notice')+text(192,79,'Mia',12,'notice')+text(435,79,'14:28',10,'muted')+text(192,101,'Bin am Tor, wer kommt noch?',14)+rect(275,118,197,42,'focus','rx="12"')+text(286,137,'Ich, in 10 min.',14)+text(389,153,'14:30 · ✓ 3×',10,'muted')+text(192,184,'Nachricht schreiben …',12,'muted');save('01-chats',s)

s=frame('Scan','Enter Details   A Lokal   B Flood   Space Modus')
# Fine range rings and cardinal ticks; no segmented decorative surround.
for r in (28,48,64):s+=circle(102,111,r,'line')
for x,y,xx,yy in [(102,43,102,51),(102,171,102,179),(34,111,42,111),(162,111,170,111)]:s+=line(x,y,xx,yy,'secondary')
s+=line(48,111,156,111)+line(102,57,102,165)+line(102,111,139,75,'accent')+text(97,116,'+',16,'readout')+text(126,84,'◆',13,'accent')+text(73,137,'●',12,'notice')+text(98,41,'N',10,'muted')+text(62,191,'RADIUS 2 km',10,'muted',True)+section(185,35,287,'12 NODES / LETZTE 15 MIN','accent')
for i,(a,b) in enumerate([('◆ RPT-Mitte','−7 dB  1H  12s'),('● Tracy','−11 dB  2H  3m'),('▣ Room-Kiez','−4 dB  1H  5m'),('◎ Wetter-01','−14 dB  3H  9m')]):
 y=60+i*28;s+=rect(185,y,287,24,'focus' if i==0 else 'surface','rx="9"')+rect(189,y+5,3,14,'accent' if i==0 else 'secondary','rx="1.5"')+text(198,y+16,a,12)+text(350,y+16,b,10,'readout' if i==0 else 'muted',True)
s+=pill(185,177,112,16,'SCAN AKTIV','notice',9)+text(345,189,'GPS FIX · 78%',10,'muted');save('02-scan',s)

s=frame('','Enter Entsperren')+section(28,35,127,'SYSTEM BEREIT','secondary')+pill(373,35,99,17,'GESPERRT','notice')+text(170,92,'14:32',44,'readout')+text(164,115,'Freitag, 30. Oktober',14,'muted')
for x,l,v,col in [(28,'AKKU','78 %','accent'),(141,'MESH','−8 dB','secondary'),(254,'GPS','8 SAT','readout'),(367,'NEU','2 MSG','notice')]:
 s+=section(x,135,105,l,col)+text(x+8,180,v,24,col)
save('03-lock',s)

s=frame('','Enter Ändern   Backspace Zurück   / Einstellung suchen')+line(171,36,171,190)
for i,a in enumerate(['Identität','Funk','Kanäle','Display','Hinweise']):
 y=35+i*31;s+=pill(28,y,133,24,a.upper(),'accent' if i==1 else 'secondary',11)
for i,(a,b) in enumerate([('Preset','EU 868 Narrow'),('Frequenz','869.618 MHz'),('Bandbreite','62.5 kHz'),('Spreizfaktor','SF8'),('Sendeleistung','22 dBm')]):
 y=35+i*29;s+=text(184,y+17,a,12,'muted')+pill(330,y,142,23,b,'readout' if i==0 else 'notice',11)
s+=text(184,191,'AIRTIME 1 H   2,1 % / 10 %',10,'muted',True);save('04-settings',s)

s=frame('Tools','Space Pause   Enter lang Aktionen   Backspace Zurück')
for x,w,l,v,col in [(28,127,'NOISE / dBm','−112','accent'),(159,94,'MIN 60 S','−118','secondary'),(257,94,'MAX 60 S','−96','notice'),(355,117,'KANAL / MHz','869.618','readout')]:
 s+=section(x,35,w,l,col)+text(x+6,82,v,28 if x<355 else 20,col)
for y,l in [(101,'−90'),(135,'−105'),(169,'−120')]:s+=line(65,y,472,y)+text(28,y+4,l,10,'muted',True)
s+='<path d="M65 166L88 166L98 174L110 162L148 162L163 170L180 164L219 164L235 157L254 160L270 146L280 148L292 108L300 99L308 121L320 135L332 159L368 163L384 173L399 165L439 165L456 159L472 163" fill="none" stroke="#D2E86C" stroke-width="1.5"/>'+text(65,190,'−60 s',10,'muted')+text(445,190,'jetzt',10,'muted');save('05-noise-floor',s)

s=frame('Karte','Encoder Zoom   Pfeile Pan   P Position   Enter Wegpunkt')
# Content clip keeps map paths out of the navigation frame.
s+='<defs><clipPath id="map"><rect x="28" y="33" width="444" height="160"/></clipPath></defs><g clip-path="url(#map)">'
for x in range(28,480,24):s+=line(x,33,x,193)
for y in range(33,194,24):s+=line(28,y,472,y)
s+='<path d="M20 158Q110 112 191 137T350 87T490 66" fill="none" stroke="#203F46" stroke-width="16"/><path d="M40 25L138 94L204 80L289 147L404 180L480 135M80 201L138 94L209 21M289 147L330 21M404 180L457 21" stroke="#698C87" fill="none" stroke-width="2"/><path d="M138 94L204 80L250 116L289 147" stroke="#D2E86C" stroke-width="2" fill="none" stroke-dasharray="4 3"/>'
s+=circle(250,116,10,'accent')+text(245,120,'▲',11,'accent')+text(131,90,'◆',14,'notice')+'</g>'+section(28,35,164,'KARTE / SCHEMA','secondary')+pill(319,169,153,23,'RPT-Mitte · 640 m','readout',11)+line(36,184,96,184,'text')+text(36,177,'200 m',10,'muted');save('06-map',s)

s=frame('Chats','Enter Chat öffnen   Backspace Schließen')+text(28,135,'#berlin',18)+text(28,160,'Mia: Bin am Tor, wer kommt noch?',14,'muted')+rect(28,35,444,73,'focus','rx="12"')+section(28,35,444,'NACHRICHT / DIREKT','notice')+text(41,73,'Tracy · 2 Hops',12,'readout')+text(431,73,'jetzt',10,'muted')+text(41,95,'Treffpunkt verschoben auf 15:30',16);save('07-notification',s)
icons={'chat':'M3 3h10v8H7l-4 3z','contacts':'M10 5a2 2 0 1 1-4 0a2 2 0 1 1 4 0M3 14v-2c0-4 10-4 10 0v2','scan':'M8 1v2M8 13v2M1 8h2M13 8h2M8 8l4-4M14 8a6 6 0 1 1-6-6','map':'M1 3l5-2 4 2 5-2v12l-5 2-4-2-5 2zM6 1v12M10 3v12','tools':'M2 14l7-7M9 2a4 4 0 0 0 5 5l-3-1-1-3z','battery':'M1 4h12v8H1zM15 6v4M4 6v4M7 6v4M10 6v4','gps':'M8 1v3M8 12v3M1 8h3M12 8h3M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','signal':'M2 14v-3M6 14V8M10 14V5M14 14V2','client':'M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','repeater':'M8 2l6 6-6 6-6-6z','room':'M2 2h12v12H2zM5 5h6v6H5z','sensor':'M14 8a6 6 0 1 1-12 0a6 6 0 1 1 12 0M10 8a2 2 0 1 1-4 0a2 2 0 1 1 4 0'}
for name,d in icons.items():(ROOT/f'assets/icons/{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16"><path d="{d}" stroke="#7CB9B0" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" fill="none"/></svg>')
names=['Chats','Scan / Mesh-Umgebung','Sperrbildschirm','Funk-Einstellungen','Noise Floor','Offline-Karte · Schema','Nachrichten-Banner']
cards=''.join(f'<article><h2>{i+1:02} / {n}</h2><img class="screen" src="../../assets/display/{p.name}?v=4" width="480" height="222" alt="{n}"><a href="../../assets/display/{p.name}?v=4">SVG öffnen · 480 × 222 px</a></article>' for i,(n,p) in enumerate(zip(names,sorted(OUT.glob('*.svg')))))
(ROOT/'docs/design/preview.html').write_text('''<!doctype html><html lang="de"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MeshCator · Designstudie 04</title><style>*{box-sizing:border-box}body{margin:0;background:#101819;color:#EFF4E8;font:16px Arial,sans-serif}main{max-width:1120px;margin:auto;padding:48px 32px}header{border-bottom:2px solid #D2E86C;padding-bottom:32px;margin-bottom:32px}h1{font-size:44px;font-weight:500;margin:12px 0}p{color:#AEC2BD;line-height:1.6;max-width:760px}.eyebrow,h2{font:12px monospace;letter-spacing:1px;color:#D2E86C}h2{margin:0 0 16px}button{background:#253C3C;border:1px solid #557F7B;border-radius:12px;color:#EFF4E8;padding:10px 16px;cursor:pointer;margin:8px 8px 0 0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:32px}article{overflow:auto}.screen{display:block;border:1px solid #355052;box-sizing:border-box;max-width:100%;height:auto}a{display:inline-block;color:#AEC2BD;font:12px Arial;margin-top:12px}.large .grid{display:block}.large article{margin-bottom:32px}.native .screen{max-width:none}.large .screen{max-width:none;width:960px;height:444px}.swatches{display:flex;gap:16px;flex-wrap:wrap;margin:32px 0}.swatches span{border-top:6px solid var(--c);padding-top:10px;font:12px monospace}.icons img{width:32px;height:32px;margin:14px}.brand{max-width:480px;width:100%;display:block}</style><main><header><div class="eyebrow">MESHCATOR / DESIGNSTUDIE 04</div><h1>Verbindung. Orientierung. Übersicht.</h1><a href="color-variants.html?v=5">Neue Farbstudien: Warm Orange und Neon Purple →</a><p>Standalone-Mesh-Kommunikation für den LilyGo T-LoRa Pager. Maßhaltige Display-Entwürfe nach DESIGN.md: 480 × 222 Pixel, Eigenständige Feldgerät-Optik: Graphit, Petrol und Limette, offene Flächen, weich gerundete Karten und klare Navigation. Alle Daten sind illustrative Beispiele; die Karte ist ein Schema.</p><button onclick="document.body.className=''">Ans Fenster anpassen</button><button onclick="document.body.className='native'">1:1 · CSS-Pixel</button><button onclick="document.body.className='large'">2× · Detailansicht</button></header><div class="grid">'''+cards+'''</div><header style="margin-top:48px"><h2>IDENTITÄT / LOGO</h2><img class="brand" src="../../assets/brand/meshcator-logo.png" alt="MeshCator Logo"><p>Limette markiert den Fokus, Petrol strukturiert die Ansichten und Sand signalisiert neue Nachrichten. Das bestehende Logo bleibt als Markenreferenz erhalten. Das Logo verbindet Mesh-Knoten, ein M und einen Scan-Ring.</p></header><h2>DESIGN-TOKENS</h2><div class="swatches">'''+''.join(f'<span style="--c:{v}">{k}<br>{v}</span>' for k,v in C.items())+'''</div><h2>12 EIGENE UI-GLYPHEN / 16-PX-RASTER</h2><div class="icons">'''+''.join(f'<img src="../../assets/icons/{n}.svg?v=4" title="{n}" alt="{n}">' for n in icons)+'''</div><p>Native SVGs für die UI-Planung; vor Firmware-Einsatz in LVGL-Glyphen oder Bitmaps überführen. Schriftvorschau verwendet lokal vorhandene Inter/Arial bzw. Menlo. Font-Metrik, Sonnenlicht-Lesbarkeit und Encoder-Bedienung müssen auf Hardware geprüft werden.</p></main></html>''')
print('7 display SVGs, 12 icon SVGs, preview.html created')
