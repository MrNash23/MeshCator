from pathlib import Path
from html import escape
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'assets/display'
C={'bg':'#05080C','surface':'#0B121A','focus':'#122030','line':'#1E3246','text':'#E4EEF5','muted':'#8BA3B6','cyan':'#47C8E0','amber':'#F0A23B','green':'#3CCB8A'}
def rect(x,y,w,h,c,extra=''): return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{C.get(c,c)}" {extra}/>'
def line(x,y,a,b,c='line'): return f'<path d="M{x} {y}L{a} {b}" fill="none" stroke="{C.get(c,c)}"/>'
def text(x,y,s,size=14,c='text',mono=False): return f'<text x="{x}" y="{y}" fill="{C.get(c,c)}" font-size="{size}" font-family="{ "Menlo,monospace" if mono else "Inter,Arial,sans-serif"}">{escape(s)}</text>'
def circle(x,y,r,c='cyan'): return f'<circle cx="{x}" cy="{y}" r="{r}" fill="none" stroke="{C.get(c,c)}"/>'
def base(mode,hints):
 s=rect(0,0,480,222,'bg')+rect(0,0,480,20,'surface')+line(0,20,480,20)+line(0,202,480,202)
 for x,m in [(8,'Chats'),(54,'Kontakte'),(122,'Scan'),(166,'Karte'),(211,'Tools')]: s+=text(x,14,m.upper() if m==mode else m,10,'cyan' if m==mode else 'muted')
 s+=text(321,14,'14:32',10,'muted',True)+text(379,14,'GPS',10,'green')+text(441,14,'78%',10,'text')
 for i in range(4):s+=rect(410+i*6,7,4,7,'cyan' if i<3 else 'line')
 return s+text(8,216,hints,10,'muted')
def save(name,s):
 (OUT/f'{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="480" height="222" viewBox="0 0 480 222" role="img"><title>MeshCator – {name} – Beispieldaten</title>{s}</svg>')
def focus(x,y,w,h):return f'<path d="M{x+6} {y}H{x}V{y+6}M{x+w-6} {y}H{x+w}V{y+6}M{x} {y+h-6}V{y+h}H{x+6}M{x+w-6} {y+h}H{x+w}V{y+h-6}" fill="none" stroke="#47C8E0"/>'
s=base('Chats','Enter Öffnen    Enter lang Aktionen    / Suchen    Space Modus')+rect(0,21,176,181,'surface')+line(176,20,176,202)
for i,(a,b) in enumerate([('#berlin','Mia: bin am Tor …'),('Public','Jonas: ok, danke'),('Tracy','Du: bis gleich ✓')]):
 y=28+i*48
 if i==0:s+=rect(6,y,164,42,'focus')+focus(6,y,164,42)
 s+=text(14,y+17,a)+text(14,y+34,b,12,'muted')
s+=rect(146,36,16,16,'amber', 'rx="3"')+text(150,48,'2',11,'bg')+text(188,42,'#berlin',18)+text(384,42,'3 HOPS',10,'muted',True)+line(188,53,468,53)+text(188,74,'Mia',12,'cyan')+text(434,74,'14:28',10,'muted')+text(188,96,'Bin am Tor, wer kommt noch?',14)+rect(276,118,192,43,'focus','rx="4"')+rect(466,118,2,43,'#3474C8')+text(287,138,'Ich, in 10 min.')+text(385,154,'14:30 · ✓ 3×',10,'muted')+text(188,187,'Nachricht schreiben …',12,'muted')
save('01-chats',s)
s=base('Scan','Enter Details    A Advert lokal    B Advert Flood    Space Modus')+line(176,20,176,202)
for x in range(8,176,24):s+=line(x,26,x,196)
for y in range(28,201,24):s+=line(8,y,170,y)
for r in (24,48,70):s+=circle(88,110,r,'line')
s+=line(18,110,158,110)+line(88,40,88,180)+f'<path d="M88 110L137 60A70 70 0 0 1 158 110Z" fill="#47C8E0" opacity=".08"/>'+line(88,110,137,60,'cyan')+text(84,35,'N',10,'muted')+text(81,191,'S',10,'muted')+text(83,115,'+',16,'cyan')+text(112,77,'◆',12,'cyan')+text(55,145,'●',11,'green')+text(133,128,'◎',13,'amber')+text(188,40,'NODES 12',12,'text',True)+text(362,40,'LETZTE 15 MIN',10,'muted',True)
for i,(a,b) in enumerate([('◆ RPT-Mitte','−7 dB  1H  12s'),('● Tracy','−11 dB  2H  3m'),('▣ Room-Kiez','−4 dB  1H  5m'),('◎ Wetter-01','−14 dB  3H  9m')]):
 y=51+i*29
 if i==0:s+=rect(184,y,288,27,'focus')+focus(184,y,288,27)
 s+=text(190,y+18,a,12)+text(350,y+18,b,10,'muted',True)
