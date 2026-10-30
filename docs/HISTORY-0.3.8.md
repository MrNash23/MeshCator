# Nachrichtenverlauf 0.3.8

Am 30.10.2026 auf dem ursprünglichen MeshCator-Pager installiert. Einstellungen, Verlauf und Katalog zusätzlich über USB gesichert; vollständiger vorheriger App-Slot verifiziert gesichert. Firmware-Prüfsumme und unveränderte übrige Partitionen geprüft, USB-Wartung antwortet nach Neustart.

- 512 Nachrichten insgesamt im direkt verfügbaren Verlauf (DM und Channels, TX und RX gemeinsam). Älteste Einträge fallen bei Überschreitung heraus. Bis zu 64 Gespräche bleiben unterstützt.
- Internes Archiv weiterhin mit Prüfsumme, temporärer Datei und Backup; vorhandene Archive mit 96 Nachrichten bleiben lesbar. Größenlimit für Wiederherstellung und USB-Import jetzt 256 KiB.
- Mit gemounteter SD-Karte wird jedes neue Sendeereignis und jede neue empfangene Nachricht an `/meshcator/messages.tsv` angehängt. Kein erneutes Schreiben des gesamten SD-Verlaufs. Ohne Karte bleibt der interne Verlauf verfügbar. Schreibfehler bei gemounteter Karte erscheinen als Meldung; es gibt keine nachträgliche Synchronisation ausgefallener SD-Schreibvorgänge.
- SD-Protokoll: UTF-8, sieben durch Tabs getrennte Spalten: Unix-Zeit (UTC, 0 unbekannt), TX/RX, direct/channel, Gesprächs-ID, Gesprächsname, initialer Delivery-Status (0 Received, 1 Queued, 2 Failed, 3 Simulated, 4 Delivered, 5 Unconfirmed), Text. Backslash, Tab, CR und LF werden als `\\`, `\t`, `\r`, `\n` maskiert. Channel-Schlüssel werden nicht exportiert. Inhalt ist Klartext, einschließlich privater Nachrichten.
- ACK-Änderungen und Wiederholungsversuche ändern den initialen Protokolleintrag nicht. Das Protokoll ist ein Eingangs-/Sendeereignisarchiv, kein später aktualisierter Zustellbericht. Bestehende Nachrichten werden beim Update nicht rückwirkend exportiert.
- Das Archiv wächst bis zur verfügbaren SD-Kapazität. Löschen eines Verlaufs oder Channels auf dem Gerät löscht diese separate Archivdatei nicht. Datei bei Bedarf am Computer sichern oder löschen. Bei Stromverlust während eines Schreibvorgangs kann der letzte Datensatz unvollständig sein.
- Die 512 aktuellen Nachrichten lassen sich wie bisher mit dem Rad durchsehen. Ältere SD-Einträge werden in dieser Version nicht in die Pager-Chatansicht nachgeladen; sie können am Computer gelesen werden.

Export über USB:

```sh
python tools/meshcator_usb.py --port /dev/cu.usbmodem11401 get sd:messages.tsv messages.tsv
```

Verifikation: Tests für 520 empfangene Nachrichten, Begrenzung auf die neuesten 512, Wiederherstellung eines Archivs >64 KB, Archivierung vor Ablauf der Verlaufseinträge, SD-Schreibfehler ohne Verlust des internen Eintrags, UTF-8/Emoji/Tab/Zeilenumbruch-Maskierung und Dateianhang im Simulator. Hardware-SD-Test steht aus.
