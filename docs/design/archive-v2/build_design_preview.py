from pathlib import Path
from html import escape
from math import sin, cos, radians
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets/display'
C={'bg':'#000000','surface':'#100D16','focus':'#292033','line':'#43344F','text':'#FFF2E8','muted':'#C7B8D4','orange':'#FF9C32','pink':'#F49AC2','lavender':'#ADA3FF','peach':'#FFC794','plum':'#B77CAB','green':'#A7DEBD'}
def rect(x,y,w,h,c,extra=''):return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{C.get(c,c)}" {extra}/>'
def line(x,y,a,b,c='line'):return f'<path d="M{x} {y}L{a} {b}" fill="none" stroke="{C.get(c,c)}"/>'
def text(x,y,s,size=14,c='text',mono=False):return f'<text x="{x}" y="{y}" fill="{C.get(c,c)}" font-size="{size}" font-family="{ "Menlo,monospace" if mono else "Inter,Arial,sans-serif"}">{escape(s)}</text>'
def label(x,y,s,c='bg',size=10):return f'<text x="{x}" y="{y}" fill="{C.get(c,c)}" font-size="{size}" font-weight="700" font-family="Arial Narrow,Arial,sans-serif">{escape(s)}</text>'
def circle(x,y,r,c='lavender'):return f'<circle cx="{x}" cy="{y}" r="{r}" fill="none" stroke="{C.get(c,c)}"/>'
def pill(x,y,w,h,s,c='lavender',size=10):return rect(x,y,w,h,c,f'rx="{h/2}"')+label(x+9,y+h/2+size*.35,s,size=size)
def frame(mode,hints):
 s=rect(0,0,480,222,'bg')
 # Swept top-left elbow; all colored frame regions carry navigation or status.
 s+='<path d="M4 65V24Q4 4 24 4H91V24H32Q20 24 20 36V65Z" fill="#FF9C32"/>'+label(24,18,'MESHCATOR',size=9)
 tabs=[('Chats',95,57),('Kontakte',156,75),('Scan',235,50),('Karte',289,54),('Tools',347,53)]
 for m,x,w in tabs:
  s+=rect(x,4,w,20,'orange' if m==mode else ('pink' if m=='Kontakte' else 'lavender'))+label(x+6,18,m.upper(),size=9)
  if m==mode:s+=rect(x+5,26,w-10,2,'orange')
 s+=pill(404,4,72,20,'14:32','peach')
 s+=rect(4,69,16,69,'pink')+rect(4,142,16,40,'lavender')
 s+='<path d="M4 186H20V190Q20 198 30 198H94V204H25Q4 204 4 186Z" fill="#FF9C32"/>'
 s+=rect(98,198,135,6,'lavender')+rect(237,198,98,6,'pink')+rect(339,198,69,6,'orange')+rect(412,198,64,6,'plum')
 s+=text(28,217,hints,10,'muted')
 return s

def save(name,s):
 (OUT/f'{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="480" height="222" viewBox="0 0 480 222" role="img"><title>MeshCator – {name} – Design 02 – Beispieldaten</title>{s}</svg>')

def section(x,y,w,title,c='lavender'):return rect(x,y,w,17,c,'rx="8"')+label(x+8,y+12,title)

s=frame('Chats','Enter Öffnen   Enter lang Aktionen   / Suche   Space Modus')+line(180,36,180,190)
for i,(name,msg) in enumerate([('#berlin','Mia: bin am Tor …'),('Public','Jonas: ok, danke'),('Tracy','Du: bis gleich ✓')]):
 y=36+i*49
 s+=section(28,y,143,name.upper(),'orange' if i==0 else 'lavender')+text(34,y+34,msg,12,'text' if i==0 else 'muted')
 if i==0:s+=pill(149,y+1,21,15,'2','pink',9)
