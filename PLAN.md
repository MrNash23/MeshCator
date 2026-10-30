# MeshCator – MeshCore-Firmware für den LilyGo T-LoRa Pager – Projektplan

> **Implementierungsstand:** Ein erster C++/LVGL-Stand mit SDL-Simulator und
> kompilierbarer SX1262-Firmware liegt vor. Build, Bedienung und offene Punkte:
> [`README.md`](README.md). Der folgende Backlog ist weiterhin die Ziel-Roadmap,
> keine Liste bereits implementierter Funktionen.

> Stand: 30.10.2026 · Basis: MeshCore `v1.18.0` (meshcore-dev/MeshCore, MIT)
> Ziel: eigenständige (standalone) MeshCore-Firmware für den T-LoRa Pager, die vom Funktionsumfang her an **Ripple Ultra** und **MeshOS** heranreicht und auf Hardware und Bedienung des Pagers (480×222-Querformat, Scrollrad, QWERTY) zugeschnitten ist.
> **UI/UX:** Stil „Feldinstrument“ (angelehnt an den Enterprise-Tricorder) × Apple-Klarheit. Details in [`DESIGN.md`](DESIGN.md).

---

## 1. Ausgangslage & Recherche

### 1.1 Hardware T-LoRa Pager (Variante SX1262)

| Komponente | Chip | Zweck in der Firmware |
|---|---|---|
| MCU | ESP32-S3, 16 MB Flash, 8 MB PSRAM | LVGL-Puffer und Kartenkacheln im PSRAM |
| Display | 2,33" IPS 480×222, ST7796, ohne Touch | UI im Querformat, Hintergrundbeleuchtung per PWM |
| Eingabe | TCA8418-Tastaturcontroller (QWERTY, beleuchtet) + Drehencoder mit Taster | gesamte Navigation |
| Tasten | Reset, Display an/aus, Power | Sleep, Wake, Shutdown |
| LoRa | SX1262 (433–920 MHz); andere Revisionen mit LR1121 | Mesh-Funk |
| GNSS | u-blox MIA-M10Q | Position, Zeit, Tracks |
| RTC | PCF85063 | Uhrzeit ohne GPS |
| Audio | ES8311-Codec, Lautsprecher, Mikrofon, Kopfhörerbuchse | Töne, später eventuell Sprachnachrichten |
| Haptik | Vibrationsmotor (DRV2605, *in LilyGoLib prüfen*) | stille Benachrichtigung |
| Sensorik | BHI260AP (IMU) | Kompass oder Ausrichtung (optional) |
| NFC | ST25R3916 | Kontakt- und Kanal-Austausch per NFC (optional) |
| Power | BQ25896 (Laden) + BQ27220 (Fuel Gauge), IO-Expander XL9555 für Power-Enables | Akkustand in %, Ladezustand, Peripherie abschalten |
| Speicher | microSD | Karten, Tracks, Backups, Screenshots |

**Pinout und Treiber:** Die Pins nicht neu ermitteln, sondern aus **LilyGoLib** (`T-LoraPager`-Board-Definition) und den bestehenden Pager-Ports übernehmen (siehe 1.3). arduino-esp32 ≥ 3.3.0 wird für die Board-Definition vorausgesetzt.

### 1.2 Referenz-Firmwares (nicht forkbar!)

| | Ripple **Ultra** (Scott Powell / Ripple Radios) | **MeshOS** (Andy Kirby) |
|---|---|---|
| Quellcode | **Closed Source**, nur Binaries | **Closed Source**, nur Releases-Repo `andymux/meshos-releases` |
| Lizenzmodell | kostenlos, Paid-Features per Unlock-Code | kostenlos testen, 8 £ pro Gerät (an die MAC gebunden) |
| Pager-Support | ja (Port der Ultra-Firmware, Beta) | laut Blog ja, laut Release-Notes unklar |
| Stärken | Multi-Profile, Repeater-/Sensor-/Room-Admin, Offline-Karten von SD, GPS-Tracks, Custom Paths, Tags, Kid Mode, Regions/Scopes | Chat-UI mit Bubbles, Lock-Screen, Mesh-Tools (Trace Route, Noise Floor, Signal Meter, Repeater-Scanner), Terminal, URL→QR |

