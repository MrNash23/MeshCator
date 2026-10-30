from pathlib import Path
from html import escape
R=Path(__file__).resolve().parents[1]
P={'ink':'#161616','paper':'#F2F0E9','yellow':'#F9EF55','purple':'#AA7ED7','lime':'#C5EE6A','pink':'#F0AFDE','blue':'#9BB9E7'}
def box(x,y,w,h,c,r=0):return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{P.get(c,c)}"/>'
def txt(x,y,s,z=14,c='ink',bold=False):return f'<text x="{x}" y="{y}" font-family="Arial,sans-serif" font-size="{z}" font-weight="{700 if bold else 400}" fill="{P.get(c,c)}">{escape(s)}</text>'
def line(x,y,a,b,c='ink'):return f'<path d="M{x} {y}L{a} {b}" stroke="{P.get(c,c)}" opacity=".3"/>'
def shell(mode,dark):
 bg='ink' if dark else 'paper';fg='paper' if dark else 'ink'
 s=box(0,0,480,222,bg)+txt(12,16,'MC / '+mode,11,fg,True)+txt(337,16,'14:32   GPS   78%',11,fg)
 return s
def footer(s,dark,hint='↵ Öffnen     Drehen Auswahl     Space Modus'):
 return s+txt(12,216,hint,10,'paper' if dark else 'ink')
def save(kind,name,s):
 p=R/f'assets/display/study-{kind}/{name}.svg';p.write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="480" height="222" viewBox="0 0 480 222"><title>MeshCator / offene Studie / {kind} / {name}</title>{s}</svg>')
