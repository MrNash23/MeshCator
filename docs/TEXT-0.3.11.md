# Text und Sprachen – 0.3.11-dev

Am 31.10.2026 installiert: USB-Datensicherung und vollständiger vorheriger App-Slot gesichert, Firmware und Erhalt aller übrigen Partitionen verifiziert. USB antwortet nach Neustart. 23/23 Tests bestanden; sichtbare Abnahme am Pager noch offen.

Gesprächsnamen in der Übersicht sind auf eine Zeile begrenzt und werden bei Platzmangel mit Punkten gekürzt. Die Vorschau besitzt eine eigene feste Höhe; sie kann den Fokusbereich nicht überdecken. Die bisherige byteweise Kürzung wurde entfernt, damit mehrbyteige UTF-8-Zeichen nicht zerschnitten werden. Auch die Überschrift im geöffneten Chat ist auf eine Zeile begrenzt.

Deutsche UI-Texte nutzen nun echte Umlaute und ß (etwa Gespräche, Kanäle, Öffnen, Auswählen und Schließen). Fehlende Übersetzungen für Nachricht und EMPFANGEN wurden ergänzt. Französisch nutzt nœud statt noeud.

Alle acht Schriftgrößen enthalten ASCII, Latin-1 einschließlich ÄÖÜäöüß, französischer Akzente, Æ/æ und Ç/ç, spanischer Akzente, Ñ/ñ, ¿/¡; zusätzlich Œ/œ, Ÿ, ẞ und ’ sowie Euro. Der bestehende Emoji-Fallback bleibt erhalten. Nur die große 36-Pixel-Schrift verzichtet aus Platzgründen auf Kerningtabellen; ihre Glyphen bleiben vollständig enthalten. Dies ist kein vollständiger Unicode-Zeichensatz; beliebige andere Schriften oder kombinierende Unicode-Sequenzen sind damit nicht allgemein abgedeckt.

`tests/test_font_coverage.py` prüft die tatsächlich generierten Glyphen aller acht Fonts gegen den Übersetzungskatalog und die besonderen Buchstaben aller vier Sprachen. Er prüft zusätzlich, dass die ersetzten deutschen ASCII-Umschreibungen nicht wiederkehren. Der Simulator kann mit `--text-preview --screen 0` lange Gesprächsnamen und mehrsprachige Nachrichtentexte darstellen. Screenshot: `implementation/chat-text.png`.
