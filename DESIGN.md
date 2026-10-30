# MeshCator – Design- & UX-Konzept

> **Aktuelle Richtung – Designstudie 04:** Eigenständige Feldgerät-Gestaltung mit
> Graphit, Petrol und Limette, weich gerundeten Karten und offenen Inhaltsflächen.
> Die Nutzerentscheidung ersetzt Designstudie 02 und die Franchise-Anlehnung
> der ursprünglichen Beschreibung. Aktuelle Vorgaben: `docs/design/STYLE-V3.txt`.
> Ältere Abschnitte bleiben als Entstehungskontext erhalten.

> Ergänzung zu [`PLAN.md`](PLAN.md). Gilt für alle Screens der Pager-Firmware.
> Leitbild: **„Feldinstrument“**, angelehnt an den Stil des Tricorders aus *Star Trek: Enterprise* (NX-01-Ära), umgesetzt mit der Klarheit und Ruhe eines Apple-Interfaces.

---

## 1. Leitbild

| | Was wir übernehmen | Was wir **nicht** tun |
|---|---|---|
| **Tricorder (Enterprise)** | dunkle, tiefe Flächen; kühle Blau- und Türkistöne mit sparsamen Bernstein-Akzenten; feine 1-px-Linien und Messraster; kleine Labels in Versalien mit Sperrung; Skalen, Segmentbalken, Wellenformen; die Scan-Animation als „das Gerät arbeitet gerade“ | keine Logos, Insignien oder Namen aus dem Franchise; keine 1:1-Nachbildung von Original-Screens; keine Original-Fonts oder -Sounds; keine LCARS-„Elbows“ (das ist eine andere Ära) |
| **Apple** | eine Aufgabe pro Screen; klare typografische Hierarchie; viel Luft; Inhalt vor Dekoration; konsistente Gesten; Progressive Disclosure; ruhige, physikalisch plausible Animationen | keine iOS-Optik kopieren (Glas, Blur, iOS-Icons); keine Touch-Muster, die mit Encoder und Tastatur nicht funktionieren |

**Merksatz:** *Ruhig, wenn man liest. Instrument, wenn man misst.*
Text-Screens (Chats, Einstellungen) sind zurückhaltend und fast „apple-clean“. Die Instrument-Ästhetik (Raster, Skalen, Sweeps) erscheint nur dort, wo echte Messdaten gezeigt werden: Statusleiste, Scan, Karte, Tools, Admin-Telemetrie.

---

## 2. UX-Prinzipien

1. **Eine Aufgabe pro Screen.** Jeder Screen hat genau eine Primäraktion, erreichbar mit `⏎` oder einem Encoder-Klick.
2. **Alles erreichbar, wenig sichtbar.** Sekundäraktionen liegen im **Aktionsmenü** (`⏎` lang). Jede Funktion ist in **maximal 3 Ebenen** erreichbar und zusätzlich über die **Befehlspalette** auffindbar (siehe 4.3).
3. **Tastatur zuerst, Encoder immer.** Jede Funktion ist mit dem Encoder allein bedienbar (Einhand-Betrieb). Die Tastatur ist der Turbo: Losschreiben sucht, Kürzel springen direkt.
4. **Status ist sichtbar, aber leise.** Zustellung, Signal, GPS und Akku werden als kleine, eindeutige Glyphen dargestellt, nie als Popup. Popups gibt es nur für eingehende Nachrichten und echte Fehler.
5. **Nie nur Farbe.** Jeder Zustand hat zusätzlich ein Symbol oder Text (Sonnenlicht, Farbsehschwäche).
6. **Verzeihend.** Destruktive Aktionen brauchen eine Bestätigung oder bieten 5 s lang „Rückgängig“. `⌫` lang führt immer eine Ebene zurück und verliert keine Eingaben (Entwürfe werden gespeichert).
7. **Sofortiges Feedback.** Jede Eingabe zeigt innerhalb von 50 ms eine Reaktion (Fokus, Klick-Ton oder Haptik). Lange Operationen (Login, Trace) zeigen eine Scan-Animation und lassen sich abbrechen.
8. **Sinnvolle Defaults.** Der First-Boot-Wizard fragt nur Name und Region ab. Alles andere funktioniert ohne Konfiguration.

---

## 3. Layout-System (480 × 222 px)