➡️ **Ein „Fork von Ripple“ ist nicht möglich.** Ultra und MeshOS dienen nur als **Feature-Referenz**. Code darf aus ihnen nicht übernommen werden.

### 1.3 Offene Basis-Projekte (forkbar)

| Repo | Lizenz | Stand | Bemerkung |
|---|---|---|---|
| `meshcore-dev/MeshCore` | MIT | aktiv, v1.18.0 | Protokoll, `companion_radio` (BLE/USB-Protokoll für die App), `ui-new`. **Keine Pager-Variante upstream**, nur `lilygo_tdeck` |
| `dcooper94/meshcore-lilygo-pager-SX1262` | MIT | Push 13.10.2026, sehr klein | Fork des LR1121-Ports für SX1262, LVGL-Tile-UI, GPS, Karte |
| `CJvanSoest/meshcore-lilygo-pager-LR1121` | MIT | Push 18.09.2026 | Original des obigen Ports (Tiles: Radio, Channels, DM, Contacts, Discovered, Map, Settings) |
| `BamBam1121/inw-mesh` („Squatch Mesh“) | **GPL-3.0** | sehr aktiv (29.10.2026) | Pager-Firmware mit Messaging, Karte, Region Scopes, Self-Update und Web-Installer; BLE-Companion kompatibel |
| `868meshbot/Saitama` | GPL-3.0 | aktiv | nur T-Deck, gute UI-Referenz |
| `Joel-Claw/OpenMeshOS` | WTFPL | Juli 2026 | T-Deck, LVGL 9, MeshCore als Submodule (Architektur-Vorbild) |

---

## 2. Empfohlenes Vorgehen

### 2.1 Optionen

| Option | Pro | Contra |
|---|---|---|
| **A) Eigenes Repo, MeshCore als Git-Submodule, Pager-HAL aus den MIT-Ports übernehmen** ⭐ | saubere Trennung, Upstream-Updates per `git submodule update`, freie Lizenzwahl, Architektur von Anfang an für den vollen Ultra-Umfang | etwa 2–3 Wochen mehr Startaufwand |
| B) Fork von `dcooper94/…-SX1262` | läuft sofort auf der Hardware, MIT | kleine, junge Codebasis; MeshCore vermutlich eingebettet statt als Submodule, Upstream-Merges werden schmerzhaft |
| C) Fork von `inw-mesh` | größter Funktionsumfang aller offenen Pager-Firmwares | **GPL-3.0 wird auf das eigene Projekt übertragen**, fremde Architektur, viel bestehender Code zum Einarbeiten |
| D) Fork von MeshCore + neue Variante + `ui-new` erweitern | nah an Upstream, eventuell als PR zurückführbar | `ui-new` ist eine kleine Mono-UI (etwa 1.200 Zeilen) für OLED und kein LVGL; der volle Umfang passt dort nicht hinein |

### 2.2 Empfehlung: Option A (Hybrid)

1. **Neues Repo** `pager-meshcore` (MIT oder Apache-2.0), PlatformIO.
2. **MeshCore als Submodule** (`lib/MeshCore`), auf einen Release-Tag gepinnt. Patches nur über Unterklassen bzw. Hooks. Unvermeidbare Änderungen als kleine PRs upstream einreichen.
3. **Hardware-Schicht** (Pins, Power-Sequenz, Display-Init, TCA8418, Encoder) aus `dcooper94/CJvanSoest` (MIT, mit Attribution) und **LilyGoLib** übernehmen.
4. **Mesh-Logik** auf `examples/companion_radio/MyMesh` + `DataStore` aufbauen. Damit bleibt die Firmware **gleichzeitig BLE-Companion** für die offiziellen MeshCore-Apps, ein großer Vorteil gegenüber reinen Standalone-Firmwares.
5. **UI komplett neu mit LVGL 9**, in Komponenten und Screens aufgeteilt, für 480×222 und Encoder-Fokusnavigation (LVGL-Groups).
6. `inw-mesh` (GPL) und Saitama **nur als Ideenquelle** nutzen, keinen Code kopieren, solange das eigene Projekt nicht GPL ist.

### 2.3 Zielarchitektur