s+=text(188,188,'RADIUS 2 km',10,'muted',True)+text(354,188,'SCAN AKTIV',10,'cyan',True)
save('02-scan',s)
s=rect(0,0,480,222,'bg')+text(192,71,'14:32',36)+text(156,95,'Freitag, 30. Oktober',14,'muted')
for x,label,val in [(28,'AKKU','78 %'),(143,'MESH','−8 dB'),(258,'GPS','8 SAT'),(373,'NEU','2 MSG')]:s+=text(x,133,label,10,'muted',True)+text(x,159,val,22,'amber' if label=='NEU' else 'text')
s+=text(175,206,'Enter zum Entsperren',12,'muted');save('03-lock',s)
s=base('','Enter Ändern    Backspace Zurück    / Einstellung suchen')+rect(0,21,176,181,'surface')+line(176,20,176,202)
for i,a in enumerate(['Identität','Funk','Kanäle','Display','Benachrichtig.']):
 y=31+i*32
 if i==1:s+=rect(6,y-1,164,28,'focus')+focus(6,y-1,164,28)
 s+=text(14,y+18,a)
for i,(a,b) in enumerate([('Preset','EU 868 Narrow'),('Frequenz','869.618 MHz'),('Bandbreite','62.5 kHz'),('Spreizfaktor','SF8'),('Sendeleistung','22 dBm')]):
 y=43+i*30;s+=text(188,y,a,12,'muted')+text(350,y,b,12)+line(188,y+9,470,y+9)