```
 0 ┌────────────────────────────────────────────────────────┐
   │ Statusleiste / Modus-Leiste                       20 px │
20 ├────────────────────────────────────────────────────────┤
   │                                                        │
   │ Inhaltsbereich                                  182 px │
   │ (Raster 4 px, Außenrand 8 px)                          │
   │                                                        │
202├────────────────────────────────────────────────────────┤
   │ Hinweisleiste (Key-Hints), ausblendbar            20 px │
222└────────────────────────────────────────────────────────┘
```

- **Grundraster 4 px**, Außenrand 8 px, Abstand zwischen Elementen 8 px, zwischen Gruppen 16 px.
- **Split-View** (wie beim iPad): Das breite Format wird als **Liste links (176 px) + Detail rechts (288 px)** genutzt (Chats, Kontakte, Einstellungen, Scan). Der Fokus wechselt mit `⏎` nach rechts und mit `⌫` nach links.
- **Vollbild** für Chat-Eingabe, Karte, Lock-Screen und Tools. Status- und Hinweisleiste können hier auf 14 px schrumpfen.
- **Listenzeilen:** 1-zeilig 28 px, 2-zeilig 40 px. Damit passen etwa 4–6 Einträge sichtbar.
- Die Hinweisleiste zeigt kontextabhängig 3–4 Tasten. Sie kann in den Einstellungen ausgeblendet werden („Profi-Modus“), dann wächst der Inhalt um 20 px.

---

## 4. Navigationsmodell

### 4.1 Modi (Top-Level)

Fünf Modi in der Modus-Leiste. Der aktive Modus steht in Versalien und ist hervorgehoben:

`CHATS · Kontakte · Scan · Karte · Tools`

- `␣` (Space) bzw. Encoder im Header wechseln den Modus. Alternativ `Alt+1…5`.
- „Scan“ ersetzt die bisherige Bezeichnung „Discover“. Er ist der zentrale Instrument-Screen.
- Die Einstellungen sind kein Modus, sondern erreichbar über die Befehlspalette, das Aktionsmenü auf Home oder `Alt+S`.

### 4.2 Tiefe & Gesten

| Eingabe | Bedeutung (überall gleich) |
|---|---|
| Encoder drehen | Fokus bewegen / Wert ändern |
| Encoder klick, `⏎` | Primäraktion |
| Encoder lang, `⏎` lang | **Aktionsmenü** (alle Sekundäraktionen) |
| `⌫` lang | zurück (eine Ebene) |
| Losschreiben in Listen | Filtern / Schnellsuche |
| `/` | **Befehlspalette** |
| `␣` | nächster Modus (auf Top-Level) |
| `Alt` + Taste | Kürzel (in den Hints angezeigt) |

### 4.3 Befehlspalette („alles finden“)

Die Palette ist der Schlüssel zu „Apple-einfach, aber alles erreichbar“. Sie öffnet sich mit `/` als Overlay von oben.

```
┌────────────────────────────────────────────────────────┐
│ ›  trac▏                                               │
├────────────────────────────────────────────────────────┤
│ ⌁  Trace Route …                        Tools          │
│ ⟟  Tracks                               Karte          │
│ ●  Tracy (Kontakt)                      2 Hops · 3 min │
│ ⚙  GPS-Track-Aufnahme                   Einstellung    │
└────────────────────────────────────────────────────────┘
```

Durchsucht werden: Kontakte, Kanäle, Einstellungen (jede einzelne!), Aktionen („Advert senden“, „GPS aus“, „Display dunkler“) und Tools. Die letzten Treffer werden zuerst angezeigt.

### 4.4 Aktionsmenü

Ein **Action-Sheet**, das von rechts einfährt (96–160 px breit, Liste). Die Primäraktion steht oben, destruktive Aktionen in Rot ganz unten mit Trennlinie. Maximal 8 Einträge; wenn es mehr sind, kommt ein „Mehr …“-Eintrag.

---

## 5. Visuelle Sprache

### 5.1 Farben (Design-Tokens)

**Nacht (Standard)**

| Token | Wert | Verwendung |
|---|---|---|
| `bg` | `#05080C` | Hintergrund |
| `surface` | `#0B121A` | Flächen, Listen |
| `surface-2` | `#122030` | Fokus-Hintergrund, Sheets |
| `line` | `#1E3246` | Trennlinien, Raster |
| `text` | `#E4EEF5` | Primärtext |
| `text-2` | `#8BA3B6` | Sekundärtext |
| `text-3` | `#526878` | Labels, Deaktiviertes |
| `accent` | `#47C8E0` | Fokus, Scan, Auswahl (Cyan) |
| `accent-2` | `#3474C8` | eigene Nachrichten, Flächen-Akzent (Blau) |
| `attention` | `#F0A23B` | Ungelesen, Warnung, Alerts (Bernstein) |
| `ok` | `#3CCB8A` | zugestellt, Fix, verbunden |
| `danger` | `#E5484D` | Fehler, Löschen |