```
pager-meshcore/
├── lib/MeshCore/                 (git submodule, gepinnt)
├── src/
│   ├── main.cpp                  Boot, Task-Setup
│   ├── hal/                      Board-Abstraktion (nur hier Hardware-Zugriffe)
│   │   ├── PagerBoard.{h,cpp}    Power (XL9555, BQ25896, BQ27220), Sleep, Buttons
│   │   ├── Display.{h,cpp}       ST7796 via LovyanGFX → LVGL-Flush, Backlight
│   │   ├── Input.{h,cpp}         TCA8418 + Encoder → LVGL indev (keypad + encoder)
│   │   ├── Audio.{h,cpp}         ES8311, Töne/MP3 von SD
│   │   ├── Haptics.{h,cpp}
│   │   ├── Gnss.{h,cpp}          u-blox, NMEA/UBX, Track-Recorder
│   │   └── Storage.{h,cpp}       LittleFS + SD (FAT32), Pfad-Abstraktion
│   ├── mesh/
│   │   ├── PagerMesh.{h,cpp}     erbt/adaptiert companion_radio::MyMesh
│   │   ├── Profiles.{h,cpp}      Multi-Netzwerk-Profile (sandboxed)
│   │   ├── MessageStore.{h,cpp}  persistente Chat-Historie (SD/LittleFS)
│   │   ├── AdminSession.{h,cpp}  Repeater/Room/Sensor-Login + CLI
│   │   └── EventBus.{h,cpp}      Mesh → UI Events (Queue, thread-safe)
│   ├── ui/
│   │   ├── theme/                Design-Tokens (tokens.h), Fonts, Icon-Font → DESIGN.md
│   │   ├── widgets/              Komponenten-Bibliothek (DESIGN.md Kap. 6)
│   │   ├── screens/              ein File pro Screen (siehe Kap. 4)
│   │   ├── i18n/                 Strings DE/EN
│   │   └── Nav.{h,cpp}           Screen-Stack, Back, Shortcuts, Befehlspalette
│   └── apps/                     Tools: Map, Tracks, TraceRoute, NoiseFloor, Terminal
├── tools/                        Tile-Downloader, Track→KML, Screenshot-Konverter
├── web-flasher/                  ESP Web Tools Manifest
└── docs/
```

**Grundregeln**
- LVGL läuft in **einem** Task (Core 1), Mesh/Radio in einem anderen (Core 0). Kommunikation nur über `EventBus` (FreeRTOS-Queues), kein direkter UI-Zugriff aus Mesh-Callbacks.
- LVGL-Draw-Buffer und Kachel-Cache im PSRAM.
- Alle Einstellungen versioniert speichern (Schema-Version + Migration).
- Zwei Build-Varianten wie bei Ultra: **Standard** (Config im Flash) und **`-SD`** (Config, Kontakte und Historie auf SD).

---

## 3. Funktionsumfang (Feature-Backlog)

Priorität: **P0** = MVP · **P1** = Ultra/MeshOS-Parität · **P2** = Mehrwert und Pager-spezifisch · **P3** = nice to have.
Quelle: **U** = Ripple Ultra, **M** = MeshOS, **N** = neu / Pager-spezifisch.

### 3.1 System & Hardware
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| S1 | Boot, Power-Sequenz, Peripherie-Enables über XL9555 | P0 | N |
| S2 | Display + LVGL 9, Backlight-Dimmung, Auto-Screen-Off (Timeout einstellbar) | P0 | U |
| S3 | Tastatur (TCA8418) inkl. Alt/Shift/Sym-Layer, Tastaturbeleuchtung | P0 | U |
| S4 | Encoder: drehen = Fokus, drücken = OK, lang drücken = Kontextmenü/Clipboard | P0 | U |
| S5 | Tasten: Display an/aus, lang drücken = Shutdown; Power-on | P0 | U |
| S6 | Akku in % (BQ27220), Ladeanzeige, Tiefentladeschutz | P0 | N |
| S7 | Light-Sleep bei Screen-Off, Wake per Taste, Encoder oder eingehender Nachricht; LoRa bleibt im RX | P1 | U |
| S8 | Uhrzeit: RTC ↔ GPS ↔ Mesh-Zeit synchronisieren, Zeitzone | P0 | U |
| S9 | Töne über ES8311 (Nachricht, Advert, Boot), Lautstärke, Kopfhörer-Erkennung | P1 | U/M |
| S10 | Eigene Sounds als MP3 von SD (`alert.mp3`, `new-advert.mp3` …) | P2 | U |
| S11 | Vibration für stille Benachrichtigung | P1 | N |
| S12 | Screenshot (Alt+Cap) auf SD | P2 | U |
| S13 | OTA-Update: von SD, über WLAN, über BLE (MeshCore-App DFU-kompatibel prüfen) | P1 | N |
| S14 | Web-Flasher (ESP Web Tools), Merged- und Update-Binaries | P1 | U |
| S15 | Backup/Restore von Identität, Kontakten und Kanälen auf SD | P1 | N |
| S16 | Factory Reset | P0 | – |
| S17 | Lock-Screen (Uhrzeit, Akku, Mesh-Signal, ungelesene Nachrichten), optional mit PIN | P1 | M |
| S18 | First-Boot-Wizard + Hilfe-Tutorial | P0 | U/M |

