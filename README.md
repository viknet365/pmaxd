pmaxd
=====

daemon for communication between linux and a visonic powermax alarm

work well on visonic powermax pro with openwrt and a tplink 703N

see this thread for more information:
http://www.domoticaforum.eu/viewtopic.php?f=68&t=7152

The external script must set as follow

Directory /etc/pmaxd/

alarm

armedAway

armedHome

disarmed

pmaxd.conf

zoneActive

zoneBatt

zoneClose


zoneEvent

zoneOpen


Example of zoneOpen

#!/bin/sh

NOW=$(date +"%d/%m/%Y %H:%M")



if [ $1 = "4" ]; then #Puerta

EMAIL="To: email@gmail.com
Subject: ALARMA - Zona Puerta abierta $NOW

Zona Puerta abierta
$NOW
"

echo "$EMAIL" | sendmail -t

 
