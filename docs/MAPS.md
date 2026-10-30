# Offline-Karten in MeshCator

Die integrierte Karte wird aus Natural Earth Landflächen und Seen im
Datenmaßstab 1:50 Millionen erzeugt. „50m“ im Quelldateinamen bedeutet
50 Millionen, **nicht 50 Meter**. Natural Earth stellt diese Geodaten als
[Public Domain](https://www.naturalearthdata.com/about/terms-of-use/) bereit.
Es werden keine fertigen Kartenbilder eines Kartendienstes kopiert.

`tools/build_map_tiles.py` erzeugt 341 eigene XYZ-Kacheln für Zoom 0–4,
jeweils 256 × 256 Pixel. Eine komprimierte 1-Bit-Maske unterscheidet Land
und Wasser; die Farben werden beim Rendern aus dem aktiven Theme berechnet.
Der Atlas benötigt 162.813 komprimierte Datenbytes zuzüglich Index. Quellen
und SHA-256-Prüfsummen stehen in `assets/maps/tiles-manifest.json`.
Die Rohdaten liegen als `ne_50m_land.geojson` und `ne_50m_lakes.geojson`
in `assets/maps/`; das Ergebnis ist `src/maps/WorldTiles.h`.
Reproduktion: Python 3 mit Pillow, dann `python tools/build_map_tiles.py`.

Die Karte enthält Küsten, Inseln, Landflächen und Seen. Sie ist eine
Übersichtskarte ohne Straßen oder Hausnummern. Ländergrenzen und
Regionsnamen werden als eigene Vektorebene ergänzt (siehe unten). Oberhalb Zoom 4 wird sie vergrößert; dadurch entsteht kein neuer
Detailgrad. Eigene SD-Kacheln ermöglichen detailliertere lokale Karten.
Die frühere 1:110-Millionen-Küstenlinie bleibt als dokumentiertes Ausgangsasset
im Repository, wird aber nicht mehr vom Renderer verwendet.

## Eigene SD-Kacheln

- Web Mercator / XYZ, Zoomstufen 0 bis 18, 256 × 256 Pixel.
- Pfad: `/meshcator/tiles/{z}/{x}/{y}.bmp` auf der SD-Karte.
- Windows-BMP, RGB 24 Bit, unkomprimiert; kein TMS-Y und kein MBTiles-Container.
- Beispiel: `/meshcator/tiles/2/2/1.bmp`.
- `tools/import_tiles.py source_xyz sd/meshcator/tiles` konvertiert vorhandene
  PNG/JPEG/WebP/BMP-Kacheln mit Pillow. Es lädt keine Kartendaten herunter.
  Vorhandene Zieldateien werden nicht überschrieben.
- Nutzer müssen für ihre eigenen Kacheln die jeweils passenden Nutzungsrechte
  haben. MeshCator bringt keine kopierten Straßenkarten oder Download-Schlüssel mit.

Settings → Map → Map source: Built-in / SD card / Automatic.
Automatic überlagert verfügbare Kacheln über der eingebauten Karte. Fehlende
SD-Kacheln werden ausdrücklich als `SD --` bzw. `SD / partial` angezeigt.
Der Cache ist auf acht Kacheln begrenzt. Pro Renderdurchlauf wird höchstens
eine neue Kachel gelesen, damit Funk und Eingabe dazwischen weiterlaufen.
Fehlende Kacheln werden nach zehn Sekunden erneut gesucht; dafür ist keine
GPS-Uhrzeit erforderlich. Bei Austausch bereits geladener Dateien die App neu
starten. Der Simulator liest `tiles/z/x/y.bmp` neben seiner Settings-Datei.

## Bedienung

Rad dreht den Zoom, Klick öffnet das Kartenmenü. Das Menü enthält Vollbild,
eigene Position, Kontakte ein/aus, Standort merken, gemerkte Orte und Navigation
beenden. Back schließt zuerst ein Menü und danach das Vollbild. Back >2 Sekunden
führt zum Dashboard.

Ein Ort kann nur mit gültigem GPS-Fix gespeichert werden. Die Position wird
beim Öffnen des Namenseditor eingefroren, damit sie während der Texteingabe nicht
wandert. Bis zu 16 benannte Orte bleiben nach Neustart gespeichert. Die Auswahl
eines Ortes zentriert die Karte und zeigt bei GPS-Fix Entfernung und Peilung vom
aktuellen Standort. Das ist Luftliniennavigation, kein Straßenrouting.

## Länder und Regionen

Die eigene Vektorebene enthält 242 Länder-/Gebietseinträge aus Natural Earth
1:50 Millionen sowie 215 Verwaltungseinheiten aus dem 1:10-Millionen-Datensatz.
Regionale Abdeckung: Deutschland, Frankreich, Belgien, Niederlande, Luxemburg,
Dänemark, Polen, Tschechien, Österreich und Schweiz. Andere Länder haben derzeit
nur die Landesebene. Natural Earth verwendet überwiegend De-facto-Grenzen;
die Darstellung ist keine Aussage zur politischen Anerkennung.

Ländernamen erscheinen bei Übersichtszoom, Regionen ab Zoom 5. Ab Zoom 6
nennt die Kopfzeile das Land und, soweit enthalten, die Region am Kartenmittelpunkt.
Diese Zuordnung bezieht sich auf den Kartenmittelpunkt, nicht zwingend auf den
GPS-Standort. Namen folgen der eingestellten Sprache. Überlappende Namen werden
unterdrückt, höchstens sieben gleichzeitig angezeigt. SD-Karten werden nicht
mit zusätzlichen Namensetiketten überlagert.

Die Grenzen sind stark vereinfachte Übersichtsdaten mit 0,01°-Koordinatenraster;
bei Zoom 18 sind sie **nicht lagegenau**. Die Regionsanzeige nahe einer Grenze
kann entsprechend abweichen. Ein Name in der Kopfzeile bedeutet keine zusätzliche
Straßenauflösung. Herkunft/Prüfsummen: `assets/maps/admin-manifest.json`.
Reproduktion: `python3 tools/build_admin_map.py` → `src/maps/AdminData.h`.