s+=text(192,48,'#berlin',18)+pill(367,34,105,18,'3 HOPS · 78%','peach')+rect(192,57,280,3,'pink')+text(192,79,'Mia',12,'pink')+text(435,79,'14:28',10,'muted')+text(192,101,'Bin am Tor, wer kommt noch?',14)+rect(275,118,197,42,'focus','rx="14"')+text(286,137,'Ich, in 10 min.',14)+text(389,153,'14:30 · ✓ 3×',10,'muted')+text(192,184,'Nachricht schreiben …',12,'muted');save('01-chats',s)

s=frame('Scan','Enter Details   A Lokal   B Flood   Space Modus')
# Segmented radial display: range, not decorative numbers.
for r in (28,48):s+=circle(102,111,r,'line')
for start,col in [(5,'orange'),(65,'pink'),(125,'lavender'),(185,'peach'),(245,'pink'),(305,'lavender')]:
 a,b=radians(start),radians(start+52);x1,y1=102+64*cos(a),111+64*sin(a);x2,y2=102+64*cos(b),111+64*sin(b)
 s+=f'<path d="M{x1} {y1}A64 64 0 0 1 {x2} {y2}" fill="none" stroke="{C[col]}" stroke-width="8"/>'
s+=line(48,111,156,111)+line(102,57,102,165)+line(102,111,139,75,'orange')+text(97,116,'+',16,'peach')+text(126,84,'◆',13,'orange')+text(73,137,'●',12,'pink')+text(98,41,'N',10,'muted')+text(62,191,'RADIUS 2 km',10,'muted',True)+section(185,35,287,'12 NODES / LETZTE 15 MIN','orange')
for i,(a,b) in enumerate([('◆ RPT-Mitte','−7 dB  1H  12s'),('● Tracy','−11 dB  2H  3m'),('▣ Room-Kiez','−4 dB  1H  5m'),('◎ Wetter-01','−14 dB  3H  9m')]):
 y=60+i*28;s+=rect(185,y,287,24,'focus' if i==0 else 'surface','rx="12"')+rect(185,y,6,24,'orange' if i==0 else 'lavender','rx="3"')+text(198,y+16,a,12)+text(350,y+16,b,10,'peach' if i==0 else 'muted',True)
s+=pill(185,177,112,16,'SCAN AKTIV','pink',9)+text(345,189,'GPS FIX · 78%',10,'muted');save('02-scan',s)

s=frame('','Enter Entsperren')+section(28,35,127,'SYSTEM BEREIT','lavender')+pill(373,35,99,17,'GESPERRT','pink')+text(170,92,'14:32',44,'peach')+text(164,115,'Freitag, 30. Oktober',14,'muted')
for x,l,v,col in [(28,'AKKU','78 %','orange'),(141,'MESH','−8 dB','lavender'),(254,'GPS','8 SAT','peach'),(367,'NEU','2 MSG','pink')]:
 s+=section(x,135,105,l,col)+text(x+8,180,v,24,col)
save('03-lock',s)

s=frame('','Enter Ändern   Backspace Zurück   / Einstellung suchen')+line(171,36,171,190)
for i,a in enumerate(['Identität','Funk','Kanäle','Display','Hinweise']):
 y=35+i*31;s+=pill(28,y,133,24,a.upper(),'orange' if i==1 else 'lavender',11)
for i,(a,b) in enumerate([('Preset','EU 868 Narrow'),('Frequenz','869.618 MHz'),('Bandbreite','62.5 kHz'),('Spreizfaktor','SF8'),('Sendeleistung','22 dBm')]):
 y=35+i*29;s+=text(184,y+17,a,12,'muted')+pill(330,y,142,23,b,'peach' if i==0 else 'pink',11)
s+=text(184,191,'AIRTIME 1 H   2,1 % / 10 %',10,'muted',True);save('04-settings',s)

s=frame('Tools','Space Pause   Enter lang Aktionen   Backspace Zurück')
for x,w,l,v,col in [(28,127,'NOISE / dBm','−112','orange'),(159,94,'MIN 60 S','−118','lavender'),(257,94,'MAX 60 S','−96','pink'),(355,117,'KANAL / MHz','869.618','peach')]:
 s+=section(x,35,w,l,col)+text(x+6,82,v,28 if x<355 else 20,col)