### 3.2 Identität & Netzwerk-Profile
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| I1 | Schlüsselpaar erzeugen, Name setzen | P0 | U/M |
| I2 | Radio-Settings: Frequenz, BW, SF, CR, TX-Power; Presets (EU 868 Narrow, US 915 …) | P0 | U/M |
| I3 | **Multi-Profile**: mehrere Netze, jeweils mit eigener Identität, eigenen Kontakten, Kanälen und Prefs (sandboxed); Wechsel mit Reboot | P1 | U |
| I4 | Advert: manuell (Zero-Hop/Flood), Auto-Advert (nach Boot, dann alle X h) | P0 | U |
| I5 | Position im Advert: keine, manuell oder GPS | P1 | U |
| I6 | Telemetrie-Berechtigungen (Base / Location / Environment × Deny / Base / Base+GPS) | P1 | U |
| I7 | Optimise: RX-Delay, Interference-Threshold, AGC-Reset-Intervall, Double-ACKs | P2 | U |
| I8 | Regions/Scopes (MeshCore `RegionMap`), Scope pro Kanal | P1 | U |
| I9 | Off-Grid Client Repeat (Client wiederholt Pakete in definierter Region) | P2 | U |
| I10 | Identität als QR-Code bzw. per NFC teilen | P2 | N |

### 3.3 Kontakte & Discover
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| K1 | Kontaktliste mit Typ-Icons (Client, Repeater, Room, Sensor), zuletzt gehört, Hops | P0 | U |
| K2 | Filter: Alle / Clients / Repeater / Rooms / Sensoren / Favoriten / Tag | P1 | U |
| K3 | Buchstaben-Sprung (A–Z), `@` = nach letztem Advert sortieren | P0 | U |
| K4 | Discover: gehörte Adverts, Zeit seit Empfang, Hops, SNR/RSSI, bekannte Kontakte grün markiert | P0 | U/M |
| K5 | Node-Details: hinzufügen/entfernen, Advert teilen, Pfad, Position auf Karte, Distanz/Peilung | P0 | U/M |
| K6 | Tags (bis 3, je 9 Zeichen) + lokale Notizen pro Kontakt | P2 | U |
| K7 | Auto-Add-Policy (alle, nur Clients, manuell) | P1 | N |
| K8 | Kontakt per QR/NFC/SD importieren und exportieren | P2 | N |
| K9 | Blockieren/Stummschalten | P1 | N |

### 3.4 Nachrichten (Direkt)
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| D1 | Chat-Ansicht mit Bubbles, Zeitstempeln (umschaltbar), Zustellstatus (gesendet, ACK, fehlgeschlagen) | P0 | U/M |
| D2 | Senden mit Retry (bis zu 4 Versuche), Direkt → Flood-Fallback, „Reset Path“ | P0 | U |
| D3 | Pfad-Anzeige: direkt oder Flood + Hops; Out-Path `[n]` / `[?]` | P1 | U |
| D4 | Nachrichten-Details: Zeit, Pfad, SNR, „auf Karte zeigen“ bei Koordinaten im Text | P1 | U |
| D5 | Persistente Historie pro Kontakt (Ringpuffer, SD bei `-SD`-Build) | P0 | N |
| D6 | Ungelesen-Zähler, Benachrichtigungs-Popup, Display und Tastaturlicht wecken | P0 | M |
| D7 | Antworten/Zitieren, Quick-Replies (vordefinierte Texte) | P2 | M |
| D8 | Emoji-Font + Eingabe-Picker, UTF-8 (Umlaute!) | P1 | M |
| D9 | Lange Nachrichten: Zeilenumbruch statt horizontalem Scroll (Vorteil gegenüber Ultra), Zeichen- und Byte-Zähler | P0 | N |
| D10 | URL/Koordinate in der Nachricht → als QR anzeigen | P2 | M |
| D11 | Clipboard: kopieren und einfügen (Nachricht, Koordinate, Key, Identität) | P1 | U |
| D12 | Custom Paths: manuell (Hash-Liste), benannte gespeicherte Pfade, Flood/Direct-Schnellwahl | P2 | U |

