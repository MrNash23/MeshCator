# MeshCator: Prüfung aus Nutzersicht

**Nachfolgestand:** [0.3.0-dev](FEATURES-0.3.md) implementiert inzwischen die wesentlichen unten genannten Lücken. Diese Tabelle dokumentiert den Audit vor diesem Ausbau.

Stand: 30. Oktober 2026, 0.2.0-dev. **Urteil: Entwicklungsstand, noch keine
alltagstauglich abgenommene Veröffentlichung und keine Funktionsgleichheit mit
Ripple oder MeshOS.** Der Grundablauf ist nachvollziehbar, wichtige Funktionen
sind jedoch unvollständig. Ein erfolgreicher Build ersetzt keinen Funk-,
Bedienungs- oder Dauertest am Gerät.

Geprüft wurden Implementierung, automatisierte Kern- und Renderprüfungen,
native Simulatorbilder und die unten verlinkten Referenzen. Der Pager ist
abgesteckt. Es wurde in diesem Durchgang nichts auf ihn geschrieben.

## In diesem Durchgang verbessert

- Globale Benachrichtigungen unter Settings → Notifications: alle stumm,
  Standardausgabe, Lautstärke und Display-Aufwecken. Standard ist Vibration,
  Lautstärke 40 %, Aufwecken an. Keine Sprachsynthese: Audio bedeutet Hinweiston.
- Je Chat oder Channel: Radklick → Notifications → Use default / Silent /
  Sound / Vibration / Sound + vibration. Maximal 32 gespeicherte Ausnahmen.
  Die globale Stummschaltung übersteuert auch eine explizite Ton-Ausnahme.
  Stumme Nachrichten erscheinen weiterhin im Verlauf und erhöhen Ungelesen.
  `[M]` kennzeichnet stumme Gespräche in der Übersicht.
- Regeln für Direktnachrichten hängen am Kontakt-Schlüssel; Kanalregeln an
  einem SHA-256-Fingerabdruck des Kanalschlüssels, nicht am wiederverwendbaren
  Kanalplatz. Zwei lokale Einträge mit demselben Schlüssel teilen deshalb die
  Alarmregel. Der Fingerabdruck ersetzt keine Geheimhaltung schwacher Schlüssel.
- Ausgaben laufen zentral über die neue UI; alte, von der Companion-Verbindung
  abhängige Audio-Hooks sind für MeshCator abgeschaltet. Start erfolgt ohne
  Begrüßungston. Mehrere Eingänge werden auf höchstens einen Alarm pro drei
  Sekunden begrenzt. Das Display kann dennoch aufgeweckt werden.
- Im geöffneten Chat bei eingeschaltetem Display kein zusätzlicher Alarm;
  bei Menü darüber oder ausgeschaltetem Display zählt der Eingang als ungelesen.
  Änderungen werden erst nach erfolgreichem Speichern im UI übernommen.
- Die direkte Live-Nachrichtenübergabe umgeht die alte Begrenzung auf 119 Bytes.
  Die alte SD-Verlaufsschicht bleibt davon getrennt und ist weiterhin ein offener
  Punkt; sie ist kein verlässlich wiederhergestellter MeshCator-Verlauf.
- Eigener Kartenatlas: 341 komprimierte Kacheln, Landflächen und Seen aus
  Natural Earth 1:50 Millionen, zur Laufzeit in unseren Farben. Keine kopierten
  Kartenbilder. Einzelheiten und Grenzen: [MAPS.md](MAPS.md).

Diese Benachrichtigungsregeln gelten für den **Pager**. Die Smartphone-App
verwaltet ihre eigenen Benachrichtigungen. Eine lokale Stummschaltung wird
nicht als App-Synchronisierung ausgegeben. Vibration und Ton sind kompiliert
angebunden, aber noch nicht am angeschlossenen Gerät gehört bzw. gefühlt.

## Funktionsabgleich

„Implementiert“ beschreibt lokalen Code und Simulator, nicht eine vollständige
Geräteabnahme. Protokollfunktionen zählen nicht automatisch als sichtbare UI.

