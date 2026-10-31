# MeshCator

MeshCore-Firmware und LVGL-Oberfläche für den LilyGo T-LoRa Pager SX1262.
Die aktuelle Implementierung ist ein Entwicklungsstand, keine abgeschlossene
Umsetzung des gesamten Feature-Backlogs aus PLAN.md.

## Stand 0.3.25-dev

512 Nachrichten im persistenten Verlauf; neue Nachrichten werden bei verfügbarer SD-Karte zusätzlich unter `/meshcator/messages.tsv` protokolliert. Ältere SD-Einträge sind per USB exportierbar, werden aber noch nicht in die Chatansicht nachgeladen. Details und Grenzen: [Nachrichtenarchiv](docs/HISTORY-0.3.8.md). 29 lokale Tests bestanden. SD-Karten wurden am Pager erprobt.

Aktuell enthalten sind unter anderem individuelle Lesemarkierungen mit Einstieg bei der ältesten ungelesenen Nachricht, statische Absenderflächen, Emoji-Unterstützung, haptisches Menüfeedback und optimiertes SD-Kartenladen. Karten lassen sich mit A/S/W/Z verschieben; Nodes haben kontrastreiche Markierungen mit transparentem Außenring. Display-Aufwecken bei Nachrichten ist separat konfigurierbar. Details: [Funktionsstand](docs/FEATURES-0.3.md) und [Karten](docs/MAPS.md).

## Desktop-Simulator

Voraussetzungen: CMake, C++17, SDL2. Unter macOS ist SDL2 beispielsweise über
Homebrew verfügbar. LVGL 9.2.2 wird beim ersten Configure geladen.

```sh
cmake -S . -B build/simulator -DCMAKE_BUILD_TYPE=Debug
cmake --build build/simulator -j 8
ctest --test-dir build/simulator --output-on-failure
./build/simulator/meshcator --settings build/simulator/settings.txt
```

Der Simulator ist ausdrücklich **DEMO / kein Funk**. Die Beispieldaten werden
nur im Desktop-Backend angelegt. Ohne Simulator-Abhängigkeiten lassen sich
die Kernlogiktests so ausführen:

```sh
cmake -S . -B build/core -DMESHCATOR_SIMULATOR=OFF
cmake --build build/core
ctest --test-dir build/core --output-on-failure
```

Bedienung:

| Eingabe | Funktion |
|---|---|
| Tab / Space | Modusauswahl öffnen; im Chat bleibt Space ein Leerzeichen |
| Pfeile / Mausrad | Auswahl; im Chat Nachrichtenverlauf |
| Enter | Öffnen / Senden / Theme wechseln / Diagramm pausieren |
| Escape | Zur Übersicht, Entwurf bleibt erhalten |
| Backspace | Ein UTF-8-Zeichen löschen |
| F2 | Theme wechseln und speichern |
| F3 / F4 | Lokales / Flood-Advert (im Simulator nur simuliert) |
| F5 | Sperrbildschirm |
| + / - | Radius für Radar und Positionsplot |

Vier Themes: Farbkarten, helle Palette, Warm Orange, Neon Purple. Die helle
Palette verwendet derzeit noch dieselbe Kartenanordnung wie das dunkle Theme.

## Pager-Firmware

```sh
pio run -e meshcator-pager
```

Python 3.12 wird empfohlen. Auf dem Entwicklungsrechner funktioniert der
PlatformIO-Einstieg `~/.platformio/penv/bin/pio` mit Python 3.12; der Homebrew-
Einstieg verwendet Python 3.14 und scheitert bei einer Abhängigkeit.

Der SX1262-Port benötigt aktuell Arduino-ESP32 2.0.17 (PlatformIO espressif32
6.11.0), unter anderem wegen seiner LEDC-API. LilyGoLib wird **nicht** mitgebaut.
Die im ursprünglichen Plan vorgesehene Umstellung auf Arduino 3.3 bleibt offen.
Default-Radiowerte: 869.618 MHz, 62.5 kHz, SF8, CR5, 14 dBm. Das ist ein
konkreter Entwicklungs-Preset und noch kein First-Boot-Regionsassistent.
Die bestehenden Funk-/Kanal-/Identitätseinstellungen können über den
MeshCore-USB-Companion konfiguriert werden. BLE ist derzeit deaktiviert.
Vorhandene gespeicherte Radioeinstellungen haben Vorrang vor Build-Defaults.