### 3.5 Kanäle (Gruppen)
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| C1 | Public-Kanal ab Werk | P0 | U |
| C2 | Eigene Kanäle: Name + Key (Hex/Base64) oder generiert (128/256 bit) | P0 | U |
| C3 | Hashtag-Kanäle (`#name` → abgeleiteter Key) | P0 | U |
| C4 | Mehr als 6 Kanäle (MeshCore erlaubt `MAX_GROUP_CHANNELS=40`) | P1 | N |
| C5 | Bestätigungszähler + „Heard by“-Liste (welche Repeater wiederholt haben) | P1 | U |
| C6 | Kanal pro Scope/Region | P1 | U |
| C7 | Kanal teilen per QR/NFC | P2 | N |
| C8 | Mute pro Kanal, Benachrichtigungsregeln (nur Erwähnung `@name`) | P1 | N |

### 3.6 Rooms, Repeater- & Sensor-Admin
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| R1 | Room-Server: Login (Gast/Admin), Posts lesen/schreiben, Sync | P1 | U |
| R2 | Room-Admin-CLI | P2 | U (paid) |
| R3 | Repeater-Login, Telemetrie, „Get Stats“, Batterie in der Titelzeile | P1 | U (paid) / M |
| R4 | Repeater-CLI mit lesbarer Ausgabe (`neighbors`, `get`, `get acl`, `setperm`) + Befehls-Vorlagen | P1 | U (paid) |
| R5 | Repeater-Uhr synchronisieren, Advert auslösen, Reboot | P1 | M |
| R6 | Sensor-Admin: ACL, Admin Sign-Up, `get mma`, `setperm`-Shortcuts | P2 | U (paid) |
| R7 | Sensor-Alerts (Low/High-Priority, Darstellung in Orange) | P2 | U |
| R8 | Gespeicherte Admin-Passwörter (verschlüsselt im Profil) | P1 | N |
| R9 | Remote Admin / **Kid Mode** (Gerät gesperrt, nur Kontakte mit Tag als Kacheln) | P3 | U |

### 3.7 Mesh-Tools
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| T1 | Trace Route mit SNR pro Hop | P1 | M |
| T2 | Noise-Floor-Monitor (Live-Graph) | P1 | M |
| T3 | Mesh-Signal-Meter (zuletzt gehörter Repeater, Zeit, SNR) | P1 | M |
| T4 | Repeater-Scanner + Whitelist | P2 | M |
| T5 | Last Heard (Stationen, Signal, Distanz) | P1 | M |
| T6 | Paket-Log / Terminal (farbcodiert, CLI, Spiegelung auf USB-Serial) | P2 | M |
| T7 | Statistik: TX/RX-Zähler, Airtime, Duty-Cycle (EU-868-relevant!) | P1 | N |
| T8 | Range-Test / Ping (automatisch, Log auf SD) | P3 | N |

### 3.8 GPS & Karte
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| G1 | GPS-Status: Fix, Satelliten, HDOP, Sentences-Zähler, Baudrate | P0 | U |
| G2 | GPS an/aus/Intervall (Stromsparen) | P1 | N |
| G3 | Offline-Karte: Raster-Kacheln von SD `/tiles/{z}/{x}/{y}.png`, Zoom per Encoder, Pan per Tasten | P1 | U/M |
| G4 | Kontakt-Pins, eigene Position, Follow-Modus (`P`) | P1 | U |
| G5 | Grob-Weltkarte im Flash als Fallback ohne SD | P2 | M |
| G6 | GPS-Tracks: aufnehmen, laden, umbenennen, löschen, Höhenprofil | P2 | U |
| G7 | Track-Export GPX/KML direkt auf SD | P2 | N |
| G8 | Wegpunkte setzen und per Nachricht teilen | P2 | N |
| G9 | Kompass/Peilung zu Kontakt (BHI260AP) | P3 | N |
| G10 | Tile-Downloader-Skript (OSM, unter Beachtung der Nutzungsbedingungen) | P1 | U |

