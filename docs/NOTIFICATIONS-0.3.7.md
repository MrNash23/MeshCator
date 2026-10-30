# MeshCator 0.3.7: Entdeckung und Aufwecken

Installiert auf dem ursprünglichen MeshCator-Pager am 30.10.2026; 21/21 Tests bestanden. Der vollständige vorherige App-Slot wurde vor dem Schreiben verifiziert gesichert. Firmware-Prüfsumme und unveränderte übrige Partitionen wurden geprüft; USB-Wartung antwortet nach Neustart. Bericht: `backups/pager-2026-10-30/update-0.3.7-original/update-report.json`.

- BLE unterdrückt wiederholte NEW_ADVERT-Frames (0x8a) mit gleichem vollständigen Public Key und Advert-Zeitstempel für 60 Sekunden. Der Cache umfasst 64 Ereignisse und wird beim Leeren der BLE-Verbindungsbuffers zurückgesetzt. Geänderte Zeitstempel, andere Nodes, Kontaktlisten-Antworten und Nachrichten werden nicht gefiltert. Nur erfolgreich in die BLE-Sendewarteschlange aufgenommene Frames werden gemerkt; volle Warteschlangen verhindern einen späteren Versuch nicht.
- Mehrfache Registrierung derselben Schnittstelle erzeugt keine mehrfachen Ausgaben mehr.
- Wenn eine nicht stummgeschaltete Nachricht das zuvor ausgeschaltete Display aufweckt, öffnet sich ihr Chat bei der neuesten Nachricht. Menüüberlagerungen werden geschlossen, ein bisheriger Nachrichtenentwurf bleibt erhalten. Bei eingeschaltetem Display erfolgt kein automatischer Chatwechsel. Stummschaltung und deaktiviertes Aufwecken bleiben wirksam. Eine ausdrücklich gesperrte Oberfläche bleibt gesperrt.

Tests: doppelte Entdeckung, neue Zeitstempel, unterschiedliche vollständige Schlüssel, ungültige/andere Frames, Cache-Reset, Timerüberlauf, idempotente Schnittstellenregistrierung, DM-/Kanal-Aufwecken, Entwurferhalt, Stumm-/Wake-Einstellungen und Sperransicht. Die Wirkung in der iOS-App und am realen Display muss nach Installation noch geprüft werden.
