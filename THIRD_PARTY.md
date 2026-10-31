# Abhängigkeiten und Herkunft

## Vendored MeshCore-Pager-Unterbau

Quelle: https://github.com/dcooper94/meshcore-lilygo-pager-SX1262
Commit: `ed61285b78145c828a3543c0aeaddc41b07bca5b`
Lizenz: MIT, vollständiger Text unter `vendor/meshcore-pager/license.txt`.
Copyright Scott Powell / rippleradios.com und weitere Autoren in den Quelltexten.
Enthält den MeshCore-Kern, companion_radio und den SX1262-Pager-Port.

Übernommen wurden src, examples, variants, arch, include, lib und boards sowie
Build-Metadaten. Lokale Anpassungen isolieren SPIFFS über MESHCATOR_FS_LABEL,
SD-Dateien unter /meshcator und die Power-Kalibrierung im NVS-Namespace mc_power. Ein optionaler Hook leitet Druck- und Loslassereignisse der Zurück-/Löschtaste
an MeshCator weiter. Tastatur-Debugausgaben wurden entfernt, um den USB-Protokollstrom nicht zu stören. Der originale
UITask wird nicht kompiliert; `src/hal/PagerApp.cpp` bindet die eigene Oberfläche
an dieselben Companion-Hooks an. Der Startbildschirm ruft optional den eigenen MeshCator-Splash-Hook auf.
Keine Änderungen werden upstream publiziert.

Der geplante Upstream-Tag v1.18.0 war beim Abruf nicht vorhanden. Statt einen
Tag zu erfinden, ist dieser konkrete Port-Commit die initiale Firmware-Basis.
Der neuere upstream MeshCore-Stand und LilyGoLib wurden nur zur Recherche
heruntergeladen, nicht als Laufzeitabhängigkeit eingebaut.

## LVGL und weitere Bibliotheken

LVGL 9.2.2: https://github.com/lvgl/lvgl (MIT), Commit
`7f07a129e8d77f4984fff8e623fd5be18ff42e74`. Simulator und Firmware verwenden
 dieselbe Version. SDL2 ist eine Systemabhängigkeit (zlib-Lizenz).
RadioLib ist auf Commit `6d8934836678d8894e3d556550475b37dce3e2b6` gepinnt.
Die anderen direkten Firmware-Versionen stehen in platformio.ini;
transitive Bibliotheksabhängigkeiten werden von PlatformIO aufgelöst.
Deren Lizenzen bleiben in den heruntergeladenen Bibliotheken erhalten.

## Lokaler Display-Adapter und Schriften

`src/hal/Display.cpp` ist von TPagerLVGL.cpp desselben MIT-Port-Commits
abgeleitet. Änderungen: PSRAM-Puffer und Fehlerprüfung.

Die generierten `src/ui/fonts/meshcator_font_*.c` enthalten Montserrat Medium
aus dem LVGL-9.2.2-Quellbaum, erzeugt mit lv_font_conv 1.5.3, 4 bpp,
ASCII + Latin-1 + Euro, ergänzt um Œ/œ, Ÿ, ẞ und ’. Lizenz: SIL OFL, `src/ui/fonts/OFL.txt`.

## Kartenatlas und Haptik (Audit 2026-10-30)

Natural Earth Land / Lakes 1:50 Millionen: Public Domain, siehe
https://www.naturalearthdata.com/about/terms-of-use/ . Eigene Kachelerzeugung,
keine fremden gerenderten Karten. Herkunft und Hashes: assets/maps/tiles-manifest.json.
Adafruit DRV2605 Library 1.2.4: MIT-Lizenz, Original-Lizenz liegt in der
PlatformIO-Abhängigkeit; https://github.com/adafruit/Adafruit_DRV2605_Library .
Zlib wird im Desktop zum Entpacken verwendet (zlib-Lizenz), auf dem ESP32-S3
die vorhandene ROM-miniz-Funktion. Pillow ist nur ein Werkzeug zur Asset-Erzeugung.

## Emoji-Schrift (ab 0.3.16)

Noto Emoji, Copyright 2013 Google LLC, SIL Open Font License 1.1.
Quelle: https://github.com/google/fonts/tree/main/ofl/notoemoji .
Originalfont und Lizenz: `assets/fonts/noto-emoji/`; SHA-256 und Auswahl im
`manifest.json`. `tools/build_emoji_font.py` erzeugt den modifizierten,
monochromen MeshCator-Fallback (358 Zeichen, 20 px, 2 bpp). Die bisherige
Sammlung eigener zwölf Zeichen wurde ersetzt. Keine Apple-Emoji-Grafiken.

## Credits und Lizenzprüfung 0.3.16

Einstellungen → Info / Credits zeigt die Komponenten, Autoren und Lizenzen
sowie die Adresse des Quellcode-Repositories. `LICENSES/components.json`
ist das gemeinsame Inventar; `tools/generate_credits.py` erzeugt daraus die
Geräteansicht. `LICENSES/` enthält die Originaltexte, Copyright-Header und
das offizielle ESP-IDF-Copyright-Verzeichnis. `linked-archives.json` wurde
mit der Linker-Map des Launcher-Builds abgeglichen. Installierte, aber nicht
verlinkte Build-Abhängigkeiten werden zusätzlich aufgeführt. Die
SDK-Untermodulquellen und Hashes stehen in `sdk-notice-sources.json`.

MeshCators eigener Code steht unter MIT. Das ist keine Umlizenzierung der
Abhängigkeiten: Arduino-ESP32 2.0.17 und MicroNMEA 2.0.6 enthalten LGPL-Code;
GCC-Laufzeitbibliotheken haben eine Runtime Exception. Schriften verwenden
OFL, Kartendaten sind Public Domain. Die Espressif-Funkbibliotheken werden
als Binärbibliotheken geliefert; ihre beigelegten Lizenztexte lauten Apache-2.0.
Sie sind damit nicht als vollständig verfügbarer Quellcode zu bezeichnen.

Die ungenutzten Nordic-Header im vendorten Unterbau haben eine auf Nordic-
Hardware beschränkte Lizenz und werden nicht in die ESP32-Firmware eingebunden.
Sie sind im Inventar ausdrücklich als „source only“ erfasst, nicht als MIT.
Weitere nicht verwendete Plattformverzeichnisse behalten ihre Originalhinweise.

Bei einer öffentlichen Binärveröffentlichung gehören `LICENSE`, `LICENSES/`,
`THIRD_PARTY.md`, der zugehörige Quellstand einschließlich lokaler Änderungen
und die Build-Anleitung zur Veröffentlichung. Insbesondere sind die
LGPL-Quellen samt Möglichkeit zum Neubauen/Neuverlinken bereitzustellen;
eine Credits-Seite allein ersetzt diese Materialien nicht. Diese Änderung
stellt die Hinweise lokal bereit, veröffentlicht aber noch kein Release.

Die Sammlung erfasst den konkreten ESP32-Launcher-Build, seine Fonts/Karten
und die wichtigsten Simulator-/Build-Werkzeuge. Für andere Targets oder
zukünftige Bibliotheksupdates muss das Inventar erneut abgeglichen werden.