### 3.9 Konnektivität
| # | Funktion | Prio | Quelle |
|---|---|---|---|
| B1 | **BLE-Companion-Modus** (offizielle MeshCore-Apps) parallel zur Standalone-UI | P1 | N |
| B2 | USB-Serial-Companion (MeshCore-Config-Tool, Web-Client) | P1 | N |
| B3 | WLAN: nur für OTA und Zeit-Sync (optional) | P3 | N |
| B4 | NFC: Kontakt- und Kanal-Austausch durch Aneinanderhalten zweier Pager | P3 | N |

---

## 4. Menü- und Screen-Struktur

> Visuelle Gestaltung, Layout-Raster, Navigationsmodell (Modi, Aktionsmenü, **Befehlspalette** `/`) und Wireframes siehe [`DESIGN.md`](DESIGN.md).

**Layout 480×222:** Statusleiste 18 px oben (Profil, Uhrzeit, Ungelesen, GPS, BLE, Akku). Darunter der Inhaltsbereich. Optional unten eine Hinweisleiste mit Key-Hints (z. B. `⏎ Öffnen  ⌫ Zurück  ␣ Seite`).

**Globale Bedienung**

| Eingabe | Aktion |
|---|---|
| Encoder drehen | Fokus hoch/runter, auf der Karte zoomen |
| Encoder drücken / Enter | Auswahl |
| Encoder lang / Enter lang | Kontextmenü |
| Backspace lang (> 300 ms) | Zurück |
| Space | nächste Home-Seite |
| A–Z (in Listen) | Sprung zum Eintrag |
| `@` | Liste nach Aktualität sortieren |
| Alt + Cap | Screenshot |
| Tippen in einer Liste | direkt Schnellsuche/Filter (Pager-Vorteil Tastatur) |
| Taste Mitte kurz / lang | Display aus / Shutdown-Dialog |

```
Boot
├── First-Boot-Wizard
│   ├── Sprache (DE/EN)
│   ├── Name
│   ├── Region/Preset (Frequenz, BW, SF, CR)
│   ├── Profilname
│   ├── Schlüssel erzeugen / aus SD-Backup importieren
│   └── Hilfe-Tutorial (Bedienung) → erstes Advert senden?
│
├── Lock-Screen (Uhr, Akku, Signal, Ungelesen)
│
└── Home  (Seiten per Space)
    ├── [1] Chats          – kombinierte Liste: Kanäle + DMs, sortiert nach letzter Aktivität, Ungelesen-Badge
    ├── [2] Kontakte       – Filter-Tabs (Alle/Clients/Repeater/Rooms/Sensoren/Fav/Tag)
    ├── [3] Scan           – Radar + Liste gehörter Adverts; Send ID Local / Broadcast
    ├── [4] Karte          – Pins, eigene Position, Track-Overlay
    ├── [5] Tools          – siehe unten
    └── ☰ Menü (lang drücken auf Home)
        ├── Netzwerke/Profile    – Liste, Neu, Bearbeiten, Wechseln (Reboot)
        ├── Identität            – Name, Auto-Advert, Position im Advert, QR anzeigen
        ├── Funk                 – Frequenz, BW, SF, CR, TX-Power, Presets
        ├── Kanäle               – Liste, Neu (Name/Key/#Hashtag/Generieren), Scope, Mute, Teilen
        ├── Regionen/Scopes
        ├── Benachrichtigungen   – Ton, Lautstärke, Vibration, Wake-on-Message, Regeln
        ├── Display              – Helligkeit, Timeout, Theme (Nacht/Tag/Rotlicht), Schriftgröße, Hinweisleiste, Bewegung reduzieren, Tastaturlicht
        ├── GPS                  – an/aus, Intervall, Baudrate, GPS-Info
        ├── Zeit                 – Zeitzone, Zeitquelle, manuell setzen
        ├── Telemetrie           – Berechtigungen
        ├── Tags                 – 3 Tags verwalten
        ├── Optimieren           – RX-Delay, Interference-Threshold, AGC-Reset, Double-ACK
        ├── Konnektivität        – BLE-Companion an/aus + PIN, USB-Companion, WLAN
        ├── Speicher             – SD-Status, Backup, Restore, Historie löschen
        ├── Kid Mode / Sperre
        ├── System               – Firmware-Info, OTA-Update, Neustart, Factory Reset
        └── Hilfe                – Tastenbelegung, Tutorial erneut

Chat-Screen (DM/Kanal)
├── Verlauf (Bubbles, Status-Icons, Pfad-Info)
├── Eingabefeld (mehrzeilig, Zähler, Emoji-Picker per Sym-Taste)
└── Kontextmenü: Details · Kopieren · Antworten · Pfad… (Flood/Direct/Custom/Gespeichert)
                 · Reset Path · Kontakt-Info · Mute · Historie löschen · Heard by (Kanal)

Node-Details
├── Name, Typ, Key-Prefix, zuletzt gehört, Hops, SNR, Position/Distanz
└── Aktionen: Nachricht · Hinzufügen/Entfernen · Favorit · Tags · Notiz · Teilen (Advert/QR/NFC)
              · Auf Karte · Trace Route · Telemetrie anfordern · Login (Repeater/Room/Sensor)

Admin-Session (Repeater / Room / Sensor)
├── Übersicht: Telemetrie, Stats, Batterie, Uptime
├── CLI: Eingabe + Verlauf + Vorlagen-Liste
└── Aktionen: Uhr sync · Advert · Neighbors · ACL · Reboot

Tools
├── Trace Route
├── Noise Floor
├── Signal Meter / Last Heard
├── Repeater-Scanner
├── Tracks (Aufnehmen/Stop/Laden/Umbenennen/Löschen/Export)
├── Paket-Log / Terminal
└── Statistik (Airtime, Duty-Cycle, Zähler)
```