**Tag** (für Sonnenlicht): heller Hintergrund `#E9EEF2`, Text `#0A1622`, Akzente abgedunkelt. Das Display steht dabei auf 100 %.
**Rotlicht** (Nachtsicht): alles in Rottönen auf Schwarz, minimale Helligkeit.

Regeln: Cyan bedeutet „interaktiv/Fokus“, Bernstein bedeutet „braucht Aufmerksamkeit“. Höchstens **ein** Akzent dominiert pro Screen. Kontrast für Text mindestens 4.5:1.

### 5.2 Typografie

| Rolle | Schrift | Größe | Stil |
|---|---|---|---|
| Groß-Readout (Uhr, Messwerte) | **Inter** Light | 36 / 28 px | Tabular Numbers |
| Titel | Inter SemiBold | 18 px | Satzschreibung |
| Body / Chat | Inter Regular | 14 px (Option: 16 px) | Satzschreibung |
| Sekundär | Inter Regular | 12 px | `text-2` |
| **Label (Instrument)** | **JetBrains Mono** Medium | 10 px | VERSALIEN, +8 % Sperrung, `text-3` |
| Daten (Freq., Keys, Pfade, CLI) | JetBrains Mono | 12 px | Tabular |

Beide Schriften stehen unter der SIL OFL. Fonts werden mit `lv_font_conv` als 4-bpp-LVGL-Fonts erzeugt (Latin-1 + Euro + Pfeile + Emoji-Subset als eigener Font).
→ Die **Mischung** aus ruhiger Grotesk für Inhalt und kleinen Mono-Versalien für Messdaten ergibt genau die Verbindung „Apple × Instrument“.

### 5.3 Instrument-Elemente

| Element | Aussehen | Einsatz |
|---|---|---|
| **Eck-Klammern** `⌜ ⌟` | 6 px lange Ecken in `accent` statt umlaufendem Rahmen | Fokus-Indikator in Listen und Readouts |
| **Segmentbalken** | 5–10 Segmente à 3 px, 1 px Abstand | Akku, Signal, SNR, Fortschritt |
| **Skala mit Ticks** | 1-px-Linie, Haupt-/Neben-Ticks | Noise Floor, Frequenz-Picker, Lautstärke |
| **Readout-Block** | Label (Mono-Versalien) über großem Wert + Einheit | Telemetrie, GPS, Stats |
| **Raster** | 1 px `line`, 50 % Deckkraft, 24-px-Gitter | Hintergrund von Scan, Karte, Graphen |
| **Sweep** | rotierender Radarstrahl bzw. horizontal laufende Scanlinie | **nur** während aktiver Vorgänge (Scan, Login, Trace) |
| **Sparkline / Wellenform** | 1,5-px-Linie `accent`, Fläche 15 % | Noise Floor, Signal-Historie |

### 5.4 Icons

Ein eigener, schlichter Icon-Font auf einem **16-px-Raster** mit 1,5 px Strichstärke, ohne Füllungen. Basis sind Lucide (ISC) oder Tabler (MIT), angepasst. Typ-Icons für Nodes: Client ●, Repeater ◆, Room ▣, Sensor ◎.

### 5.5 Bewegung

- Push/Pop: horizontaler Slide über 160 ms, ease-out. Sheets 140 ms. Modus-Wechsel: Crossfade 120 ms.
- Kein Bounce, kein Overshoot. Instrument-Animationen (Sweep) laufen linear und nur bei echter Aktivität.
- Option „Bewegung reduzieren“ ersetzt alles durch sofortige Wechsel (das spart auch Strom).
- Performance-Budget: ≥ 30 fps beim Scrollen, Partial Refresh, Draw-Buffer im PSRAM.

### 5.6 Ton & Haptik

Eigene, kurze Klänge im „Instrument“-Charakter, aber selbst erstellt (keine Samples aus dem Franchise):