| Nutzeraufgabe | MeshCator heute | Bewertung / nächste Arbeit |
|---|---|---|
| Ersteinrichtung, Sprache, Funkprofil | Wizard, vier Sprachen, TX-Sperre vor Bestätigung | Implementiert; Abbruch, Neustart und ungültige Gerätekonfiguration am Gerät prüfen |
| Startpunkt und Navigation | Dashboard, Rad-Auswahl, Klick, Seiten-Zurück, langes Back nach Hause | Grundkonzept vorhanden; physische Tasten, Wake-Verhalten und lange Listen abnehmen |
| Direktnachrichten und Channels | Senden, Lesen, getrennte Filteroption | Filter ist in Settings versteckt; schneller Wechsel gehört in die Gesprächsübersicht |
| Erkennen, ob eine DM angekommen ist | UI zeigt Queued/Failed, sendMessage liefert ACK-Daten | **Freigabeblocker:** ACK, Timeout und erneutes Senden fehlen im sichtbaren Ablauf |
| Channel-Zustellung | Senden vorhanden | Keine Empfänger-ACKs versprechen; lokale Übergabe ist keine Zustellgarantie |
| Verlauf nach Neustart | UI hält höchstens 96 Nachrichten im RAM | **Freigabeblocker:** kein vollständiger persistenter Verlauf; alte DM-SD-Schicht begrenzt Text und wird nicht in neuen UI-Verlauf geladen |
| Entwürfe | Beim Wechsel innerhalb der Sitzung erhalten | Persistenz über Neustart fehlt |
| Globale / individuelle Alarme | Stumm, Ton, Vibration, beides, Master-Mute | Neu implementiert und logisch getestet; Hardware- und Verbindungsszenarien offen |
| Private / öffentliche Kanäle | Hashtag und 128-Bit-Hex-Schlüssel, Zufallsschlüssel | Erstellen vorhanden; Bearbeiten, Löschen, Teilen/Import, 256-Bit/Base64-Eingabe fehlen im UI |
| Kontakte | Liste, Details, Favorit, Pfad zurücksetzen, Chat starten | Entdecken/gezielt übernehmen, Import/Export, Löschen und Teilen unvollständig |
| Karten / Radar | Offline-Atlas, eigene SD-Kacheln, Kontakte, Vollbild | Länder weltweit, Regionen Deutschland/Nachbarn ergänzt; keine Straßen; Landmaske >4 nur vergrößert |
| Ziele merken | 16 benannte Orte, GPS-Prüfung, Luftlinie und Peilung | Löschen/Umbenennen fehlt; kein Routenplaner oder Track-Recorder |
| Netzwerkregionen / Scopes | Funkprofile und manuelle Funkparameter | Nicht mit MeshCore-Scopes verwechseln: deren Verwaltung fehlt im UI |
| Repeater / Sensor / Room | Basis-Companion und optionales Client-Forwarding | Keine vollständige Remote-Admin-, Telemetrie- oder Room-Oberfläche; kein Repeater-Modus |
| USB-Wartung | MeshCore-Companion-Protokoll vorhanden | Keine vollständige Wartungsoberfläche für alle MeshCator-Einstellungen, Verlauf, SD und Restore |
| Bluetooth-App | NUS-Transport kompiliert, Schalter/Name/PIN | **Freigabeblocker:** Pairing, Nachrichten, Konfigurationsabgleich, Wiederverbindung real prüfen |
| Emojis | UTF-8-Eingabe/Transport im Core | Schrift enthält keine Emoji-Glyphen; Anzeige und Picker fehlen weiterhin |
| Betrieb / Aktualisierung | Sicherungs- und Migrationswerkzeuge vorbereitet | **Freigabeblocker:** vollständige Sicherung und sichere Migration aller bestehenden Installationen ausstehend |
| Laufzeit und Ressourcen | Build passt in geplanten 0x240000-Slot | Nur etwa 6,7 % Flash-Reserve; PSRAM/Heap, Akku und Lastbetrieb noch messen |

## Referenzbasis und Einordnung

