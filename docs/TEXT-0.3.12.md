# Vollständige Gesprächsnamen – 0.3.12-dev

Am 31.10.2026 installiert. Einstellungen, Verlauf und Katalog sowie vorheriger App-Slot gesichert; Firmware und Erhalt aller übrigen Partitionen verifiziert. USB antwortet nach Neustart.

Die Kürzung von Kanal-/Gesprächsnamen aus 0.3.11 wird vollständig zurückgenommen. In der Übersicht stehen zwei breite Karten untereinander. Der Name wird vollständig umbrochen dargestellt; nur die letzte Nachricht besitzt einen begrenzten Vorschaubereich. Die aktive Karte bleibt orange, andere Karten blau. Status/unread steht separat rechts unten. Drehrad, Auswahl und Gesprächsfilter bleiben unverändert.

Im geöffneten Chat bleibt der vollständige Name über der Nachricht stehen. Bei besonders langen Namen wird die Schrift passend verkleinert (18/16/14/12 Pixel), ohne Zeichen wegzulassen. Der Nachrichtenbereich beginnt darunter. Die Übersicht nutzt 16 Pixel, bei Bedarf 14. Gespeicherte Namen dürfen höchstens 64 UTF-8-Bytes enthalten.

24/24 Tests bestanden, Firmware gebaut. Der neue LVGL-Test `full_name_layout` prüft vollständigen Labeltext, Umbruchmodus und Höhenbegrenzung in Übersicht und geöffnetem Chat für vier Sprachen, einen langen mehrsprachigen Namen sowie 31 und 64 breite W-Zeichen. Er schlägt bei abgeschnittenem Namen oder überlappendem Namensbereich fehl.