Pager: Drehrad wählt aus, kurzer Klick führt die angezeigte Aktion aus,
langer Druck öffnet immer die Modusauswahl. Seitentaste: zurück; in der Gesprächsübersicht bleibt sie ohne Aktion. Karte: Drehen zoomt, Klick öffnet das Menü.
Chat: Drehen scrollt, ausschließlich Enter sendet; Radklick sendet nie, sondern öffnet das Chat-Kontextmenü.
Backspace löscht Text; die Seitentaste führt zurück und schließt Menüs. In den
Einstellungen wählt das Rad Sprache oder Theme; Klick/Enter öffnet die Auswahl
und bestätigt sie. Zurück verwirft die noch nicht bestätigte Auswahl. Die Tastatur behält FN-Symbole und
Großschreibung des Referenzports. Die Encoder-Flanken werden per Interrupt
erfasst, damit Display-Rendering keine Schritte verschluckt.

USB: Standard-MeshCore-Companion-Befehle sind im Unterbau vorhanden;
das ist keine vollständige Wartungsschnittstelle für alle MeshCator-Funktionen.
Insbesondere fehlen Theme-/UI-Konfiguration, umfassender SD-Dateizugriff
und eine getestete Backup-/Restore-Oberfläche über USB. Firmware-Updates
über den ESP-Bootloader müssen den zusätzlichen App-Slot beachten.
Tastatur-Debugausgaben auf dem USB-Protokoll wurden entfernt.
Der neue 0.2.0-dev-Build enthält die MeshCore-Nordic-UART-BLE-Schnittstelle.
Settings → Bluetooth steuert Ein/Aus, Gerätename und sechsstellige PIN.
Ein echter Pairing-/Nachrichtentest mit der Smartphone-App steht noch aus.

## Zusätzliche Installation im vorhandenen Launcher

Der gerätespezifische Build `pio run -e meshcator-launcher` ergänzt die
bestehende Mehrfachinstallation. Die vorhandenen App-Partitionen `app0`,
`firmwa`, `meshos` und `roami` behalten Offset, Größe und Subtyp.
Für 0.2.0 wird FT8 innerhalb seiner bisherigen Reservierung von 0x870000
nach 0x920000 verschoben; sein neuer Slot endet weiterhin bei 0xb70000.
Die Migration erfordert ein vollständiges, per Flash-MD5 bestätigtes Backup
und bricht ab, wenn der entfallende FT8-Reservierungsbereich Daten enthält.

| Bereich | Offset | Größe |
|---|---|---|
| MeshCator (ota_4) | 0x6e0000 | 0x240000 |
| FT8 (ota_1) | 0x920000 | 0x250000 |
| MeshCator-Dateisystem (mcfs) | 0xe70000 | 0x100000 |

Dieser Build ist ausschließlich für die geprüfte 16-MB-Partitionierung
bestimmt. Kein `pio run -t upload`: der generische Upload würde auch den
bestehenden Launcher-Bootloader ersetzen. `tools/migrate_launcher.py` schreibt ausschließlich FT8 an seine neue Adresse,
MeshCator und zuletzt die Partitionstabelle. Bootloader, OTA-Auswahl, NVS und
beide Dateisysteme werden nicht geschrieben; alle erhaltenen Partitionen
werden vor dem Neustart gegen das Backup geprüft. Die Anzahl OTA-Slots bleibt gleich.

MeshCator verwendet SPIFFS-Label `mcfs`, SD-Verzeichnis `/meshcator` und
NVS-Namensräume `meshcator` / `mc_power`. Das vorhandene `spiffs` bleibt
unberührt. NVS ist weiterhin eine gemeinsame physische Partition.
Die lokalen Sicherungen und das Installationsprotokoll liegen unter
`backups/pager-2026-10-30/` (nicht versioniert); `RESTORE.txt` beschreibt die
gezielte Rücknahme. Die erste Installation hatte nur Teilbereichssicherungen;
die Erweiterung setzt ein vollständiges 16-MB-Abbild voraus.

## Architektur

- `src/core`: hardwarefreie Zustands-/Navigationslogik, Nachrichtenspeicher,
  UTF-8-Prüfung, Entwürfe pro Chat, Geodäsie.