for kind,dark in [('cards',True),('bands',False)]:
 s=shell('CHATS',dark)
 if dark:
  s+=txt(12,51,'Gespräche',29,'paper',True)+txt(352,48,'02 UNGELESEN',10,'lime')
  for i,(name,msg,col,meta) in enumerate([('#berlin','Mia: Bin am Tor.','yellow','2 NEU'),('Tracy','Bis gleich!','purple','DIREKT'),('Public','Jonas: Danke.','lime','KANAL')]):
   x=12+i*156;s+=box(x,65,148,132,col,20)+txt(x+12,87,meta,10)+txt(x+12,116,name,24,'ink',True)+txt(x+12,142,msg,12)+txt(x+12,180,'↗' if i==0 else '→',22)
   if i==0:s+=box(x+89,168,48,19,'ink',9)+txt(x+98,181,'FOKUS',9,'yellow',True)
 else:
  for i,(name,msg,col,meta) in enumerate([('#berlin','Mia: Bin am Tor.','yellow','2 NEU'),('Tracy','Bis gleich!','pink','DIREKT'),('Public','Jonas: Danke.','blue','KANAL')]):
   y=30+i*55;s+=box(8,y,464,52,col,10)+txt(20,y+32,f'0{i+1}',17)+txt(65,y+33,name,29,'ink',True)+line(251,y+9,251,y+43)+txt(264,y+20,meta,9,'ink',True)+txt(264,y+38,msg,12)+txt(440,y+33,'→',20)
   if i==0:s+=box(8,y+10,4,32,'ink',2)
 save(kind,'01-chats',footer(s,dark))
 s=shell('NACHRICHT',dark);fg='paper' if dark else 'ink'
 s+=txt(12,51,'#berlin',29,fg,True)+txt(370,47,'3 HOPS',10,fg)
 s+=box(12,65,310,57,'yellow',18)+txt(24,84,'MIA · 14:28',10,'ink',True)+txt(24,108,'Bin am Tor, wer kommt noch?',16)
 s+=box(190,131,278,54,'purple' if dark else 'blue',18)+txt(203,153,'Ich, in 10 min.',17,'ink',True)+txt(203,174,'14:30 · von 3 Repeatern gehört',10)
 save(kind,'02-conversation',footer(s,dark,'Tippen Antworten     ↵ Senden     ⌫ Zurück'))
 s=shell('SCAN',dark);fg='paper' if dark else 'ink'
 s+=box(12,31,143,164,'lime' if dark else 'yellow',20)+txt(24,54,'MESH / UMGEBUNG',10,'ink',True)+txt(22,113,'12',64,'ink',True)+txt(24,137,'Nodes gehört',14)+txt(24,178,'Letzte 15 Minuten',11)
 for i,(name,data,col) in enumerate([('RPT-Mitte','−7 dB · 1 Hop · 12 s','yellow' if dark else 'pink'),('Tracy','−11 dB · 2 Hops · 3 m','purple' if dark else 'blue'),('Wetter-01','−14 dB · 3 Hops · 9 m','lime')]):
  y=31+i*56;s+=box(165,y,303,52,col,15 if dark else 8)+txt(178,y+22,name,19,'ink',True)+txt(178,y+41,data,12)+txt(442,y+31,'→',18)
 save(kind,'03-scan',footer(s,dark,'↵ Details     A Advert lokal     B Flood     Space Modus'))
 s=shell('MESHCATOR',dark);fg='paper' if dark else 'ink'
 s+=txt(12,90,'14:32',70,fg,True)+txt(295,58,'FREITAG',11,fg,True)+txt(295,82,'30. Oktober',20,fg)
 for i,(title,value,col) in enumerate([('AKKU','78 %','yellow'),('MESH','−8 dB','purple' if dark else 'blue'),('GPS','8 SAT','lime'),('NEU','2 MSG','pink')]):
  x=12+i*117;s+=box(x,112,105,82,col,18)+txt(x+12,133,title,10,'ink',True)+txt(x+12,171,value,24,'ink',True)
 save(kind,'04-lock',footer(s,dark,'↵ Entsperren'))
 # Technical views retain rounded color panels, with precise plot geometry.
 s=shell('RADAR',dark)
 s+=box(12,29,206,170,'lime' if dark else 'yellow',20)
 for r in [23,46,66]:s+=f'<circle cx="115" cy="113" r="{r}" fill="none" stroke="#161616" opacity=".3"/>'
 s+=line(49,113,181,113)+line(115,47,115,179)+txt(111,43,'N',10)+txt(111,190,'S',10)+txt(33,117,'W',10)+txt(189,117,'E',10)
 s+='<path d="M115 113L160 66A66 66 0 0 1 181 113Z" fill="#161616" opacity=".08"/><path d="M115 113L160 66" stroke="#161616" stroke-width="1.5"/>'
 s+=txt(110,118,'+',17)+txt(143,86,'◆',14)+txt(79,144,'●',12)+txt(158,139,'◎',15)
 s+=txt(232,49,'Umgebung',25,'paper' if dark else 'ink',True)
 for i,(name,data,col) in enumerate([('◆ RPT-Mitte','640 m · 038° · −7 dB','yellow' if dark else 'pink'),('● Tracy','410 m · 224° · −11 dB','purple' if dark else 'blue'),('◎ Wetter-01','780 m · 118° · −14 dB','lime')]):
  y=61+i*39;s+=box(230,y,238,35,col,12)+txt(241,y+15,name,13,'ink',True)+txt(241,y+29,data,10)
 s+=txt(233,192,'2 km · GPS-Positionen · Scan aktiv',10,'paper' if dark else 'ink')
 save(kind,'05-radar',footer(s,dark,'↵ Node-Details     Encoder Radius     ⌫ Zurück'))
 s=shell('NOISE FLOOR',dark)
 s+=box(12,29,136,170,'yellow',20)+txt(24,51,'AKTUELL / dBm',10,'ink',True)+txt(22,96,'−112',40,'ink',True)+txt(24,124,'MIN  −118 dBm',12)+txt(24,145,'MAX  −96 dBm',12)+txt(24,180,'869.618 MHz',12)
 s+=box(158,29,310,170,'purple' if dark else 'blue',20)+txt(173,49,'SIGNALVERLAUF / 60 SEKUNDEN',10,'ink',True)
 for y,l in [(70,'−90'),(118,'−105'),(166,'−120')]:s+=line(206,y,454,y)+txt(170,y+4,l,10)
 for x in [206,267,330,391,454]:s+=line(x,65,x,171)
 s+='<path d="M206 160L217 160L227 168L237 157L254 157L263 162L279 159L294 161L306 150L318 156L329 143L338 139L347 101L355 89L364 117L375 142L386 157L399 160L411 168L425 159L439 157L454 160" fill="none" stroke="#161616" stroke-width="2" stroke-linejoin="round"/>'
 s+=txt(206,187,'−60 s',10)+txt(315,187,'−30 s',10)+txt(430,187,'jetzt',10)
 save(kind,'06-signal',footer(s,dark,'Space Pause     ↵ Messwert     ⌫ Zurück'))
 s=shell('KARTE',dark)+box(12,29,294,170,'blue',20)
 s+='<defs><clipPath id="map"><rect x="12" y="29" width="294" height="170" rx="20"/></clipPath></defs><g clip-path="url(#map)">'
 for x in range(26,307,28):s+=line(x,29,x,199)
 for y in range(38,200,28):s+=line(12,y,306,y)
 s+='<path d="M-5 164Q81 83 140 126T316 65" fill="none" stroke="#E4EEF5" stroke-width="15"/><path d="M35 29L91 84L138 77L201 145L282 165M42 201L91 84L115 20M201 145L254 29" fill="none" stroke="#161616" stroke-width="2" opacity=".6"/><path d="M91 84L138 77L165 107L201 145" fill="none" stroke="#161616" stroke-width="3" stroke-dasharray="5 4"/>'
 s+=txt(83,84,'◆',17)+box(99,59,82,19,'yellow',7)+txt(106,73,'RPT-Mitte',11,'ink',True)+txt(158,115,'▲',19)+box(19,35,74,18,'paper',7)+txt(26,48,'N ↑  SCHEMA',9)+line(27,179,85,179)+txt(27,173,'200 m',10)+'</g>'
 s+=box(316,29,152,102,'yellow',20)+txt(328,50,'ZIEL / REPEATER',10,'ink',True)+txt(328,78,'640 m',28,'ink',True)+txt(328,101,'038° · Nordost',12)+txt(328,119,'RPT-Mitte',12)
 s+=box(316,140,152,59,'lime' if dark else 'pink',16)+txt(328,161,'TRACK AKTIV',10,'ink',True)+txt(328,186,'1,8 km',24,'ink',True)
 save(kind,'07-map',footer(s,dark,'Encoder Zoom     Pfeile Pan     P Position     ↵ Wegpunkt'))