- [MeshCore Companion Protocol](https://github.com/meshcore-dev/MeshCore/blob/main/docs/companion_protocol.md):
  Referenz für Kontakte, Kanalverwaltung, Nachrichten und Gerätekonfiguration.
  Das Vorhandensein des Transportes beweist weder App-Kompatibilität noch
  vollständige Bedienbarkeit in unserer Oberfläche. Die tatsächlich verwendete
  Port-Version ist in THIRD_PARTY.md dokumentiert.
- [Ripple User Guide](https://files.liamcottle.net/MeshCore/Documentation/Ripple_User_Guide.pdf):
  Als Maßstab für eigenständige Gerätebedienung dienen insbesondere Wizard,
  Kontaktaktionen, DM-Wiederholungen, Kanalverwaltung, Karten und Scopes.
  Der Guide unterscheidet kostenpflichtige Funktionen; vollständige Ripple-Parität
  ist keine bereits erfüllte Anforderung unseres Builds. Die dokumentierten
  Retry-/Pfad-Abläufe zeigen besonders deutlich die fehlende Rückmeldung beim Senden.
- [MeshOS Firmware v1.7.9](https://github.com/andymux/meshos-releases/releases/tag/fw-v1.7.9):
  Persistenter Sound-Schalter, stiller Start und verständliche Fehler-/Retry-Abläufe
  sind sinnvolle Qualitätsmaßstäbe. Diese Firmware-Veröffentlichung betrifft
  T-Deck und ThinkNode M9. Funktionen der getrennten Android/macOS-App werden
  hier ausdrücklich nicht pauschal als Firmware-Funktionen gezählt.
- [Natural Earth Nutzungsbedingungen](https://www.naturalearthdata.com/about/terms-of-use/):
  Der Anbieter erklärt die Geodaten als Public Domain und erlaubt eigene Gestaltung
  und kommerzielle Nutzung. Unsere Quellen, Prüfsummen und eigene Erzeugung sind
  dokumentiert. Das ist keine Garantie für die Genauigkeit der Karte.

## Verständliches Bedienkonzept: vorhanden und noch zu verbessern

Der feste Kern bleibt: Rad drehen wählt, Radklick bestätigt oder öffnet das
kontextuelle Menü, Enter bestätigt und sendet ausschließlich im Chat. Die
Seitentaste führt eine Ebene zurück. Die Tastatur-Löschtaste löscht in einem
Textfeld; Halten über zwei Sekunden führt zum Dashboard. Rad halten öffnet die
Hauptnavigation. In der Karte bedeutet Drehen Zoom, im Chat Verlauf blättern.
Diese beiden Ausnahmen müssen in der Hilfe und dem jeweiligen Hinweis sichtbar sein.

Noch keine Abnahme als „intuitiv“: Filter und Kanalerstellung sind zu weit
unter Settings verborgen. Manche Menüs bieten nur einen Teil der erwarteten
Kontakt-/Chataktionen. Die aktive globale Stummschaltung sollte zusätzlich
prominent auf dem Dashboard sichtbar werden. Die Hilfe muss nach Ausbau der
Kontakt- und Kanalverwaltung erneut anhand echter Nutzeraufgaben geprüft werden.

## Reihenfolge bis zur Freigabe

1. Verlässliches Messaging: ACK/Timeout/Retry für DMs, vollständige Texte,
   wiederhergestellter Verlauf und Entwürfe, Speicherfehler sichtbar behandeln.
2. Kanal- und Kontaktverwaltung vervollständigen; Filter direkt in Gespräche.
   Emoji-Unterstützung ausdrücklich abschließen oder als nicht verfügbar kennzeichnen.
3. Einstellungen über Gerät und Companion abgleichen, atomare Fehlerbehandlung
   zwischen NVS, MeshCore-Prefs und BLE verbessern; USB-Wartungsumfang definieren.
4. Pager wieder anschließen: vollständige Flash-Sicherung verifizieren, geplante
   Migration prüfen, alle vorhandenen Firmwares und Daten erhalten, danach flashen.
5. Abnahme mit zweitem Funkgerät und Smartphone: RX/TX, DM-ACK, öffentliche/private
   Channels, BLE verbunden/getrennt, USB verbunden/getrennt, Display an/aus,
   global stumm und Ausnahmen, Sound/Vibration, Neustart, volle Speicher und
   fehlende SD. Auch Funklast während Kartenzoom, Tastenbedienung und Akkulaufzeit.

## Nachweise dieses Durchgangs

- `cmake --build build/simulator -j8` erfolgreich.
- `ctest --test-dir build/simulator --output-on-failure`: 14/14 bestanden.
- Neue Tests: Master-Mute, Chat-/Channel-Ausnahmen, Kanalidentität bei Slotwechsel,
  Speicherausfall, Legacy-Einstellungen, defekte Alarm-Erweiterung, Display aus,
  lange UTF-8-Nachricht; alle 341 Kacheln entpackt, Land-/Meer-/See-/Datumsgrenzenproben.
- Native Ansichten für Karte, Vollbild und Benachrichtigungen gerendert und visuell
  geprüft. Render-Smoke-Tests prüfen Absturzfreiheit, nicht vollständige Ergonomie.
- `pio run -e meshcator-launcher` erfolgreich. Exakte Größe und SHA-256:
  [current-build.json](implementation/current-build.json).
- Nicht geprüft: reale Alarmhardware, GPS/Funk, BLE-Pairing, Langzeitverhalten,
  Migration und Startfähigkeit der erhaltenen Fremdinstallationen.

Nachtrag: Länder-/Regionsgrenzen und lokalisierte Namen nach Nutzerwunsch ergänzt. Regionaler Umfang und Genauigkeitsgrenzen siehe MAPS.md. Hardwareabnahme weiterhin offen.