- `src/ui`: echte LVGL-Widgets für Chats, Verlauf, Kontakte, Radar, Signal,
  Küstenkarte, Dashboard, Settings und Sperrbildschirm. Kein Screenshot als UI.
  Eigene Montserrat-Bitmapfonts mit Latin-1/Umlauten und Euro (SIL OFL).
- `src/hal/PagerApp.cpp`: Adapter zum gepinnten SX1262-Port; echte Kontakt-,
  Nachrichten-, Batterie-, GPS- und Radio-Anbindungen.
- `src/mesh/SimBackend.*`: ausdrücklich getrenntes Desktop-Demo-Backend.
- `vendor/meshcore-pager`: gepinnte MIT-Quelle, siehe THIRD_PARTY.md.
- `tools/build_*`: bisherige Designstudien, unabhängig vom Firmware-Build.

## Aktuelle Grenzen

- Auf dem Pager zusätzlich installiert; USB-Companion antwortet als
  MeshCator-0.1.0-dev, LVGL meldet UI ready, PSRAM und SD sind initialisiert.
  Visuelle Bedienungsabnahme und Funk-Gegenstellentests stehen noch aus.
  Im Startlog ist ein I2C-Lesefehler sichtbar; dessen Ursache ist noch offen.
- Eingebaute Natural-Earth-Küstenkarte (Public Domain), optionale SD-BMP-Kacheln,
  benannte Orte und Peilung/Entfernung sind implementiert. Freies Pan,
  Track-Aufzeichnung, GPX und Straßenrouting fehlen. Siehe docs/MAPS.md.
- Empfangene Nachrichten erscheinen live; die neue UI-Historie ist derzeit
  ein begrenzter RAM-Ring. Der Port speichert empfangene DMs auf SD, die neue Oberfläche
  lädt diesen Verlauf noch nicht wieder ein. Ausgehende Nachrichten der neuen
  Oberfläche werden noch nicht auf SD gespeichert. Kanalhistorie ist noch nicht persistent.
- Erfolgreiches Senden bedeutet **an Funkwarteschlange übergeben**, nicht
  zugestellt. ACK-Zuordnung, Wiederholungen und Heard-by fehlen in der UI.
- Radar verwendet bekannte Kontaktpositionen. Nodes ohne Position erhalten
  keine erfundene Peilung. SNR ist im Geräteadapter noch nicht angeschlossen.
- MC2 speichert Sprache, Theme, Ersteinrichtung, Geräte-, Funk-, BLE-, Display-
  und Kartenoptionen einschließlich bis zu 16 Orten. MeshCore bleibt die
  maßgebliche Quelle für Funk-/Identitätsparameter aus USB/BLE. Admin, OTA-UI
  und Audio-Einstellungen fehlen weiterhin.
- Der Port arbeitet zunächst kooperativ in einem Loop: Mesh, Sensoren, UI.
  Die im Plan vorgesehene Aufteilung in FreeRTOS-Tasks/EventBus ist noch offen.
- Zeitangabe ist UTC. Tages-/Rotlicht-Modus und große Schrift sind offen.

Die Designsuche bleibt offen. `docs/design/editorial-study.html` enthält die
farbigen Karten mit Radar, Diagramm und Karten-Schema; die implementierte
Version muss schrittweise auf echter Hardware getestet werden.

## Build-Ergebnisse

Lokal verifiziert: Firmware-Build erfolgreich; 9/9 CTest-Tests erfolgreich
(Kernverhalten plus acht Render-Smoke-Tests). Statischer RAM: 173.440 Bytes
(52,9 %); Flash: 1.554.185 Bytes (29,6 % der App-Partition).
Die Display-Puffer belegen zusätzlich 76.800 Bytes PSRAM.

Der Entwicklungs-Build erzeugt `.pio/build/meshcator-pager/firmware.bin`.
Dies ist eine App-Partition, kein zusammengefügtes Erstinstallations-Image.
Nicht als alleinigen Raw-Flash bei Offset 0 verwenden. Für Geräte ohne Launcher kann ein Upload über PlatformIO die passende
Standalone-Partitionierung installieren. Auf dem vorhandenen Multiboot-Gerät
keinen generischen PlatformIO-Upload ausführen: Dieser ersetzt Bootloader und
Partitionstabelle. Die zusätzliche Installation verwendet meshcator-launcher
und die gerätespezifische Tabelle config/partitions-launcher.csv.