for y,l in [(101,'−90'),(135,'−105'),(169,'−120')]:s+=line(65,y,472,y)+text(28,y+4,l,10,'muted',True)
s+='<path d="M65 166L88 166L98 174L110 162L148 162L163 170L180 164L219 164L235 157L254 160L270 146L280 148L292 108L300 99L308 121L320 135L332 159L368 163L384 173L399 165L439 165L456 159L472 163" fill="none" stroke="#FF9C32" stroke-width="1.5"/>'+text(65,190,'−60 s',10,'muted')+text(445,190,'jetzt',10,'muted');save('05-noise-floor',s)

s=frame('Karte','Encoder Zoom   Pfeile Pan   P Position   Enter Wegpunkt')
# Content clip keeps map paths out of the navigation frame.
s+='<defs><clipPath id="map"><rect x="28" y="33" width="444" height="160"/></clipPath></defs><g clip-path="url(#map)">'
for x in range(28,480,24):s+=line(x,33,x,193)
for y in range(33,194,24):s+=line(28,y,472,y)
s+='<path d="M20 158Q110 112 191 137T350 87T490 66" fill="none" stroke="#332B4A" stroke-width="16"/><path d="M40 25L138 94L204 80L289 147L404 180L480 135M80 201L138 94L209 21M289 147L330 21M404 180L457 21" stroke="#9D7A9D" fill="none" stroke-width="2"/><path d="M138 94L204 80L250 116L289 147" stroke="#FF9C32" stroke-width="2" fill="none" stroke-dasharray="4 3"/>'
s+=circle(250,116,10,'orange')+text(245,120,'▲',11,'orange')+text(131,90,'◆',14,'pink')+'</g>'+section(28,35,164,'KARTE / SCHEMA','lavender')+pill(319,169,153,23,'RPT-Mitte · 640 m','peach',11)+line(36,184,96,184,'text')+text(36,177,'200 m',10,'muted');save('06-map',s)

