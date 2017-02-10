pmaxd
=====

daemon for communication between linux and a visonic powermax alarm

work well on visonic powermax pro with openwrt and a tplink 703N

see this thread for more information:
http://www.domoticaforum.eu/viewtopic.php?f=68&t=7152

The external script must set as follow

Directory /etc/pmaxd/
root@OpenWrt:/etc/pmaxd# ls -lah
drwxr-xr-x    2 root     root        1.0K Aug 31 10:00 .
drwxr-xr-x    1 root     root        2.0K Feb 10 03:01 ..
-rwxr-xr-x    1 root     root         253 Jun 18  2016 alarm
-rwxrwxr-x    1 root     root         300 Nov 11 12:11 armedAway
-rwxrwxr-x    1 root     root         314 Nov 11 12:12 armedHome
-rwxrwxr-x    1 root     root         311 Nov 11 12:11 disarmed
-rw-r--r--    1 root     root         260 Jun 16  2016 pmaxd.conf
-rwxrwxr-x    1 root     root         194 Aug 31 02:35 zoneActive
-rwxrwxr-x    1 root     root         231 Jun 18  2016 zoneBatt
-rwxrwxr-x    1 root     root         472 Aug 31 17:00 zoneClose
-rwxrwxr-x    1 root     root        1.4K Aug 31 17:50 zoneEvent
-rwxrwxr-x    1 root     root         469 Aug 31 17:00 zoneOpen


