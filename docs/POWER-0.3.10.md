# Energieprüfung und USB-Status – 0.3.10-dev

Am 31.10.2026 installiert. Vorherige Einstellungen, Verlauf und Katalog über USB gesichert; vollständiger vorheriger App-Slot verifiziert gesichert. Firmware und unveränderte übrige Partitionen geprüft. USB-Wartung antwortet nach Neustart. Bericht: `backups/pager-2026-10-31/update-0.3.10/update-report.json`.

## Befunde im bisherigen Code

1. PowerManager las und schrieb die XL9555-Eingangsregister 0x00/0x01 statt der Ausgangsregister 0x02/0x03. Seine GPS-Stromschaltung konnte damit nicht wie beabsichtigt arbeiten. Er verwendet jetzt den bereits vorhandenen Board-Helfer (Richtung und Ausgangsregister) für NFC und den GPS-Board-Helfer für GPS-Versorgung und Reset.
2. Der Kommentar und die Abschaltung für NFC enthielten P3. Laut Board-Pinmap ist P3 die LoRa-Versorgung, NFC liegt auf P5. Nur P5 wird abgeschaltet. LoRa bleibt für Nachrichtenempfang aktiv.
3. GPS wurde zum Booten eingeschaltet; bei gespeicherter Einstellung Aus konnte die Initialisierung wegen des bereits false gesetzten Softwarezustands das reale Abschalten überspringen. Die Einstellung wird beim UI-Start jetzt unbedingt angewendet. Änderungen über BLE werden mit dem Model-Abgleich übernommen. Bestehender GPS-Zyklus: 30 Sekunden an, 270 Sekunden aus.
4. Auch mit dunklem Display wurden Model, LVGL-Objekte und Display jede Sekunde bearbeitet. Rendering/LVGL-Handler pausieren jetzt bei Display-Aus; Model-Abgleich alle 10 Sekunden statt jede Sekunde, sofort beim Aufwachen. Radio, BLE, USB, Eingaben, Benachrichtigungen und Archiv-Timer laufen weiter. Der dunkle UI-Loop gibt mit delay(1) CPU-Zeit frei; kein Deep-Sleep.
5. Audio-I2S/DMA lief nach der Initialisierung dauerhaft. Start erfolgt jetzt für Töne, Stop nach Tonende und direkt nach Initialisierung. Verstärker startet ausgeschaltet. Codec-Analogversorgung wird nicht neu programmiert; deren Restverbrauch bleibt zu messen.
6. Dimmen schrieb wiederholt dieselbe Helligkeit. Identische Werte werden übersprungen; eine Eingabe stellt auch aus gedimmtem Zustand die Zielhelligkeit wieder her.

## USB-Anzeige

BQ25896 REG0B liefert Power-Good und Ladezustand. Die Statuszeile zeigt `USB` bei externer Versorgung, `USB+` bei Vorladung/Schnellladung. Ladung beendet oder pausiert bleibt `USB`. OTG-Ausgang wird nicht als externe USB-Versorgung angezeigt; ein I2C-Lesefehler erzeugt keine positive Anzeige. USB-Datenverbindung/Serial-DTR wird nicht als Ladeindikator verwendet.

## Verifikation und offene Messung

22 lokale Tests einschließlich realem PowerManager mit Arduino-Shim: NFC ausschließlich P5, GPS-Aus, GPS-Zyklus, Dimmen ohne Wiederholung, Aufwachen, USB-Versorgung/Ladezustand/OTG/Lesefehler. USB-Ladeanzeige zusätzlich im LVGL-Simulator gerendert.

Keine Strommessung und noch kein Laufzeittest am Pager. Die Codefehler sind nachgewiesen; ihr Anteil an der kurzen Laufzeit ist nicht gemessen. LoRa-Empfang, BLE bei aktivierter Einstellung, MCU, Displaycontroller und weitere Peripherie verbrauchen weiterhin Strom. Display-Aus ist kein Tiefschlaf. Die Prozentanzeige nutzt RemainingCapacity mit selbstkalibrierter Voll-Referenz; Prozentabfall allein beweist keinen defekten Akku.

Nächster Hardwaretest: voll laden, USB abziehen, dieselben GPS-/BLE-/Funk-Einstellungen beibehalten und bei abgeschaltetem Display den Verlauf der Restkapazität bzw. Laufzeit vergleichen. Zusätzlich GPS-Aus, NFC-P5-Aus, weiterhin aktiven LoRa-Empfang, BLE-Verbindung, Benachrichtigungston und Aufwachen überprüfen. Für belastbare Verbrauchswerte am Akku messen; USB-Eingangsstrom während des Ladens ist kein reiner Geräteverbrauch.

## Quellen

- [LilyGO Hardware-Pinmap](https://github.com/Xinyuan-LilyGO/LilyGoLib/blob/master/docs/hardware/lilygo-t-lora-pager.md): P3 LoRa, P4 GPS, P5 NFC, P7 GPS Reset.
- [TI BQ25896 Datenblatt](https://www.ti.com/lit/ds/symlink/bq25896.pdf): REG0B, PG_STAT, CHRG_STAT und OTG.

Die separat aufgefallenen Abweichungen der Keyboard-/SD-Pins zwischen älterem Port und aktueller Herstellerdokumentation werden in diesem Fix nicht verändert; die konkrete Boardrevision muss vor einer Anpassung geprüft werden.
