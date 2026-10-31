# MeshCator 0.3.0-dev — neue Funktionen

Implementierungsstand, noch nicht auf dem Pager installiert. Der vollständige
Build und die automatisierten Tests ersetzen weder Funk-Gegenstellentests noch
die Abnahme mit Bluetooth-App und realem Dateisystem.

## Nachrichten

- Direkte Nachrichten: Rückmeldung „zugestellt“ nach passendem MeshCore-ACK.
  Die vom Funkstack gelieferte Wartezeit führt ohne ACK zu „nicht bestätigt“.
  Das bedeutet nicht automatisch, dass die Nachricht nicht angekommen ist.
- Chatmenü: letzte fehlgeschlagene/unbestätigte Nachricht erneut senden oder
  Pfad zurücksetzen und per Flood wiederholen. Kein automatisches Dauersenden.
- Channels liefern keine Empfänger-ACKs. Ihre lokale Funkübergabe wird nicht als
  Zustellung ausgegeben. Der Simulator kennzeichnet Funkaktionen als Simulation.
- Die letzten 96 Nachrichten insgesamt, Gesprächsnamen, Ungelesen-Zähler und
  Entwürfe werden intern in der dedizierten MeshCator-Dateisystempartition
  gespeichert. Es ist keine SD-Karte erforderlich.
- Prüfsumme, temporäre Datei und Rückfallkopie schützen gegen unvollständige
  Schreibvorgänge. Beschädigte Daten werden nicht ungeprüft übernommen.
- Entwürfe werden nach einer Sekunde Eingabepause gespeichert. Bei abruptem
  Stromverlust davor können die zuletzt getippten Zeichen fehlen. Nachrichten-
  und Zustelländerungen werden unmittelbar gespeichert. Speicherfehler werden
  angezeigt, nicht als erfolgreiches Speichern ausgegeben.
- Nach Neustart wird ein zuvor offener Sendeversuch als unbestätigt geladen;
  ohne erhaltenes ACK wird keine Zustellung erfunden.
- Bei geänderter Kanalbelegung/anderem Schlüssel bleibt der alte Verlauf als
  schreibgeschütztes Archiv bestehen. So landen Nachrichten eines alten privaten
  Kanals nicht versehentlich im neuen Kanal desselben Slots.

## Gespräche, Kanäle und Kontakte

Die Gesprächsübersicht hat eine zusätzliche Karte „Verwalten“. Dort liegen
Alle / Channels / Direktnachrichten, Kanal erstellen und Importieren.

Radklick im Chat bzw. auf einem Kontakt öffnet das Kontextmenü:

- Kanal: Namen/Schlüssel bearbeiten, Export/Teilen, löschen, Alarme einstellen.
  Private Schlüssel können als 128/256-Bit-Hex oder Base64 eingegeben werden;
  intern und beim Export wird Hex verwendet. Für neue private Kanäle wird ein
  zufälliger 128-Bit-Schlüssel vorgeschlagen.
- Kontakt: Umbenennen, Favorit, Pfad zurücksetzen, signierten Kontaktadvert per
  Funk teilen, exportieren/importieren, löschen und Remote-Administration.
- Verlauf löschen ist getrennt vom Löschen eines Kontaktes oder Kanals.
  Löschen startet immer mit „Abbrechen“ als Auswahl und benennt den Eintrag.
- Entfernen eines Kanals verschiebt keine anderen Kanalslots.
- Export zeigt den vollständigen Text in blätterbaren Seiten. Bei privaten
  Kanälen ist der Schlüssel bewusst sichtbar. Import ist per Tastatur oder
  komfortabler über das USB-Werkzeug möglich.

Austauschformate:

- `MC-CHANNEL1 "Name" HEXKEY`: MeshCator-Kanalexport. Kein behaupteter Standard-QR-Link.
- `MC-CONTACT1 HEXADVERTISEMENT`: signiertes MeshCore-Advertisement als Hex.
- `MC-NODE1 KEY "Name" TYPE FLAGS LAT LON`: Rückfall für Kontakte ohne gespeicherten
  Advert. Dies sind manuell übernommene Kontaktdaten, **kein signierter Identitätsnachweis**.

Die nativen MeshCore-Companion-Kommandos zum Austausch bleiben erhalten.

## Emojis

Chatmenü → Emoji einfügen zeigt 160 häufige Emojis in einem Raster mit
32 Zeichen pro Seite. Das Rad wählt über alle Seiten hinweg, Klick/Enter fügt
ein. Insgesamt 358 monochrome Noto-Emoji-Zeichen stehen in allen Textschriften
für Chat-, Kanal- und Node-Namen sowie Nachrichten zur Verfügung (OFL-1.1).