s=frame('Chats','Enter Chat öffnen   Backspace Schließen')+text(28,135,'#berlin',18)+text(28,160,'Mia: Bin am Tor, wer kommt noch?',14,'muted')+rect(28,35,444,73,'focus','rx="18"')+section(28,35,444,'NACHRICHT / DIREKT','pink')+text(41,73,'Tracy · 2 Hops',12,'peach')+text(431,73,'jetzt',10,'muted')+text(41,95,'Treffpunkt verschoben auf 15:30',16);save('07-notification',s)
icons={'chat':'M3 3h10v8H7l-4 3z','contacts':'M10 5a2 2 0 1 1-4 0a2 2 0 1 1 4 0M3 14v-2c0-4 10-4 10 0v2','scan':'M8 1v2M8 13v2M1 8h2M13 8h2M8 8l4-4M14 8a6 6 0 1 1-6-6','map':'M1 3l5-2 4 2 5-2v12l-5 2-4-2-5 2zM6 1v12M10 3v12','tools':'M2 14l7-7M9 2a4 4 0 0 0 5 5l-3-1-1-3z','battery':'M1 4h12v8H1zM15 6v4M4 6v4M7 6v4M10 6v4','gps':'M8 1v3M8 12v3M1 8h3M12 8h3M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','signal':'M2 14v-3M6 14V8M10 14V5M14 14V2','client':'M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','repeater':'M8 2l6 6-6 6-6-6z','room':'M2 2h12v12H2zM5 5h6v6H5z','sensor':'M14 8a6 6 0 1 1-12 0a6 6 0 1 1 12 0M10 8a2 2 0 1 1-4 0a2 2 0 1 1 4 0'}
for name,d in icons.items():(ROOT/f'assets/icons/{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16"><path d="{d}" stroke="#ADA3FF" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" fill="none"/></svg>')
names=['Chats','Scan / Mesh-Umgebung','Sperrbildschirm','Funk-Einstellungen','Noise Floor','Offline-Karte · Schema','Nachrichten-Banner']
cards=''.join(f'<article><h2>{i+1:02} / {n}</h2><img class="screen" src="../../assets/display/{p.name}" width="480" height="222" alt="{n}"><a href="../../assets/display/{p.name}">SVG öffnen · 480 × 222 px</a></article>' for i,(n,p) in enumerate(zip(names,sorted(OUT.glob('*.svg')))))
(ROOT/'docs/design/preview.html').write_text('''<!doctype html><html lang="de"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MeshCator · Designstudie</title><style>*{box-sizing:border-box}body{margin:0;background:#000000;color:#FFF2E8;font:16px Arial,sans-serif}main{max-width:1120px;margin:auto;padding:48px 32px}header{border-bottom:6px solid #FF9C32;padding-bottom:32px;margin-bottom:32px}h1{font-size:44px;font-weight:500;margin:12px 0}p{color:#C7B8D4;line-height:1.6;max-width:760px}.eyebrow,h2{font:12px monospace;letter-spacing:1px;color:#FF9C32}h2{margin:0 0 16px}button{background:#292033;border:1px solid #B77CAB;border-radius:18px;color:#FFF2E8;padding:10px 16px;cursor:pointer;margin:8px 8px 0 0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:32px}article{overflow:auto}.screen{display:block;border:1px solid #43344F;box-sizing:border-box;max-width:100%;height:auto}a{display:inline-block;color:#C7B8D4;font:12px Arial;margin-top:12px}.large .grid{display:block}.large article{margin-bottom:32px}.native .screen{max-width:none}.large .screen{max-width:none;width:960px;height:444px}.swatches{display:flex;gap:16px;flex-wrap:wrap;margin:32px 0}.swatches span{border-top:6px solid var(--c);padding-top:10px;font:12px monospace}.icons img{width:32px;height:32px;margin:14px}.brand{max-width:480px;width:100%;display:block}</style><main><header><div class="eyebrow">MESHCATOR / DESIGNSTUDIE 02</div><h1>Mesh trifft Sternenflotte.</h1><p>Standalone-Mesh-Kommunikation für den LilyGo T-LoRa Pager. Maßhaltige Display-Entwürfe nach DESIGN.md: 480 × 222 Pixel, Segmentierte Navigation, geschwungene Rahmen und warme Instrumentfarben. Alle Daten sind illustrative Beispiele; die Karte ist ein Schema.</p><button onclick="document.body.className=''">Ans Fenster anpassen</button><button onclick="document.body.className='native'">1:1 · CSS-Pixel</button><button onclick="document.body.className='large'">2× · Detailansicht</button></header><div class="grid">'''+cards+'''</div><header style="margin-top:48px"><h2>IDENTITÄT / LOGO</h2><img class="brand" src="../../assets/brand/meshcator-logo.png" alt="MeshCator Logo"><p>Orange markiert die Auswahl, Rosa Nachrichten und Lavendel die Navigation. Das bestehende Logo bleibt als Markenreferenz erhalten. Das Logo verbindet Mesh-Knoten, ein M und einen Scan-Ring.</p></header><h2>DESIGN-TOKENS</h2><div class="swatches">'''+''.join(f'<span style="--c:{v}">{k}<br>{v}</span>' for k,v in C.items())+'''</div><h2>12 EIGENE UI-GLYPHEN / 16-PX-RASTER</h2><div class="icons">'''+''.join(f'<img src="../../assets/icons/{n}.svg" title="{n}" alt="{n}">' for n in icons)+'''</div><p>Native SVGs für die UI-Planung; vor Firmware-Einsatz in LVGL-Glyphen oder Bitmaps überführen. Schriftvorschau verwendet lokal vorhandene Inter/Arial bzw. Menlo. Font-Metrik, Sonnenlicht-Lesbarkeit und Encoder-Bedienung müssen auf Hardware geprüft werden.</p></main></html>''')
print('7 display SVGs, 12 icon SVGs, preview.html created')
