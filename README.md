# M5StickC-ATEM-Tally
M5Stick-C(Plus) als Tally Light für BMD ATEM Switcher

## Beschreibung
M5StickC-ATEM-Tally verbindet sich mit einem Blackmagic ATEM Switcher und steuert einen M5Stick-C(Plus) mit ESP32 von M5Stack mit den Tally Informationen.

## Vorbereitung
- Python 2.7.x und Arduino-IDE mit den für M5Stick-C(Plus) und ATEM notwendigen Bibliotheken. 
- Anpassung von ATEMbase.* für den ESP32: "#ifdef ESP8266" -> "#if defined(ESP8266) || defined(ESP32)".
- Auswahl eines Debug-Levels durch Auskommentierung des passenden Werts. 
- Auskommentieren des #define M5STICKCPLUS für das verwendete Board
- ggfs. Eintragen von SSID und Kennwort fürs WLAN (WiFi). 
- ggfs. Eintragen von Name und ggfs. IPv4 der ATEM-Switcher. 

## Anzeigen
Setup-Modus
- 10 Sekunden LED an mit 50 Sekunden Pause: Warten auf Eingabe

Setup- und Tally-Modus
- 0,1 Sekunde LED an mit 2 Sekunden Pause: Warten auf einen WiFi-Connect

Tally-Modus
- LED an: Program aktiv
- graue Kameranummer auf schwarzem Grund: weder Preview noch Program aktiv (oder ATEM nicht verbunden)
- schwarze Kameranummer auf grünem Grund: Preview aktiv
- schwarze Kameranummer auf rotem Grund: Program aktiv
- grüne Kameranummer auf rotem Grund: Preview und Program aktiv

## Bedienung
Schalter an der linken Seite 
- kurz Einschalten, ganz lang (6 sec) Ausschalten

Schalter unterhalb vom Display (M5) 
- Setup-Modus: kurz nächste Seite, lang eine Seite zurück plus teilweise zusätzliche Aktionen
- Tally-Modus: kurz bis zu 8 Kameras zyklisch durchschalten, lang Stick neu starten

Schalter an der rechten Seite (ButtonB)
- Setup-Modus: kurz teilweise grüne Option ändern, lang (1 sec) ohne Funktion
- Tally-Modus: kurz Ausrichtung des Displays ändern, lang automatische Ausrichtung ein- bzw ausschalten 

TimeOut 
- Setup-Modus: von Seite 1 aus Stick auschalten, sonst Stick neu starten
- Tally-Modus: nach Verlust der WiFi-Verbindung Stick neu starten

## Setup-Modus
Seite 1 Info
- Ausgabe von Infos zum Stick und zur Bedienung
- ButtonB: Ändern der Zeit bis zum TimeOut
- M5 lang: Stick ausschalten

Seite 2 Switcher
- Auswahl des Switcher-Typs mit Anzahl der Inputs
- ButtonB: Typ auswählen,  ganz oben steht der zuletzt gespeicherte
- M5 lang: gespeicherten Switcher löschen

Seite 3 WiFi-Manager
- Start des WiFi-Managers: https://github.com/tzapu/WiFiManager
- Beim ersten Aufruf als AutoConnect
- Bei Folgeaufrufen Start des Config-Portals ATEM-Tally@M5StickC
- Im Config-Portal Eingabe von SSID, Kennwort sowie optional Name und IPv4 des Switchers
- M5 lang: Beim Warten auf Connect Stick neu starten, sonst gespeicherte Zugangsdaten löschen

Seite 4 WiFi
- Anzeige von Informationen zum WLAN (WiFi)
- Start des http-Servers zum Flashen einer neuen Firmware
- M5 lang: beim Warten auf Connect Stick neu starten

Seite 5 DNS
- DNS-Anfrage zur Auflösung des Hostnamens in eine IPv4-Adresse
- ButtonB: Wechsel zwischen der per DNS ermittelten und der ausgewählten oder eingegebenen IPv4
- M5 kurz: Wechsel in den Tally-Modus und Speicherung der Switcher-Daten

## ToDo
- Inline Dokumentation

