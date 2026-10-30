# MeshCator 0.3.1-dev: Kontakte und Repeater

Implementiert und am Desktop geprüft. Auf dem Pager nach verifizierter 16-MB-Vollsicherung installiert; FT8 wurde unter Erhalt seines gesamten belegten Bereichs verschoben. Alle übrigen Firmware- und Datenpartitionen wurden bei der Migration auf unveränderten Inhalt geprüft. Bootauswahl auf MeshCator umgestellt, USB-Wartungsantwort bestätigt. Die funktionale Geräteabnahme steht noch aus.

## Bedienung

Kontakte → Eintrag auswählen → Rad klicken. Bei Companions steht das Gespräch vorn, bei Repeatern, Room-Servern und Sensoren die Detailansicht. Nur Repeater und Room-Server bieten Remote-Administration an. Rad wählt, Klick/Enter bestätigt; Zurück schließt. In mehrseitigen Details und Antworten blättert das Rad. Lange Zurück-Betätigung führt weiterhin zur Hauptübersicht.

- **Details:** Name, Typ, vollständiger öffentlicher Schlüssel, Ausgangspfad, Hops, Advert-Zeit in UTC, vorhandene Koordinaten und Entfernung bei eigenem GPS-Fix. Advert-Zeit stammt von der Uhr des Absenders; zukünftige Zeiten erzeugen keine negativen Altersangaben.
- **Pfad bearbeiten:** `1:aabb` bedeutet zwei Hops mit je einem Hash-Byte, `2:aabbccdd` zwei Hops mit je zwei Bytes; `3:` unterstützt drei Bytes pro Hop. Ohne Präfix gilt ein Byte. Leer bedeutet direkte Zustellung ohne Repeater. Ungültige Hexwerte, unvollständige Hashes und zu lange Pfade werden abgewiesen. „Pfad zurücksetzen“ verwendet wieder Flood.
- **Ping (Zero Hop):** Basistelemetrie-Anfrage ohne Hinweg-Repeater. Gespeicherter Pfad wird anschließend wiederhergestellt. Erfolg erst nach passender Antwort; sonst Zeitüberschreitung. Gegenstellen mit verweigerter Basistelemetrie können auch in Reichweite stumm bleiben.
- **Pfad verfolgen:** Hin- und Rückweg über den bekannten Pfad; bei Repeatern einschließlich des Zielrepeaters. Ausgabe der Hop-Hashes und SNR-Werte. Bei Companions wird der Repeaterpfad geprüft, nicht das letzte Funksegment zum Companion. Unbekannte, zu lange und drei Byte breite Pfade werden abgewiesen: das verwendete Traceformat kodiert Hashbreiten als Zweierpotenzen. Keine automatische Routensuche.
- **Telemetrie abfragen:** Batterie/Spannung, Temperatur, Feuchte, Luftdruck und GPS aus Cayenne-LPP-Antworten. Welche Werte tatsächlich kommen, bestimmt die Gegenstelle mit ihren Sensoren und Freigaben. Unbekannte Typen erscheinen mit verbleibenden Rohdaten statt erfundenen Messwerten.
- **Telemetrie freigeben:** Basis, Position und Umweltsensoren je Kontakt. Das betrifft Zugriff auf die eigene Telemetrie, keine Administratorrechte auf dem entfernten Repeater. `ALL` / `OFF` kennzeichnen globale Regeln, die die Kontaktflags übersteuern. „Kontaktregeln aktivieren“ schaltet nach separater Bestätigung alle drei globalen Bereiche auf Kontaktflags um; diese Änderung betrifft alle Kontakte. Der Favoritenstatus bleibt erhalten.
- **Auf Karte ansehen:** erscheint bei bekannter Position und zentriert die Karte auf den Kontakt. Die Karte bleibt zoomfähig.
- **Kontakt als QR-Code:** öffentlicher Name, Schlüssel und Typ als MeshCore-Kontakt-URL; keine privaten Schlüssel. QR-Bild wurde mit dem macOS-Decoder erfolgreich gelesen. Das Scannen mit der MeshCore-Handy-App bleibt Teil der Geräteabnahme.
- **Remote-Administration:** Login, Geräteinfo, Status, freie Befehle sowie vorbereitete CLI-Befehle zum Lesen von Funkparametern, Sendeleistung, Advert-Intervall und Weiterleitung. Editierbare Vorlagen ändern Name, Sendeleistung, Advert-Intervall und Weiterleitung. Ein vorbereiteter Befehl wird erst nach Enter/Klick gesendet. Antworten und Fehler der Gegenstelle werden angezeigt. Dies ist keine vollständige grafische Nachbildung aller firmwareabhängigen Remote-Einstellungen; zusätzliche Befehle bleiben über das Befehlsfeld zugänglich.
- Umbenennen, Favorit, Kontakt-Advert teilen, Textexport und bestätigtes Entfernen bleiben vorhanden.

Das QR-Format folgt der [MeshCore-Dokumentation](https://github.com/meshcore-dev/MeshCore/blob/main/docs/qr_codes.md). Alle vier Oberflächensprachen sind für die neuen Menüpunkte ergänzt; technische Antworten der Gegenstelle bleiben unverändert.

## Gemeinsame Geräteabnahme

1. Companion und Repeater auswählen; passende Menüs, Details und Zurück/Home prüfen.
2. Favorit und Pfad ändern, neu starten und Persistenz prüfen; anschließend Pfad zurücksetzen.
3. Ping auf nahes Ziel und nicht erreichbares Ziel; Antwort beziehungsweise Timeout prüfen.
4. Trace über bekannten Repeaterpfad, anschließend Telemetrie abrufen.
5. Kontaktposition auf Karte öffnen, zoomen, zurückkehren.
6. QR-Code mit MeshCore-App scannen und Name/Schlüssel/Typ vergleichen.
7. Telemetriefreigaben mit zweiter Gegenstelle prüfen, einschließlich globaler Overrides und Kontaktregeln.
8. Remote-Anmeldung mit falschem und korrektem Passwort, Leseanfrage und bewusst gewählte Änderung prüfen. Abbruch während einer Anfrage darf verspätete Antworten nicht wieder öffnen.

Zusätzlich bleiben Bluetooth-Kopplung, Benachrichtigungshardware, USB-Wartung, Wiederanlauf und der bisherige Gesamtfunktionstest offen. Vor dem Flashen muss die vollständige Sicherung erfolgreich abgeschlossen und verifiziert sein.
