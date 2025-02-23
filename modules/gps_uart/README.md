# GPS over UART

https://github.com/CampusIoT/orbimote/blob/master/gnss_modules.md

## Makefile definition
* `GPS_UART_DEV`
* `GPS_UART_BAUDRATE`
* `GPS_POWER_PIN`
* `GPS_STANDBY_PIN`
* `GPS_RESET_PIN`
* `GPS_PPS_INT_PIN`
* `GPS_UART_ENABLE_TRACE`

## TODO
* [x] callback function on GPIO PPS interuptpin (GPS_PPS_PIN != GPIO_UNDEF)
* [x] GPIO RESET pin
* [x] add `GPS_POWER_PIN`
* [ ] add `GPS_STANDBY_PIN`
* [ ] fix `last fix`

## Tested (balloon-mode) modules
* [x] MediaTek MT3329 on https://www.digikey.fr/fr/products/detail/adafruit-industries-llc/746/5353613
* [ ] https://www.digikey.fr/fr/products/detail/adafruit-industries-llc/3133/6047744
* [ ] https://www.digikey.fr/fr/products/detail/sparkfun-electronics/GPS-14414/7803411
* [ ] https://www.digikey.fr/fr/products/detail/sparkfun-electronics/GPS-15210/10064422
* [x] uBlox ZOE-M8Q (on RAK5146) : UART 9600 baud. no power pin. no external PPS pin. reset pin. standby pin.


## Shell
```
> gps
usage: gps init <dev> <baudrate>
usage: gps print
usage: gps reset
usage: gps poweron
usage: gps poweroff
usage: gps standby
usage: gps wakeup
> gps poweron
> gps print
WARNING: GPS no fix
INFO: GPS Parse NMEA errors   : 0
INFO: GPS Parse NMEA unknowns : 0
INFO: GPS last fix time : 0
> gps init 2 9600
ERROR: Can not parse NMEA Invalid sentence 	?b??b??bbbbbb?r??b?r??b?r??R??j : -1.
> gps print
INFO: GPS Parse NMEA errors   : 1
INFO: GPS Parse NMEA unknowns : 0
INFO: GPS last fix time : 34
INFO: GPS time sec=1708118350 nsec=0
INFO: GPS lat=45.182190°, lon=5.735520°, alt=219m
INFO: GoogleMap : https://www.google.fr/maps/@45.182190,5.735520,17z
INFO: GPS speed=0.0 m/s (0.1 km/h), track=nan°
INFO: GPS pdop=1.99, hdop=1.30, vdop=1.51
INFO: GPS fix_quality=2, satellites_tracked=11
> gps reset
ERROR: Can not parse NMEA Invalid sentence $GNGSA,?$GNTXT,01,01,02,u-blox AG - www.u-blox.com*4E : -1.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,HW UBX-M8030 00080000*60 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,ROM CORE 3.01 (107888)*2B : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,FWVER=SPG 3.01*46 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,PROTVER=18.00*11 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,GPS;GLO;GAL;BDS*77 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,SBAS;IMES;QZSS*49 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,GNSS OTP=GPS;GLO*37 : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,LLC=FFFFFFFF-FFFFFFED-FFFFFFFF-FFFFFFFF-FFFFFF6D*5E : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,ANTSUPERV=AC SD PDoS SR*3E : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,ANTSTATUS=DONTKNOW*2D : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,PF=3FF*4B : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,ANTSTATUS=INIT*3B : 0.
ERROR: Can not parse NMEA Unknown sentence $GNTXT,01,01,02,ANTSTATUS=OK*25 : 0.
> gps print
INFO: GPS Parse NMEA errors   : 2
INFO: GPS Parse NMEA unknowns : 13
INFO: GPS last fix time : 49
INFO: GPS time sec=1708118365 nsec=0
INFO: GPS lat=45.000209°, lon=5.000537°, alt=218m
INFO: GoogleMap : https://www.google.fr/maps/@45.182209,5.735537,17z
INFO: GPS speed=0.0 m/s (0.1 km/h), track=nan°
INFO: GPS pdop=2.08, hdop=1.39, vdop=1.55
INFO: GPS fix_quality=1, satellites_tracked=9
> gps print
INFO: GPS Parse NMEA errors   : 2
INFO: GPS Parse NMEA unknowns : 13
INFO: GPS last fix time : 131
INFO: GPS time sec=1708118447 nsec=0
INFO: GPS lat=45.000220°, lon=5.000498°, alt=220m
INFO: GoogleMap : https://www.google.fr/maps/@45.182220,5.735498,17z
INFO: GPS speed=0.0 m/s (0.1 km/h), track=nan°
INFO: GPS pdop=2.08, hdop=1.39, vdop=1.55
INFO: GPS fix_quality=2, satellites_tracked=12

```
	