s+=text(188,194,'AIRTIME 1 H   2,1 % / 10 %',10,'muted',True);save('04-settings',s)
s=base('Tools','Space Pause    Enter lang Aktionen    Backspace Zurück')
for x,label,val in [(8,'NOISE FLOOR','−112'),(151,'MIN 60 S','−118'),(268,'MAX 60 S','−96'),(378,'KANAL MHz','869.618')]:s+=text(x,39,label,10,'muted',True)+text(x,69,val,28 if x<378 else 18,'cyan' if x==8 else 'text')
s+=text(85,68,'dBm',12,'muted')
for y,label in [(96,'−90'),(133,'−105'),(170,'−120')]:s+=line(45,y,469,y)+text(8,y+4,label,10,'muted',True)
s+='<path d="M45 165L65 165L77 174L90 161L124 161L137 169L150 164L192 164L207 158L233 160L250 145L261 147L273 106L280 99L288 122L296 133L307 158L346 162L366 174L382 164L420 164L440 158L468 163" fill="none" stroke="#47C8E0" stroke-width="1.5"/>'+text(45,192,'−60 s',10,'muted')+text(441,192,'jetzt',10,'muted');save('05-noise-floor',s)
s=base('Karte','Encoder Zoom    Pfeile Verschieben    P Position    Enter Wegpunkt')
for x in range(0,480,24):s+=line(x,21,x,201)
for y in range(34,202,24):s+=line(0,y,480,y)
s+='<path d="M0 156Q100 108 181 137T340 87T480 66" fill="none" stroke="#122D42" stroke-width="20"/><path d="M30 25L128 94L194 80L279 147L394 180L470 135M70 201L128 94L199 21M279 147L320 21M394 180L447 21" stroke="#526878" fill="none" stroke-width="2"/><path d="M128 94L194 80L240 116L279 147" stroke="#47C8E0" stroke-width="2" fill="none" stroke-dasharray="4 3"/>'
s+=circle(240,116,10)+text(235,120,'▲',11,'cyan')+text(121,90,'◆',14,'cyan')+rect(8,28,138,25,'surface')+text(16,45,'SCHEMA · DEMODATEN',10,'muted',True)+rect(312,161,160,32,'surface')+text(321,181,'RPT-Mitte · 640 m',12)+line(16,184,76,184,'text')+text(16,177,'200 m',10,'muted');save('06-map',s)
s=base('Chats','Enter Chat öffnen    Backspace Schließen')+text(20,112,'#berlin',18)+text(20,140,'Mia: Bin am Tor, wer kommt noch?',14,'muted')+rect(0,0,480,63,'focus')+rect(0,0,3,63,'amber')+text(12,22,'● Tracy · DM · 2 Hops',12,'cyan')+text(439,22,'jetzt',10,'muted')+text(12,47,'Treffpunkt verschoben auf 15:30',16)+line(0,63,480,63,'cyan');save('07-notification',s)
icons={'chat':'M3 3h10v8H7l-4 3z','contacts':'M10 5a2 2 0 1 1-4 0a2 2 0 1 1 4 0M3 14v-2c0-4 10-4 10 0v2','scan':'M8 1v2M8 13v2M1 8h2M13 8h2M8 8l4-4M14 8a6 6 0 1 1-6-6','map':'M1 3l5-2 4 2 5-2v12l-5 2-4-2-5 2zM6 1v12M10 3v12','tools':'M2 14l7-7M9 2a4 4 0 0 0 5 5l-3-1-1-3z','battery':'M1 4h12v8H1zM15 6v4M4 6v4M7 6v4M10 6v4','gps':'M8 1v3M8 12v3M1 8h3M12 8h3M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','signal':'M2 14v-3M6 14V8M10 14V5M14 14V2','client':'M12 8a4 4 0 1 1-8 0a4 4 0 1 1 8 0','repeater':'M8 2l6 6-6 6-6-6z','room':'M2 2h12v12H2zM5 5h6v6H5z','sensor':'M14 8a6 6 0 1 1-12 0a6 6 0 1 1 12 0M10 8a2 2 0 1 1-4 0a2 2 0 1 1 4 0'}
for name,d in icons.items():(ROOT/f'assets/icons/{name}.svg').write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 16 16"><path d="{d}" stroke="#47C8E0" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round" fill="none"/></svg>')
names=['Chats','Scan / Mesh-Umgebung','Sperrbildschirm','Funk-Einstellungen','Noise Floor','Offline-Karte · Schema','Nachrichten-Banner']
cards=''.join(f'<article><h2>{i+1:02} / {n}</h2><img class="screen" src="../../assets/display/{p.name}" width="480" height="222" alt="{n}"><a href="../../assets/display/{p.name}">SVG öffnen · 480 × 222 px</a></article>' for i,(n,p) in enumerate(zip(names,sorted(OUT.glob('*.svg')))))
(ROOT/'docs/design/preview.html').write_text('''<!doctype html><html lang="de"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MeshCator · Designstudie</title><style>*{box-sizing:border-box}body{margin:0;background:#05080c;color:#e4eef5;font:16px Arial,sans-serif}main{max-width:1120px;margin:auto;padding:48px 32px}header{border-bottom:1px solid #1e3246;padding-bottom:32px;margin-bottom:32px}h1{font-size:44px;font-weight:500;margin:12px 0}p{color:#8ba3b6;line-height:1.6;max-width:760px}.eyebrow,h2{font:12px monospace;letter-spacing:1px;color:#47c8e0}h2{margin:0 0 16px}button{background:#122030;border:1px solid #526878;color:#e4eef5;padding:10px 16px;cursor:pointer;margin:8px 8px 0 0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:32px}article{overflow:auto}.screen{display:block;border:1px solid #1e3246;box-sizing:border-box;max-width:100%;height:auto}a{display:inline-block;color:#8ba3b6;font:12px Arial;margin-top:12px}.large .grid{display:block}.large article{margin-bottom:32px}.native .screen{max-width:none}.large .screen{max-width:none;width:960px;height:444px}.swatches{display:flex;gap:16px;flex-wrap:wrap;margin:32px 0}.swatches span{border-top:6px solid var(--c);padding-top:10px;font:12px monospace}.icons img{width:32px;height:32px;margin:14px}.brand{max-width:480px;width:100%;display:block}</style><main><header><div class="eyebrow">MESHCATOR / DESIGNSTUDIE 01</div><h1>Ein ruhiges Feldinstrument.</h1><p>Standalone-Mesh-Kommunikation für den LilyGo T-LoRa Pager. Maßhaltige Display-Entwürfe nach DESIGN.md: 480 × 222 Pixel, 20 px Statusleiste, 20 px Tastenhinweise. Alle Daten sind illustrative Beispiele; die Karte ist ein Schema.</p><button onclick="document.body.className=''">Ans Fenster anpassen</button><button onclick="document.body.className='native'">1:1 · CSS-Pixel</button><button onclick="document.body.className='large'">2× · Detailansicht</button></header><div class="grid">'''+cards+'''</div><header style="margin-top:48px"><h2>IDENTITÄT / LOGO</h2><img class="brand" src="../../assets/brand/meshcator-logo.png" alt="MeshCator Logo"><p>Cyan steht für Fokus und Interaktion, Bernstein für Aufmerksamkeit. Das Logo verbindet Mesh-Knoten, ein M und einen Scan-Ring.</p></header><h2>DESIGN-TOKENS</h2><div class="swatches">'''+''.join(f'<span style="--c:{v}">{k}<br>{v}</span>' for k,v in C.items())+'''</div><h2>12 EIGENE UI-GLYPHEN / 16-PX-RASTER</h2><div class="icons">'''+''.join(f'<img src="../../assets/icons/{n}.svg" title="{n}" alt="{n}">' for n in icons)+'''</div><p>Native SVGs für die UI-Planung; vor Firmware-Einsatz in LVGL-Glyphen oder Bitmaps überführen. Schriftvorschau verwendet lokal vorhandene Inter/Arial bzw. Menlo. Font-Metrik, Sonnenlicht-Lesbarkeit und Encoder-Bedienung müssen auf Hardware geprüft werden.</p></main></html>''')
print('7 display SVGs, 12 icon SVGs, preview.html created')