| Ereignis | Ton | Haptik |
|---|---|---|
| Fokus/Klick | sehr leiser Tick (optional) | – |
| Nachricht (DM) | 2-Ton-Chirp aufsteigend | 2× kurz |
| Kanal-Nachricht | 1 weicher Ton | 1× kurz |
| Erwähnung `@name` | wie DM, höher | 2× kurz |
| Neuer Node gehört | dezentes „Ping“ | – |
| Fehler/Alert | 2-Ton fallend | 1× lang |
| Scan läuft | leises Pulsieren (optional) | – |

---

## 6. Komponenten-Bibliothek (LVGL)

`StatusBar` · `ModeStrip` · `HintBar` · `ListRow` (Icon, Titel, Untertitel, Meta rechts, Badge) · `SectionHeader` · `SplitView` · `ChatBubble` (eigene/fremde, Status-Glyphe) · `Composer` (mehrzeilig, Zähler) · `ActionSheet` · `CommandPalette` · `Banner` (Benachrichtigung) · `Toast` (Undo) · `Dialog` · `Readout` · `SegmentBar` · `Gauge`/`Scale` · `Sparkline` · `RadarPlot` · `Toggle` · `ValuePicker` (Encoder-optimiert, Ziffer für Ziffer, z. B. für Frequenzen) · `TextField` · `QRView` · `EmptyState` · `ProgressScan`.

Alle Komponenten beziehen Farben, Abstände und Fonts ausschließlich aus `ui/theme/tokens.h`. Ein Theme-Wechsel ist damit nur ein Tausch der Token-Tabelle.

---

## 7. Schlüssel-Screens (Wireframes 1:1-Proportion)

### 7.1 Chats (Split-View)
```
┌────────────────────────────────────────────────────────┐
│ CHATS  Kontakte  Scan  Karte  Tools     14:32 ▮▮▮▮▯ 78% │
├──────────────────┬─────────────────────────────────────┤
│⌜#berlin       2 ⌟│ #berlin                  ◆ 3 HOPS  │
│ Mia: bin am Tor… │                                     │
│ Public           │ Mia                           14:28 │
│ Jonas: ok danke  │ bin am Tor, wer kommt noch?         │
│ ● Tracy          │                                     │
│ du: bis gleich ✓✓│           Ich, in 10 min.  ▌        │
│                  │                     14:30 · ✓ 3×    │
├──────────────────┴─────────────────────────────────────┤
│ ⏎ Öffnen   ⏎· Aktionen   / Suchen   ␣ Modus            │
└────────────────────────────────────────────────────────┘
```
Ungelesen = Bernstein-Badge. Eigene Nachrichten rechtsbündig mit `accent-2`-Kante. `✓ 3×` = von 3 Repeatern gehört.

### 7.2 Scan (Instrument-Screen)
```
┌────────────────────────────────────────────────────────┐
│ Chats  Kontakte  SCAN  Karte  Tools     14:32 ▮▮▮▮▯ 78% │
├──────────────────────┬─────────────────────────────────┤
│   ·    ·  N  ·    ·  │ NODES 12        LETZTE 15 MIN   │
│ ·    ╭─────────╮   · │⌜◆ RPT-Mitte   −7dB  1H   12s ⌟ │
│    ╭─┼──◆──────┼─╮   │ ● Tracy       −11dB 2H    3m   │
│ W  │ │   ✛ ╲   │ │ E │ ▣ Room-Kiez   −4dB  1H    5m   │
│    ╰─┼────●──  ┼─╯   │ ◎ Wetter-01   −14dB 3H   21m   │
│ ·    ╰─────────╯   · │                                 │
│   ·    ·  S  ·    ·  │ RAD 2 km      SWEEP ▸           │
├──────────────────────┴─────────────────────────────────┤
│ ⏎ Details   A Advert lokal   B Advert Flood   ␣ Modus  │
└────────────────────────────────────────────────────────┘
```
Links ein Radar: Nodes mit Position nach Peilung und Distanz, Nodes ohne Position auf einem Ring nach SNR. Rechts die sortierte Liste. Der Sweep läuft nur, solange gerade ein Advert gesendet bzw. auf Antworten gehört wird.

### 7.3 Lock-Screen
```
┌────────────────────────────────────────────────────────┐
│                                                        │
│                     14:32                              │
│                Donnerstag, 30. Oktober                 │
│                                                        │
│   AKKU          MESH            GPS          NEU       │
│   78 %          ▮▮▮▮▯ −8 dB     8 SAT        2 ✉       │
│                                                        │
│                 ⏎ zum Entsperren                       │
└────────────────────────────────────────────────────────┘
```

