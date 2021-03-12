# M5StickC-ATEM-Tally
M5Stick-C as Tally Light for BMD ATEM Switchers

## Beschreibung
M5StickC-ATEM-Tally verbindet sich mit einem Blackmagic ATEM Switcher und steuert ein M5Stick-C ESP32 Arduino device von M5Stack mit den Tally Informationen.

## Vorbereitung
- Python 2.7.x und Arduino-IDE mit den für M5Stick-C und ATEM notwendigen Bibliotheken. 
- Anpassung von ATEMbase.* für den ESP32 im M5Stick-C: "#ifdef ESP8266" -> "#if defined(ESP8266) || defined(ESP32)" 
- Auswahl eines Debug-Levels durch Auskommentierung des passenden Werts. 
- Eintragen von SSID und Kennwort fürs WLAN. 
- Eintragen von Name und ggfs. IPv4 des Switchers. 

## Bedienung
Nach dem Einschalten werden in mehreren Schritten Setup-Informationen angezeigt, die jeweils mit dem M5-Schalter bestätigt werden müssen. 
Mit dem Schalter an der linken Seite kann der Stick ein (kurz) und aus (6 sec) geschaltet werden. 
Mit dem Schalter an der rechten Seite kann die automatische Orientierung des Displays aktualisiert werden. 
Mit einem kurzen Druck auf M5 werden die Setup-Schritte bestätigt und danach bis zu 8 Kameras zyklisch durchgeschaltet. 
Mit einem langen Druck auf M5 wird das Setup wiederholt. 
Nach dem ersten Setup-Schritt ist der WiFi-Teil abgeschaltet; dann kann gut eine neue Version über USB geflasht werden. 
Nach dem zweiten Setup-Schritt ist WiFi verbunden; dann kann eine neue Version über OTA geflasht werden. 

## Anzeigen
- gleichmäßiges Blinken der LED: Warten auf M5 beim Setup
- kurzes Blinken der LED mit 3 Sekunden Pause: Warten auf einen WiFi-Connect; bis dahin ist der Stick geblockt
- schwarze Kameranummer auf grünem Grund: Preview aktiv
- schwarze Kameranummer auf rotem Grund: Program aktiv
- grüne Kameranummer auf rotem Grund: Preview und Program aktiv

## Ideen
Verbindung zu einer Instanz von Bitfocus Companion, um dem 5-Client-Limit der ATEM Switcher zu entgehen
