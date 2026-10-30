# MeshCator 0.3.2: Karten- und Menüreaktion

Auslöser: Jede Menübewegung zeichnete die vollständig verdeckte darunterliegende Ansicht erneut. In der Kartenansicht wurden dabei alle Grenzsegmente weltweit projiziert. Auch die sekündliche Statusaktualisierung berechnete dieselbe Karte neu.

Änderungen:
- Vollflächige Menüs/Dialoge zeichnen den verdeckten Inhalt nicht mehr.
- Kartenbild, Ortsbeschriftungen und Titel bleiben zwischengespeichert, solange der sichtbare Kartenzustand gleich bleibt.
- Zoom, Ausschnitt, Farben, Sprache, Kontaktanzeige, Marker und Navigationsziel invalidieren den Cache.
- Nicht sichtbare Grenzringe werden vor der Projektion verworfen; ihre Grenzen werden einmalig berechnet.
- Der Weltatlas sucht die passende Kachel beim Kachelwechsel statt für jedes einzelne Pixel. Reiner SD-Modus berechnet keinen ungenutzten Weltatlas.
- Fehlende SD-Kacheln werden weiterhin begrenzt nachgeladen; erneute Versuche erfolgen nach zehn Sekunden. SD-Lesezugriffe bleiben synchron, daher können langsame SD-Karten weiterhin einzelne Verzögerungen verursachen.

Prüfung: 20/20 Tests bestanden. Der neue LVGL-Regressionstest löst 100 Status-Neuzeichnungen, 100 Kartenmenübewegungen und 100 Hauptmenübewegungen aus und prüft, dass keine zusätzliche Kartenberechnung stattfindet. Zoom und geänderte Kontaktanzeige erzeugen dagegen jeweils eine neue Berechnung. Desktop-Zeiten sind keine Aussage über die reale Pager-Latenz.

Geräteabnahme: Hauptmenü langsam und schnell drehen; Kartenmenü öffnen und drehen; zwischen Zoomstufen wechseln; Karte mit und ohne Kontakte sowie im Vollbild prüfen. Die Drehrichtung und Tastenbelegung wurden nicht geändert.