---

## 5. Roadmap & Meilensteine

| Phase | Inhalt | Ergebnis / Abnahme | Aufwand (grob) |
|---|---|---|---|
| **0 – Setup** | Repo, PlatformIO, MeshCore-Submodule, CI (GitHub Actions baut `.bin`), Lizenz/Attribution | grüner CI-Build | 2–3 Tage |
| **1 – HAL** | Power, Display + LVGL, Tastatur, Encoder, Buttons, Akku, RTC, SD, Hintergrundbeleuchtung | Demo-Screen, Tastendruck sichtbar, Akku in % | 1–2 Wochen |
| **1b – Design-System** | Moodboard, Tokens, Fonts, Icon-Font, 1:1-Mockups der Schlüssel-Screens, LVGL-Komponenten-Galerie, SDL-Simulator | Mockups abgenommen, Typo auf echtem Display geprüft | 1–2 Wochen (parallel zu 1) |
| **2 – Mesh-Kern** | `PagerMesh` auf `companion_radio` aufbauen, Radio SX1262, DataStore, EventBus, Adverts, Kontakte | Pager erscheint in der MeshCore-App eines anderen Nodes; Serial-Log | 1 Woche |
| **3 – MVP-UI (P0)** | Wizard, Home (Chats/Kontakte/Discover), Chat-Screen, Kanäle, Benachrichtigungen, Settings Basis, Historie | **Alltagstauglich:** DM + Public + Hashtag-Kanal mit T-Deck/App getestet → Release `v0.1` | 3–4 Wochen |
| **4 – Power & Robustheit** | Light-Sleep mit RX-Wake, Screen-Off, Lock-Screen, Watchdog, Settings-Migration, Backup/Restore | ≥ 2 Tage Standby (Ziel messen), keine Abstürze im 48-h-Test | 1–2 Wochen |
| **5 – Parität I** | Multi-Profile, Repeater-/Room-Login + CLI, Regionen/Scopes, Heard-by, Clipboard, BLE-Companion | `v0.5` | 3–4 Wochen |
| **6 – GPS & Karte** | GPS-Info, Offline-Kacheln, Pins, Tracks, Export, Tile-Tool | Karte flüssig (≥ 15 fps beim Pan) | 2–3 Wochen |
| **7 – Mesh-Tools** | Trace Route, Noise Floor, Signal Meter, Scanner, Terminal, Statistik | `v0.8` | 2 Wochen |
| **8 – Feinschliff** | Audio/MP3, Vibration, Emoji, Custom Paths, Tags/Notizen, Sensor-Admin, OTA, Web-Flasher, Doku | `v1.0` | 3–4 Wochen |
| **9 – Extras (P3)** | NFC-Austausch, Kompass, Kid Mode, Range-Test, LR1121-Variante | laufend | – |

