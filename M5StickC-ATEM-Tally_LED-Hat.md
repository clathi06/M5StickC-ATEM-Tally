# M5StickC-ATEM-Tally LED-Hat
Erweiterungsplatine für den M5Stick-C(Plus)

## Beschreibung
Im Außeneinsatz ist die eingebaute LED der Sticks nicht hell genug. 
Aber M5Stack hat einen Anschluss für Erweiterungsmodule - sogenannte Hats - vorgesehen. 
Diesen kann man nutzen, um mit GPIO 0 und 26 zwei zusätzliche LED's anzusteuern. 
Diese GPIO's dürfen allerdings nur mit wenigen mA belastet werden. 
Deshalb sind die LED's an der 3,3 und an der 5 Volt-Schiene angeschlossen. 
Der BAT-Pin ist zur Vermeidung von Kurzschlüssen abgekniffen. 

## Material
- eine beidseitig mit Lötaugen versehene Platine mit 8x9 oder 8x10 Löchern im Rastermaß 2,54 mm. 
- eine Pfostenleiste mit 8 Stiften
- 2 NPN-Transistoren, z.B. PN 2222A
- 2 Widerstände dafür mit 2,2 kOhm
- 2 helle rote LED's, z.B. SLOAN L5-R50U mit 4.200 mcd bei 20 mA an 2,2 V
- 1 dazu passender Widerstand, hier 47 Ohm
- 1 helle grüne LED, z.B. SLOAN L5BG1G mit 4.600 mcd bei 20 mA an 3,8 V
- 1 dazu passender Widerstand, hier 150 Ohm (68 gehen auch, ist dann aber sehr hell)
- 1 isolierter Draht zur Verbindung der Masse bzw. zwei, wenn die Erkennung über GPIO 36 irgendwann mal funktioniert

## Benutzung
Einfach anstecken, auch während des laufenden Betriebs. 
Bei der Verwendung in (ggfs. auch noch abgedunkelten) Innenräumen muss der Hat nicht abgezogen werden. 
Dazu gibt es zwei Modi, die sich immer beim Überlauf der Auswahl für die Kameras abwechseln. 
Gestartet wird hell: Hat-LED's sind an und eine inaktive Kamera wird mit weisser Schrift auf schwarzem Grund dargestellt. 
Beim Überlauf erfolgt der Wechsel zu dunkel: Hat-LED's bleiben aus und die Schrift ist grau. 

## ToDo
- Schaltplan zeichnen
- jemanden finden, der mit einem 3D-Drucker ein Gehäuse herstellt