Variationsselektoren, Hauttöne und ZWJ erzeugen keine zusätzlichen Kästchen.
Hauttöne bleiben monochrom; ZWJ-Kombinationen erscheinen als einzelne
Basiszeichen. Länderflaggen werden nur zur Anzeige in Codes wie [DE]
umgewandelt. Gespeicherter und gesendeter Unicode-Text bleibt unverändert.
Löschen entfernt Emoji mit Selektor/Hautfarbe bzw. ZWJ-Ketten zusammen.
Das Funklimit bleibt 150 UTF-8-Bytes. Nicht enthaltene Zeichen können
weiterhin als Kästchen erscheinen; kein vollständiger Unicode-Emoji-Satz.

Einstellungen → Info / Credits enthält Autoren, Frameworks, Bibliotheken,
Lizenzen und Quellcode-Adresse. Originaltexte liegen im Repository in LICENSES/.

## Remote-Administration

Kontaktmenü → Remote-Administration:

1. Mit dem Passwort des Zielgerätes anmelden.
2. Geräteinfo oder Status abfragen.
3. Einen vom Zielgerät unterstützten MeshCore-CLI-Befehl eingeben.

Passwörter werden maskiert und weder im Verlauf noch in Einstellungen gespeichert.
Antworten sind blätterbar, fehlende Antworten werden nach 30 Sekunden kenntlich
 gemacht. Rückmeldungen werden dem angefragten Kontakt zugeordnet. Berechtigungen
und unterstützte Befehle bestimmt die Gegenstelle. Sensor-/Room-spezifische
Assistenten und eine grafische Oberfläche für sämtliche Remote-Parameter sind
nicht Teil dieses Updates. Die Befehlseingabe ersetzt diese Spezialansichten.

## USB-Wartung

Werkzeug: `tools/meshcator_usb.py`, Python 3 und pyserial.
Ein privates, versioniertes Wartungsprotokoll ergänzt das vorhandene
MeshCore-Companion-Protokoll. Es wird ausschließlich über USB angenommen;
Antworten werden nur an die anfragende USB-Schnittstelle geschickt, nicht
über den normalen Broadcast an Bluetooth. Keine unverschlüsselte Funk-Wartung.

Beispiele (PORT durch den tatsächlichen `/dev/cu.…`-Port ersetzen):

```sh
python tools/meshcator_usb.py --port PORT info
python tools/meshcator_usb.py --port PORT set language 3
python tools/meshcator_usb.py --port PORT set bluetooth 1
python tools/meshcator_usb.py --port PORT set name "MeshCator Nord"
python tools/meshcator_usb.py --port PORT set mute 1
python tools/meshcator_usb.py --port PORT backup ./meshcator-backup
python tools/meshcator_usb.py --port PORT restore ./meshcator-backup
python tools/meshcator_usb.py --port PORT get settings ./settings.txt
python tools/meshcator_usb.py --port PORT put settings ./settings.txt
python tools/meshcator_usb.py --port PORT import ./channels-and-contacts.txt
python tools/meshcator_usb.py --port PORT put sd:tiles/4/8/5.bmp ./tile.bmp
python tools/meshcator_usb.py --port PORT get sd:tiles/4/8/5.bmp ./tile-copy.bmp
```

`set` unterstützt: theme, language, setup, frequency, bandwidth, sf, cr, power,
hash_bytes, forwarding, gps, share_location, bluetooth, brightness, timeout,
map_source, show_contacts, name, bluetooth_name, pin, mute, alert_mode, volume, wake.
Boolesche Werte: 0/1. Sprache: 0 Englisch, 1 Französisch, 2 Spanisch, 3 Deutsch.
Alarmmodus: 0 stumm, 1 Ton, 2 Vibration, 3 beides. Weitere strukturierte Felder
(Orte, individuelle Alarmregeln) sind im vollständigen Settings-Export enthalten
und können über validierten Import geändert werden.

Das Werkzeug überschreibt keine bestehenden lokalen Exportdateien. Sicherungen
werden mit restriktiven Dateirechten erstellt. Sie enthalten Kanalgeheimnisse
und private Gesprächsinhalte im Klartext. Wiederherstellung des Katalogs erfolgt
additiv; bestehende abweichende gleichnamige Kanäle führen zu einem Fehler statt
zu einem stillen Überschreiben. Einzelne bereits importierte Einträge bleiben
bei einem späteren Fehler erhalten. Das Werkzeug meldet den Fehler.

History-Restore prüft Integrität und Format und bricht bei zwischenzeitlich
verändertem Verlauf ab. SD-Uploads schreiben temporär und behalten eine
`.bak`-Datei bei Ersatz. Dateizugriff ist auf `/meshcator/` auf SD begrenzt,
Uploads auf 4 MiB, Konfigurations-/History-Uploads auf 64.000 Bytes.