### 7.4 Benachrichtigung (Banner)
Fährt in 140 ms von oben ein, verdeckt die Statusleiste und bleibt 4 s sichtbar. `⏎` öffnet den Chat, `⌫` schließt.
```
┌────────────────────────────────────────────────────────┐
│ ● Tracy · DM · 2 Hops                          jetzt   │
│ Treffpunkt verschoben auf 15:30                        │
└────────────────────────────────────────────────────────┘
```

### 7.5 Einstellungen (Split-View, Apple-Stil)
```
┌────────────────────────────────────────────────────────┐
│ Einstellungen                           14:32 ▮▮▮▮▯ 78% │
├──────────────────┬─────────────────────────────────────┤
│ PROFIL           │ FUNK                                │
│ Identität        │ Preset          EU 868 Narrow    ›  │
│⌜Funk           ›⌟│ Frequenz        869.618 MHz      ›  │
│ Kanäle           │ Bandbreite      62.5 kHz         ›  │
│ GERÄT            │ Spreizfaktor    SF8              ›  │
│ Display          │ Sendeleistung   22 dBm  ▮▮▮▮▮▮▮▯    │
│ Benachrichtig.   │ Airtime 1 h     2,1 % von 10 %      │
├──────────────────┴─────────────────────────────────────┤
│ ⏎ Ändern   ⌫ Zurück   / Einstellung suchen             │
└────────────────────────────────────────────────────────┘
```
Werte stehen inline rechts (wie bei iOS). Bearbeitet werden sie über den `ValuePicker` als Overlay, die Frequenz Ziffer für Ziffer mit dem Encoder.

### 7.6 Tool: Noise Floor
```
┌────────────────────────────────────────────────────────┐
│ ‹ Tools  NOISE FLOOR                    14:32 ▮▮▮▮▯ 78% │
├────────────────────────────────────────────────────────┤
│ AKTUELL          MIN 60 S        MAX 60 S       KANAL  │
│ −112 dBm         −118            −96            869.618│
│ ─90 ┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄╭╮┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄ │
│─105 ┼┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄╭╯╰╮┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄ │
│─120 ┼──╮╭──╮╭─────╮╭───────╯   ╰──────╮╭───────────── │
│     └┴────┴────┴────┴────┴────┴────┴────┴────┴────┴─ │
├────────────────────────────────────────────────────────┤
│ ␣ Pause   ⏎· Aktionen   ⌫ Zurück                       │
└────────────────────────────────────────────────────────┘
```

---

## 8. Barrierefreiheit & Lesbarkeit

- Zwei Schriftgrößen-Stufen (Standard / Groß). Das Layout kippt bei „Groß“ von Split-View auf eine einspaltige Ansicht.
- Kontrast ≥ 4.5:1 für Text, ≥ 3:1 für UI-Elemente; Status nie nur über Farbe.
- Tag-Modus für Sonnenlicht, Rotlicht-Modus für die Nacht.
- Alle Töne und Haptik einzeln abschaltbar; „Bewegung reduzieren“.
- Sprache DE/EN; Strings zentral (`ui/i18n`), keine fest einkodierten Texte.

---

## 9. Design-Prozess

| Schritt | Ergebnis | Phase in PLAN.md |
|---|---|---|
| 1. Moodboard | Referenzen (Tricorder-Ära, Messgeräte, Apple HIG); Abgrenzung, was *nicht* kopiert wird | 0 |
| 2. Tokens & Typo-Test | `tokens.h`, Fonts gerendert, **auf dem echten Display** bei Sonne und Nacht geprüft | 1 |
| 3. Mockups 1:1 (480×222) | HTML- oder Figma-Mockups der 7 Schlüssel-Screens; Review | 1 |
| 4. Komponenten in LVGL | Komponenten-Galerie-Screen (versteckt im Dev-Menü) | 1–3 |
| 5. Screens | Umsetzung nach Wireframes, Iteration im **LVGL-SDL-Simulator** am Mac | 3+ |
| 6. Usability-Test | 5 Aufgaben (z. B. „Kanal anlegen“, „Repeater-Stats abrufen“) mit 3–5 Testern, Zeit und Fehler messen | 3, 5, 8 |
| 7. Screenshot-Regression | Simulator rendert alle Screens → Vergleich in CI | ab 4 |

**Definition of Done pro Screen:** nur mit dem Encoder bedienbar · über die Palette auffindbar · Tag-, Nacht- und Rotlicht-Modus geprüft · Große Schrift geprüft · DE/EN · Screenshot in der Regression.