Die LVGL-Ausgaben liegen unter `docs/implementation/`; `index.html` zeigt die
echten Simulator-Screenshots. CI ist als Workflow vorbereitet, noch nicht
auf GitHub ausgeführt. Die Launcher-Variante wurde am 30.10.2026 zusätzlich
auf dem Pager installiert. Alle fünf bestehenden App-Partitionen und der
Bootloader sind per MD5 unverändert bestätigt. Nichts wurde gepusht.

Das gemeinsame Alt-Dateisystem hatte nach zwischenzeitlichen Starts der
bisherigen Firmware eine andere Prüfsumme als vor der Sicherung. Es wurde
vom Installer nicht beschrieben; die Ursache ist nicht eindeutig belegt.
Details und rohe Prüfergebnisse bleiben im lokalen Sicherungsverzeichnis.

Ein weiterer MeshCator-Start ließ das gemeinsame Alt-Dateisystem unverändert
(MD5 vor/nach dem Start identisch). Der Startvergleich ist lokal protokolliert.

Bedienungsupdate: Modusmenü und Interrupt-Encoder auf den vorhandenen App-Slot
geflasht und per Flash-Hash verifiziert. Danach fünf lesende USB-Companion-
Abfragen erfolgreich (Version, Knoteneinstellungen, Uhr, Batterie/Speicher,
Statistik). Die praktische Abnahme der neuen Tastenbedienung steht noch aus.

Splash-Logo: native LVGL-Vektorgeometrie, passend zu Farbkarten und
Netzknoten-Motiv; editierbare Wortmarke unter assets/brand/meshcator-logo.svg.
Wird unmittelbar nach Display-Initialisierung während des Systemstarts gezeigt.
Encoder-Drehrichtung nach Hardware-Rückmeldung invertiert. Radklick und
Enter sind getrennte Core-Ereignisse; nur Enter sendet Nachrichten.

Zurück-/Seitentaste länger als zwei Sekunden halten: aus jeder Ansicht
(einschließlich Menü und Sperrbildschirm) zur Hauptübersicht/Gespräche.
Entwürfe bleiben erhalten. Beim Loslassen folgt keine zusätzliche Zurück-Aktion.

Lesbarkeitsupdate: Kopfzeile 14 px, Uhr/GPS/Akku 16 px, untere Aktionen
und Meldungen 14 px, technische Nebenbeschriftungen mindestens 12 px.
Die Fußleiste hat eine eigene Fläche; Hinweise sind kontextabhängig gekürzt.

## Sprache

Englisch ist der Standard, auch bei älteren gespeicherten Einstellungen ohne
Sprachfeld. Settings → Language bietet English, Français, Español und Deutsch.
Rad: auswählen; Klick/Enter: öffnen bzw. speichern; Back: abbrechen.
Menüs, Hinweise, Fehlermeldungen und Splash sind übersetzt. Nachrichten und
Kontaktnamen bleiben unverändert. Die Sprache wird auf dem Pager im NVS-Schlüssel
meshcator/language gespeichert und bereits vor dem Splash geladen.
Desktop speichert sie als zusätzliche Zeile im bisherigen Einstellungsformat;
alte Dateien bleiben lesbar. Übersetzungen stehen in src/core/I18n.cpp.
Zehn lokale Tests bestehen, inklusive Sprachwechsel, Abbruch, Persistenz aller
vier Sprachen und Migration alter Einstellungen auf Englisch.

Karten-Vollbild: In Map öffnet Radklick/Enter das Kartenmenü. Fullscreen
vergrößert die aktuelle Karte auf 480 × 222. Exit fullscreen oder
Back führt zur normalen Kartenansicht zurück; Back schließt zunächst ein
offenes Menü. Rad zoomt weiterhin, langes Back geht ebenfalls eine Ebene zurück. Position und
Zoom bleiben beim Umschalten erhalten. Basiskarte und SD-Kacheln verwenden
dieselbe Web-Mercator-Projektion im Normal- und Vollbildmodus.

Korrektur der physischen Tastatur-Zurück-/Löschtaste: kurzes Drücken löscht
im Nachrichteneditor genau ein UTF-8-Zeichen, außerhalb geht es zurück.
Die kurze Aktion erfolgt beim Loslassen. Halten 0,8 Sekunden führt eine Ebene zurück
(zum vorherigen Screen bzw. aus dem offenen Menü), ohne Zeichen zu löschen; der Entwurf bleibt erhalten. Dies gilt
zusätzlich zur Seitentaste. SDL-Backspace bildet dieselbe Kurz-/Langdrucklogik ab.