Diese Anwendungssicherung enthält **nicht den privaten Radio-Identitätsschlüssel
und nicht die anderen Firmware-Installationen**. Für die vollständige Geräte-
Sicherung und Wiederherstellung dienen weiterhin `tools/backup_launcher.py`
und die dokumentierten Flash-/Partitionswerkzeuge. Es gibt kein ungeprüftes
„gesamten Flash überschreiben“ im neuen USB-Werkzeug.

## Verifikation und Grenzen

Automatisierte Tests prüfen Kernabläufe, ACK/Timeout/Retry, Archiv-/Entwurfs-
Roundtrip, defekte Dateien, Rückfallkopie, Löschbestätigung, Emoji-Einfügen,
Remote-Timeout, Kanalidentitätswechsel und Trennung des USB-Wartungskanals von
Bluetooth. Dazu kommen USB-Client-Chunking/Prüfsumme, Kartenprüfung und native
Render-Smoke-Tests. Exakte Builddaten: `implementation/current-build.json`.

Offen bleibt die Geräteabnahme: reale ACKs, erneutes Senden, Stromunterbrechung
beim Speichern, SD-Tausch/volle Datenträger, Bluetooth-App-Verhalten, Remote-
Gegenstellen, USB-Dateitransfers und Dauerlast. Nicht behauptet werden vollständige
Ripple-/MeshOS-Parität, beliebige Unicode-Emojis oder eine bereits getestete
Migration des Mehrfachinstallations-Pagers.

## Kontakt-/Repeater-Ergänzung 0.3.1

Siehe [Funktionsumfang und Geräteabnahme](NODE-FUNCTIONS-0.3.1.md). Die dort ergänzten Kontaktfunktionen ersetzen die entsprechenden Einschränkungen dieses früheren Stands.

### Eingabe-Haptik (0.3.17)

Menüauswahl per Drehrad erzeugt einen kurzen DRV2605-Tick; Radklick und Enter erzeugen einen kurzen Bestätigungsimpuls. Kein Tick beim Kartenzoom oder Scrollen des Chatverlaufs. Ohne blockierende Wartezeit, bei schnellen Menüwechseln auf einen Tick je 40 ms begrenzt. Eine laufende Nachrichtenvibration hat Vorrang. Bestätigung quittiert die Eingabe, nicht die Zustellung einer Nachricht. Auf Hardware noch zu prüfen.

## 0.3.20: Ungelesene Nachrichten und Absender

Chats und Kanäle öffnen bei der ältesten gespeicherten ungelesenen Nachricht. Das lokalisierte schwarze Neu/New/Nouv./Nuevo-Badge bleibt bei der ersten Ansicht stehen. Beim Weiterblättern oder Verlassen/Überdecken der sichtbaren Nachrichtenansicht wird nur diese Nachricht gelesen markiert. Nicht besuchte Nachrichten bleiben ungelesen; neue Eingänge verschieben beim Lesen älterer Nachrichten nicht den betrachteten Eintrag. Ist keine Nachricht ungelesen, öffnet der Chat bei der neuesten.

Lesestatus wird pro Nachricht im Archivformat MCH2 gespeichert. MCH1 wird übernommen: die bisherigen Chat-Zähler werden den neuesten eingegangenen Nachrichten zugeordnet (eigene Nachrichten werden übersprungen). Alte Firmware kann MCH2 nicht lesen; für einen Rückwechsel die vor dem Update gesicherte Historie verwenden.

Absender stehen in einer dunklen Namensfläche, der eigentliche Text separat darunter; Datum/Uhrzeit weiterhin oben rechts. Bei Kanälen wird das MeshCore-Präfix `Name: Text` für die Darstellung getrennt, der Originaltext bleibt gespeichert. Ohne Präfix wird kein Absender erfunden. Lange Texte verwenden bei Bedarf eine kleinere Schrift, damit sie die Statuszeile nicht überlagern.

## 0.3.22: Back/Del schneller

Back/Del löst Zurück nach 800 ms statt mehr als 2 Sekunden aus. Kurzes Drücken löscht weiterhin erst beim Loslassen; langes Drücken löscht keinen Text und löst nur einmal aus. Die Einführung ist in allen vier Sprachen angepasst.

## 0.3.25: Deutlicheres haptisches Feedback

Menüwechsel verwendet DRV2605-Effekt 17 (Strong Click 1, 100 %), Bestätigen Effekt 10 (Double Click, 100 %). Motorspannung und Begrenzung bleiben unverändert. Der Nutzer hat die Motorfunktion mit 0.3.24 bestätigt; die Wirkung der neuen Effekte muss am Gerät beurteilt werden.
