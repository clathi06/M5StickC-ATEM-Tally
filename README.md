# M5StickC-ATEM-Tally
M5Stick-C(Plus) als Tally Light für BMD ATEM Switcher

## Beschreibung
M5StickC-ATEM-Tally verbindet sich mit einem Blackmagic ATEM Switcher und steuert einen M5Stick-C(Plus) mit ESP32 von M5Stack mit den Tally Informationen.
Zur Umgehung der Beschränkung auf 5 Clients kann die Kommunikation auch über einen MQTT-Server aka Broker erfolgen. 
Dann dient ein Stick als Publisher und die anderen als Subscriber. 

## Vorbereitung
- Python 2.7.x und Arduino-IDE mit den für M5Stick-C(Plus) und ATEM notwendigen Bibliotheken. 
- Anpassung von ATEMbase.* für den ESP32: "#ifdef ESP8266" -> "#if defined(ESP8266) || defined(ESP32)".
- Auswahl eines Debug-Levels durch Auskommentierung des passenden Werts. 
- ggfs. Auskommentieren des #define M5STICKCPLUS für den verwendeten Stick
- ggfs. Eintragen von Namen und IPv4 der ATEM-Switcher und des Brokers 

## Anzeigen
Setup-Modus
- abwechselnd 3 Sekunden LED rot und grün (am LED-Hat): Warten auf Eingabe

Setup- und Tally-Modus
- 0,1 Sekunde alle LED an mit 1 Sekunde Pause: Warten auf einen WiFi und/oder MQTT-Connect

Tally-Modus
- rote LED an: Program aktiv
- grüne LED am LED-Hat an: Preview aktiv
- kurz gelbe Kameranummer auf schwarzem Grund: WiFi und/oder MQTT nicht verbunden
- permanent weisse/graue Kameranummer auf schwarzem Grund: weder Preview noch Program aktiv
- schwarze Kameranummer auf grünem Grund: Preview aktiv
- schwarze Kameranummer auf rotem Grund: Program aktiv
- grüne Kameranummer auf rotem Grund: Preview und Program aktiv

## Bedienung
Schalter an der linken Seite 
- kurz Einschalten, ganz lang (6 sec) Ausschalten
- die saubere Methode zum Ausschalten ist ein Reboot auf die Setup-Seite 1 und ggfs. Verkürzen des TimeOut und Abwarten

Schalter unterhalb vom Display (M5) 
- Setup-Modus: kurz nächste Seite, lang (1 sec) grds. eine Seite zurück plus teilweise zusätzliche Aktionen
- Tally-Modus: kurz bis zu 8 Kameras zyklisch durchschalten, lang Stick neu starten

Schalter an der rechten Seite (ButtonB)
- Setup-Modus: kurz teilweise grüne Option ändern, lang ohne Funktion
- Tally-Modus: kurz Ausrichtung des Displays ändern, lang automatische Ausrichtung ein- bzw ausschalten 

TimeOut 
- Setup-Modus: von Seite 1 aus Stick auschalten, sonst Stick neu starten
- Tally-Modus: nach Verlust der WiFi-Verbindung Stick neu starten

## Setup-Modus
Seite 1 Info
- Ausgabe von Infos zum Stick und zur Bedienung
- ButtonB: Ändern der Zeit bis zum TimeOut
- M5 lang: Setup mit den zuletzt gespeicherten Einstellungen automatisch durchlaufen

Seite 2 Modus Wahl
- Auswahl des Modus, in dem der Stick arbeiten soll
- TA Tally@ATEM: Tally Light am ATEM
- PA PubClient@ATEM: MQTT Publisher am ATEM
- TB Tally@Broker: Tally Light am MQTT Broker
- ButtonB: Modus auswählen, ganz oben steht der zuletzt gespeicherte
- M5 lang: alle gespeicherten Einstellungen löschen

Seite 3 ATEM Wahl im Modus TA und PA
- Auswahl des Switcher-Typs mit Anzahl der Inputs
- ButtonB: Typ auswählen, ganz oben steht der zuletzt gespeicherte

Seite 4 Broker Wahl im Modus PA und TB
- Auswahl des MQTT-Servers mit Anzahl der Inputs für den Modus TB
- ButtonB: Server auswählen, ganz oben steht der zuletzt gespeicherte

Seite 5 WiFi Manager
- Start des WiFi-Managers: https://github.com/tzapu/WiFiManager
- Beim ersten Aufruf als AutoConnect
- Bei Folgeaufrufen Start des Config-Portals ATEM-Tally@M5StickC
- Im Config-Portal Eingabe von SSID, Kennwort sowie optional Hostname (ohne Domain) und IPv4 von ATEM-Switcher und MQTT-Server
- M5 lang: Beim Warten auf WiFi-Connect Stick neu starten, sonst gespeicherte Zugangsdaten löschen

Seite 6 WiFi Einstellungen
- Anzeige von Informationen zum WLAN (WiFi)
- Start des http-Servers zum Flashen einer neuen Firmware
- M5 lang: beim Warten auf WiFi-Connect Stick neu starten

Seite 7 ATEM Einstellungen im Modus TA und PA
- DNS-Anfrage zur Auflösung des Hostnamens in eine IPv4-Adresse
- ButtonB: Wechsel zwischen der per DNS ermittelten und der auf Seite 2 ausgewählten oder während Seite 5 im Config-Portal eingegebenen IPv4
- M5 kurz: Setup beenden im Modus TA

Seite 8 Broker Einstellungen im Modus PA und TB
- DNS-Anfrage zur Auflösung des Hostnamens in eine IPv4-Adresse
- ButtonB: Wechsel zwischen der per DNS ermittelten und der auf Seite 3 ausgewählten oder während Seite 5 im Config-Portal eingegebenen IPv4
- M5 kurz: Setup beenden

Seite 9 Broker Start im Modus PA und TB
- Warten auf Connect zum MQTT-Server
- nach Connect Setup beenden
- M5 lang: beim Warten auf MQTT-Connect Stick neu starten (funktioniert nur kurz während die LED flasht, der Connect-Versuch blockiert den Stick ziemlich lange)

Vor dem Wechsel in den Tally-Modus werden die Setup-Daten in den Preferences gespeichert. 

## ToDo
- Inline Dokumentation verbessern