**Gesamt bis v1.0:** grob 4–6 Monate bei Teilzeit-Entwicklung.

---

## 6. Technische Entscheidungen & Risiken

| Thema | Entscheidung / Maßnahme |
|---|---|
| Display-Treiber | **LovyanGFX** (ST7796, DMA) als LVGL-Flush. Alternative: Arduino_GFX. Die Adafruit-ST7789-Klasse aus MeshCore ist für LVGL zu langsam |
| LVGL-Version | 9.x; Fonts mit deutschem Zeichensatz + Emoji-Subset per `lv_font_conv`; eventuell Tiny-TTF aus PSRAM |
| Arduino-Core | arduino-esp32 ≥ 3.3 (vom Pager gefordert). **Risiko:** MeshCore-`esp32_base` nutzt eventuell einen älteren Core → früh prüfen (Phase 0) |
| Upstream-Kompatibilität | Nur über Vererbung/Hooks an MeshCore andocken; bei jedem MeshCore-Release Merge-Test in CI |
| Speicher | Kontakte/Kanäle über MeshCore-`DataStore` (LittleFS); Historie und Karten auf SD; PSRAM für UI-Puffer |
| Duty-Cycle EU 868 | Airtime-Tracking (T7) und Warnung, damit der Nutzer im legalen Rahmen bleibt |
| Stromverbrauch | Unbenutzte Peripherie (NFC, IMU, Codec, GPS) über XL9555 abschalten; SX1262 im RX-Duty-Cycle-Modus prüfen |
| Lizenz | Eigenes Projekt MIT; Attribution für MeshCore und die Pager-Ports; **kein** Code aus GPL-Projekten oder Closed-Source-Firmwares |
| Hardware-Varianten | Radio über Build-Flags abstrahieren (SX1262 jetzt, LR1121 später) |
| Testen | Native Unit-Tests (PlatformIO `native`) für MessageStore/Parser; LVGL-Simulator (SDL) für die UI am Mac → schnellere Iteration |

---

## 7. Nächste konkrete Schritte

1. `git init`, PlatformIO-Projekt anlegen, MeshCore als Submodule hinzufügen (Tag `v1.18.0`).
2. `dcooper94/meshcore-lilygo-pager-SX1262` und **LilyGoLib** klonen; Pinout und Init-Sequenz nach `src/hal/` übertragen.
3. Prüfen, ob MeshCores `esp32_base` mit arduino-esp32 3.3 baut (größtes Frühphasen-Risiko).
4. LVGL-„Hello Pager“: Statusleiste + Liste, Fokussteuerung über Encoder.
5. `PagerMesh` minimal: Advert senden/empfangen, Kontakt in Liste anzeigen.
6. LVGL-SDL-Simulator für den Mac einrichten (480×222).
7. Design-Tokens und Fonts aus `DESIGN.md` umsetzen, Schlüssel-Screens als 1:1-Mockup bauen und abnehmen.

---

### Quellen
- MeshCore: <https://github.com/meshcore-dev/MeshCore>
- Ripple Ultra Guide v7.14: <https://buymeacoffee.com/ripplebiz/ultra-v7-7-guide-meshcore-users>
- Ripple T-LoRa Pager Guide: <https://buymeacoffee.com/ripplebiz/t-lora-pager-guide>
- MeshOS (Beschreibung): <https://nodakmesh.org/blog/meshos-t-deck-off-grid-messenger> · Releases: <https://github.com/andymux/meshos-releases/releases>
- LilyGo T-LoraPager Wiki: <https://wiki.lilygo.cc/get_started/en/LoRa_GPS/T-LoraPager/T-LoraPager.html>
- Pager-Ports: <https://github.com/dcooper94/meshcore-lilygo-pager-SX1262> · <https://github.com/CJvanSoest/meshcore-lilygo-pager-LR1121> · <https://github.com/BamBam1121/inw-mesh>
- Weitere: <https://github.com/868meshbot/Saitama> · <https://github.com/Joel-Claw/OpenMeshOS> · <https://github.com/meshcore-ita/awesome-meshcore>