## Node- und Chat-Kontextmenüs

Kontakt/Radar → Radklick oder Enter öffnet das Node-Menü. Darin: Chat öffnen,
Details (Public Key, Pfad/Hops, Position), Favorit setzen/entfernen,
Pfad zurücksetzen sowie Hauptmenü und Schließen. Favoriten sind mit * markiert.
Favorit und Pfad-Reset schreiben die Kontakte über den MeshCore-Unterbau.
Pfad-Reset setzt OUT_PATH_UNKNOWN; es wird keine erfolgreiche neue Route behauptet.

Im Chat öffnet Radklick das Kontextmenü; langer Raddruck öffnet immer das Hauptmenü. Enter sendet
nur im Editor. Kanalchats bieten neueste Nachricht, Kanaldetails, Hauptmenü
und Schließen. Bekannte Direktkontakte zusätzlich Favorit und Pfad-Reset.
Zurück schließt zuerst Details/Menü; Entwürfe bleiben erhalten. Der Simulator
bildet Aktionen nur lokal ab; F6 öffnet dort das Kontextmenü.

### Noch keine Funktionsgleichheit mit anderen Clients

| Bereich | Stand |
|---|---|
| Node-/Chat-Kontextmenüs, Details, Favorit, Pfad-Reset | Implementiert; Hardware-Gegenprüfung der Kontaktaktionen noch offen |
| Repeater-Login, Remote-Variablen und Admin | Hooks im Port vorhanden, neue UI fehlt |
| Telemetrie, Statusabfragen, Traceroute/Discovery | Protokoll unterstützt Teile; vollständiger Anfrage-/Antwortablauf in UI fehlt |
| ACK, Zustellstatus, Wiederholung | Noch nicht in neuer UI abgebildet |
| Kontakt importieren/teilen/löschen, Entdeckte Nodes | Neue UI noch unvollständig |
| Kanäle | Öffentlich/privat anlegen und Filter implementiert; Bearbeiten/Löschen/Export noch über Companion |
| Historie nach Neustart, Export und vollständige USB-Wartung | Unvollständig |
| Funkparameter, Profile, Regionsauswahl | Wizard und Einstellungsseiten implementiert |
| BLE-App-Verbindung | Im 0.2.0-Build enthalten; echtes Smartphone-Pairing noch zu testen |

Die jetzigen Menüs schließen konkrete Bedienlücken; sie sind kein Nachweis
vollständiger Ripple-/MeshOS-Funktionsgleichheit.

## Einheitliche Bedienung und Ersteinführung

Merksatz: **Drehen wählt, Klicken bestätigt, Halten öffnet das Hauptmenü,
Zurück führt zurück.** Im Chat: Enter sendet, Radklick öffnet das Chatmenü,
kurzes Back/Delete löscht. Back/Delete 0,8 Sekunden halten geht eine Ebene zurück und
bewahrt den Entwurf. Die separate Seitentaste bleibt eine Navigationstaste.
Buchstabentasten außerhalb des Editors lösen keine versteckten Funkaktionen aus.

Vier kurze Hilfekarten erklären diese Regeln in allen vier Sprachen. Beim
Start ohne abgeschlossene Einführung erscheinen sie nach dem Splash;
Settings → Help (letzter Eintrag) öffnet sie jederzeit erneut. Erst Done
speichert help_ver=1 im NVS-Namensraum meshcator. Abbrechen/Home markiert
nichts als abgeschlossen, die Einführung erscheint beim nächsten Start erneut.
Die ältere Einstellungsdatei bleibt lesbar; der Simulator speichert den
Abschluss ebenfalls. Tastennavigation, Abbruch, Speicherfehler und erneuter
Start nach Abschluss werden in den lokalen Tests geprüft.


## Stand 0.2.0-dev: Karte und vollständige Basiseinstellungen

Beim ersten Start: Sprache → Funkprofil → Name → Companion-Modus → Bluetooth
→ Funkparameter ausdrücklich bestätigen → Tastenhilfe → Dashboard.
Vor dieser Bestätigung sperrt auch der Radioadapter ausgehende Funkpakete.
Es wird keine Region aus dem Standort abgeleitet. Profile stammen aus dem
MeshCore-Konfigurationsdienst (Snapshot in assets/radio-presets.json); sie
ersetzen keine Prüfung der für Standort und Hardware passenden Funkparameter.
„Companion + forwarding“ nutzt den vorhandenen Client-Forwarding-Modus,
es ist keine Repeater- oder Room-Server-Firmware.

