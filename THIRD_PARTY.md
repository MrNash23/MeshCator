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
ASCII + Latin-1 + Euro. Lizenz: SIL OFL, `src/ui/fonts/OFL.txt`.

## Kartenatlas und Haptik (Audit 2026-10-30)

Natural Earth Land / Lakes 1:50 Millionen: Public Domain, siehe
https://www.naturalearthdata.com/about/terms-of-use/ . Eigene Kachelerzeugung,
keine fremden gerenderten Karten. Herkunft und Hashes: assets/maps/tiles-manifest.json.
Adafruit DRV2605 Library 1.2.4: BSD-Lizenz, Original-Lizenz liegt in der
PlatformIO-Abhängigkeit; https://github.com/adafruit/Adafruit_DRV2605_Library .
Zlib wird im Desktop zum Entpacken verwendet (zlib-Lizenz), auf dem ESP32-S3
die vorhandene ROM-miniz-Funktion. Pillow ist nur ein Werkzeug zur Asset-Erzeugung.

## Eigene Emoji-Symbole

`tools/build_emoji_font.py` erzeugt zwölf selbst gezeichnete monochrome Symbole
und den Variationsselektor als LVGL-Fallbackfont. Keine Emoji-Grafiken aus einem
fremden Font; die bestehenden Montserrat-Textschriften bleiben unter SIL OFL.