html='''<!doctype html><html lang="de"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>MeshCator · Offene Designstudie / Karten & Farbbänder</title><style>*{box-sizing:border-box}body{margin:0;background:#efede6;color:#161616;font:16px Arial,sans-serif}main{max-width:1120px;margin:auto;padding:36px 20px}h1{font-size:clamp(30px,5vw,56px);letter-spacing:-2px;line-height:1.03;max-width:800px}p{line-height:1.6;max-width:740px}small{font-size:11px;letter-spacing:2px}nav{display:flex;gap:10px;flex-wrap:wrap;margin:24px 0}button,a{color:inherit}button{border:1px solid #777;background:transparent;padding:12px 16px;border-radius:22px;cursor:pointer}.study{padding:24px;border-radius:28px;margin:24px 0}.dark{background:#161616;color:#f2f0e9}.light{background:#fffdf6}h2{font-size:30px;margin:8px 0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,480px),1fr));gap:24px}figure{margin:0;min-width:0;overflow:auto}figcaption{font-size:12px;margin:12px 0}img{display:block;max-width:100%;height:auto;border-radius:10px}.native img{max-width:none}.native .grid{grid-template-columns:1fr}.palette{display:flex;gap:10px;flex-wrap:wrap;margin:24px 0}.palette span{padding:18px 12px;border-radius:12px;color:#161616;font-size:12px}.note{border-top:1px solid #aaa;padding-top:20px;font-size:13px}</style><main><small>MESHCATOR / EXPLORATION 07 · NICHT FINAL</small><h1>Mehr Fläche.<br>Mehr Typografie.<br>Präzise Instrumente.</h1><p>Aus deinen Referenzen übernehmen wir kräftige Farbkarten, klare Hierarchie und großzügige Rundungen. Zwei unterschiedliche Übersetzungen auf das Pager-Display – dieselben Inhalte, jeweils 480 × 222 Pixel. Jetzt mit Radar, Signaldiagramm und Karte inklusive Track.</p><nav><button onclick="document.querySelectorAll('.study').forEach(s=>s.hidden=false)">Beide Richtungen</button><button onclick="document.querySelectorAll('.study').forEach(s=>s.hidden=s.id!=='cards')">A · Dunkle Karten</button><button onclick="document.querySelectorAll('.study').forEach(s=>s.hidden=s.id!=='bands')">B · Helle Farbbänder</button><button onclick="document.body.classList.toggle('native')">1:1 umschalten</button></nav>'''
for kind,title,desc in [('cards','A / Dunkle Karten','Leuchtende Karten auf Anthrazit. Drei Gespräche nebeneinander; der Encoder bewegt den Fokus von Karte zu Karte.'),('bands','B / Helle Farbbänder','Drei farbige Zeilen mit großer Schrift. Namen, Vorschau und Status stehen nebeneinander; der Encoder bewegt den Fokus von oben nach unten.')]:
 html+=f'<section class="study {"dark" if kind=="cards" else "light"}" id="{kind}"><small>OFFENE RICHTUNG</small><h2>{title}</h2><p>{desc}</p><div class="palette">'+''.join(f'<span style="background:{P[c]}">{c.upper()}<br>{P[c]}</span>' for c in ['yellow','purple' if kind=='cards' else 'blue','lime','pink'])+'</div><div class="grid">'
 for p in sorted((R/f'assets/display/study-{kind}').glob('*.svg')):
  html+=f'<figure id="{kind}-{p.stem}"><figcaption>{ {"01-chats":"Gesprächsübersicht","02-conversation":"Chat lesen und antworten","03-scan":"Mesh-Umgebung","04-lock":"Sperrbildschirm","05-radar":"Radar / Positionen und Peilung","06-signal":"Signaldiagramm / Noise Floor","07-map":"Karte / Track und Ziel"}[p.stem]}</figcaption><img src="../../assets/display/study-{kind}/{p.name}?v=7" width="480" height="222" alt="{title} {p.stem}"></figure>'
 html+='</div></section>'
html+='''<p class="note">Die Farben unterscheiden hier Inhaltsflächen; Zustände wie „NEU“ oder „FOKUS“ stehen zusätzlich als Text. Kein Touch vorausgesetzt. Große Überschriften nur in Übersichten, Chattexte bleiben bei 16–17 px. Beispiele zeigen kurze Namen; lange Namen, weitere Einträge und Großschrift müssen im nächsten Entwurf ausgearbeitet werden. Radar zeigt Beispielknoten mit bekannten Positionen; die Karte ist ein Schema, keine echte Kartografie. Diagramm und Messwerte sind Beispieldaten. Keine finale Designentscheidung und noch kein Hardware-Test.</p><p><a href="color-variants.html?v=5">Bisherige Farbstudien</a> · <a href="preview.html?v=4">Bisheriges Feldinstrument-Design</a></p></main></html>'''
(R/'docs/design/editorial-study.html').write_text(html)
print('14 SVG-Mockups und offene Vergleichsstudie erstellt.')