Settings enthält Sprache, Theme, Radio/Region, Name/GPS/Positionsfreigabe,
Bluetooth/Name/PIN, Displayhelligkeit/Timeout, Kartenquelle/Kontakte,
Kanalerstellung und Gesprächsfilter, erneute Einrichtung, Version und Hilfe.
Private Kanäle verwenden einen 16-Byte-Schlüssel (32 Hex-Zeichen); ein zufälliger
Schlüssel wird angeboten. Hashtag-Kanäle leiten ihren Schlüssel aus dem exakten
Namen inklusive `#` ab. Der Filter ist unter Settings → Channels / filter
wählbar. Der Simulator simuliert die Kanalerstellung; echte Kanalpersistenz
liegt im MeshCore-Dateisystem auf dem Pager.

`cmake --build build/simulator -j8` und `ctest --test-dir build/simulator
--output-on-failure` prüfen Core, Persistenz, Wizard, Fehlerfälle,
Kartengeometrie, Kacheldecoder und neun Renderansichten (14 Tests einschließlich Benachrichtigungen und Kartenatlas).


Hardwarestatus dieses Updates: Firmware 0.2.0-dev ist gebaut, aber noch nicht
installiert. Der Pager verschwand während der Vollsicherung vom USB; es wurde
keine Partition verändert. `tools/backup_launcher.py` sichert künftig in
fortsetzbaren, einzeln verifizierten 64-KB-Blöcken. Erst ein vollständiger
Abgleich erlaubt `tools/migrate_launcher.py --apply`. Der Stand steht auch in
`docs/implementation/current-build.json`.

## Kritischer Nutzer-Audit (30.10.2026)

**Noch kein freigegebener Alltagsstand.** Der [Nutzer-Audit](docs/USER-AUDIT.md)
enthält den Funktionsabgleich mit MeshCore, Ripple und MeshOS sowie offene
Freigabeblocker. Neu: 341 eigene Public-Domain-Kartenkacheln mit Land/Seen und
Theme-Farben; globale und gesprächsbezogene Alarme (stumm, Ton, Vibration, beides).
Settings → Notifications; Chat → Radklick → Notifications. Master-Mute hat
Vorrang. Alarmhardware und Smartphone-Pairing sind noch nicht real abgenommen.

## Stand 0.3.0-dev

DM-ACK/Timeout/Retry, persistenter Verlauf und Entwürfe, Kanal-/Kontaktverwaltung,
160 auswählbare Emojis und 358 monochrome Emoji-Zeichen, Remote-Anmeldung/Abfragen/CLI und ein isolierter
USB-Wartungskanal sind implementiert. Umfang, Bedienung, USB-Befehle und Grenzen:
[FEATURES-0.3.md](docs/FEATURES-0.3.md). Frühere Audit-Listen dokumentieren den
vorherigen Stand; Hardwareabnahme und sichere Installation bleiben offen.

Credits sind unter Einstellungen → Info / Credits abrufbar. Lizenzinventar und Originaltexte: [THIRD_PARTY.md](THIRD_PARTY.md) und [LICENSES/components.json](LICENSES/components.json). Die Projekt-MIT-Lizenz ersetzt nicht die Lizenzen der Abhängigkeiten.

## Zwischenstand: Pager-Mehrfachinstallation mit Bruce (31.10.2026)

Auf ausdrücklichen Wunsch ersetzen Bruce 1.16.1 und sein eigener Speicher
Meshtastic und Roami. FT8 wurde unverändert nach 0x1a0000 verschoben;
Launcher, MeshOS, MeshCator und ihre bisherigen Daten bleiben erhalten.
`config/partitions-launcher.csv` beschreibt den neuen Zustand. MeshCator bleibt
bei 0x6e0000, verwendet jetzt aber OTA-Slot 3. Die geschützten Update-/Auswahltools
berücksichtigen sowohl das alte als auch das neue Layout.

