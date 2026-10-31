# Ungelesen-Badges – 0.3.13-dev

Am 31.10.2026 installiert. Daten und vorheriger App-Slot gesichert; Firmware und Erhalt aller anderen Partitionen verifiziert. USB antwortet nach Neustart.

Hauptmenü und Dashboard zeigen rechts oben auf Gespräche die Summe ungelesener Kanalnachrichten, auf Kontakte die Summe ungelesener Direktnachrichten. Bei null bleibt das Badge verborgen. Es zählt Nachrichten, nicht Gespräche. Stummschaltung unterdrückt weiterhin nur den Alarm, nicht die Ungelesen-Anzeige. Der Gesprächsfilter beeinflusst die globalen Zähler nicht.

Badge: schwarzer Kreis, 30 Pixel Durchmesser; Zahl in der jeweiligen Kachelfarbe. Werte über 99 erscheinen als 99+. Sättigende Summierung verhindert einen Überlauf durch große gespeicherte Zähler. Öffnen des Chats setzt dessen vorhandenen Ungelesen-Zähler zurück; die Badges werden daraus neu abgeleitet und benötigen keinen eigenen Speicherzustand.

24/24 Tests: getrennte Summen, mehrere Nachrichten im selben Kanal, stumme Nachrichten, Lesen/Öffnen, eingehende Nachricht bei geöffnetem Menü, Archiv-Wiederherstellung, 99/100 und Überlaufgrenze. Darstellung in LVGL visuell geprüft.

Ab 0.3.15 zeigt auch jede Gesprächskachel rechts oben den gleichen Kreis mit ihrem eigenen Ungelesen-Zähler. Die kleine Zahl unten rechts entfällt; dort bleibt die Kennzeichnung Kanal/Direkt. Namen reservieren bei sichtbarem Badge dessen Platz und bleiben vollständig umgebrochen. Der Layout-Test prüft lange Namen mit 0, 1 und 100 ungelesenen Nachrichten in allen vier Sprachen.