Bruce basiert auf dem offiziellen T-LoRa-Pager-Release. Zwei überprüfte
DROM-Zeichenketten wurden angepasst: `spiffs` → `brucef`, `nvs` → `bnv`.
Segmentprüfsumme und Image-SHA wurden danach neu erzeugt; keine Änderung an
Programmcode oder Funktionen. `tools/install_bruce_pager.py` dokumentiert
Quelle, feste SHA-256, genaue Offsets, Sicherungsprüfung und Migration.
Es ist ein gerätespezifisches Werkzeug, kein allgemeiner Flasher. Der
Standardaufruf erzeugt nur einen Plan; Schreiben verlangt `--apply` und
vollständige Übereinstimmung mit der 16-MB-Sicherung. Bruce bleibt ein
separates AGPL-Projekt: https://github.com/BruceDevices/firmware/releases/tag/1.16.1 .

Auch nach dem Bruce-Erststart wurden alle erhaltenen App-/Datenbereiche
verifiziert. Bruce hat nur sein eigenes LittleFS initialisiert; sein neuer
NVS-Bereich war dabei noch leer, dessen Nutzung ist noch nicht praktisch
getestet. Bruce meldete keine lesbare SD-Karte. Die privaten Sicherungen und
Prüfberichte liegen unter `backups/pager-2026-10-31/` und werden nicht publiziert.

## Zwischenstand: FT8, MeshCator, MeshOS, Roami

Am 31.10.2026 wurde Bruce auf Wunsch wieder entfernt und Roami bytegleich
(aus der verifizierten Sicherung vor Bruce) bei 0xb70000 wiederhergestellt.
Meshtastic bleibt entfernt. FT8 bei 0x1a0000, MeshOS bei 0x460000, MeshCator
bei 0x6e0000 sowie Launcher und aktuelle Datenbereiche bleiben unverändert.
Die OTA-Slots sind FT8=0, Roami=1, MeshOS=2, MeshCator=3. Der Bereich
0x920000..0xb70000 ist gelöscht und frei; Bruces private Speicherpartitionen
existieren nicht mehr. Der Start ist auf Roami gesetzt.

Vorher wurde eine neue vollständige 16-MB-Sicherung erstellt und gegen den
Pager verifiziert. `tools/restore_roami_pager.py` erzeugt standardmäßig nur
einen Plan; `--apply` verlangt eine vollständige Übereinstimmung der aktuellen
Sicherung mit dem Gerät und erhält die aktuellen Daten statt einer globalen
Rücksicherung. Private Sicherung: `backups/pager-2026-10-31/full-before-roami.bin`;
Prüfbericht: `backups/pager-2026-10-31/roami-restore/restoration-report.json`.

## Historischer Zwischenstand: FT8, MeshCator, MeshOS, Bruce

Auf erneuten Wunsch wurde Roami am 31.10.2026 wieder durch die bereits geprüfte
isolierte Bruce-1.16.1-Installation ersetzt. Bruces eigene Einstellungen und
Dateien stammen aus der verifizierten Sicherung vor der Roami-Rückkehr.
FT8, MeshCator, MeshOS, Launcher und deren aktuelle Daten wurden bytegleich
erhalten. Meshtastic und Roami sind nicht mehr installiert.

Werkzeug: `tools/restore_bruce_pager.py`, standardmäßig nur Offline-Plan;
Schreiben erfordert `--apply` und vollständigen Live-Abgleich der aktuellen
16-MB-Sicherung. Sicherung davor: `backups/pager-2026-10-31/full-before-bruce-again.bin`.
Prüfbericht: `backups/pager-2026-10-31/bruce-restore/restoration-report.json`.
Die aktuelle CSV und geschützten Update-Werkzeuge verwenden weiterhin
MeshCator bei 0x6e0000/OTA 3; Bruce liegt bei 0x920000/OTA 1 und ist zum Start gewählt.

## Aktueller Mehrfachinstallationsstand

Launcher, FT8, MeshOS, MeshCator 0.3.25 und Roami sind installiert; Bruce und Meshtastic sind entfernt. Maßgeblich ist `config/partitions-launcher.csv`. MeshCator liegt bei 0x6e0000 (OTA 3), Roami bei 0xb70000 (OTA 1). Die darüber dokumentierten Bruce-/Roami-Wechsel sind historische Zwischenstände. Firmware-Updates wurden nach Sicherung ausschließlich im MeshCator-Slot geschrieben; andere Apps und Daten wurden per Prüfsumme unverändert verifiziert. Private Gerätebackups und Kartenpakete werden nicht veröffentlicht.
