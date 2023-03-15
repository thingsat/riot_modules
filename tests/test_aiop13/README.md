# Port of AioP13 library on RIOT

## AioP13
AioP13 library is a another implementation (C++) of G3RUHs PLAN-13 for satellite (from TLE) and sun prediction for Arduino. 
* https://github.com/dl9sec/AioP13
* https://www.arduino.cc/reference/en/libraries/aiop13/

Originally authored by Mark VandeWettering K6HX (https://github.com/brainwagon/angst/tree/master/P13) 

It enables to calculate the sun and sat footprint from displaying on a LCD screen.

 Satellites [TLE](https://en.wikipedia.org/wiki/Two-line_element_set) catalog is maintained by the NORAD. Several website enables TLE searching into the NORAD database (ie https://celestrak.org/satcat/table-satcat.php?NAME=STORK&PAYLOAD=1&MAX=500, https://celestrak.org/NORAD/elements/gp.php?CATNR=51087)


## Build

```bash
export RIOTBASE=~/github/RIOT-OS/RIOT
make flash term
```

### Makefile Options

* CXXEXFLAGS : user's extra flags used to build c++ files should be defined here (e.g -std=gnu++11).

See ["Using C++ and C in a program with RIOT"](https://doc.riot-os.org/using-cpp.html)


## Console

```
> rtc settime 2022-05-18 13:08:00
> rtc gettime
2022-05-18 13:08:08
> observer
Observer: lat: 45.192721 lon: 5.759947 alt: 220.000000
> tle
Current time:   2022-05-18 13:12:16

Prediction for STORK-1 at CSUG (MAP 320x240: x = 165,y = 59):

2022-05-18 13:12:15 -> Lat: 72.583292 Lon: -23.327285 (MAP 320x240: x = 139,y = 23) Az: 343.46 El: -6.85

RX: 867.507242 MHz, TX: 867.492758 MHz

Satellite footprint map coordinates:
 0: x = 299, y = 6
 1: x = 261, y = 8
 2: x = 239, y = 12
 3: x = 225, y = 16
 4: x = 215, y = 20
 5: x = 207, y = 25
 6: x = 200, y = 29
 7: x = 193, y = 33
 8: x = 187, y = 37
 9: x = 181, y = 40
10: x = 175, y = 44
11: x = 169, y = 46
12: x = 163, y = 49
13: x = 157, y = 50
14: x = 151, y = 52
15: x = 145, y = 52
16: x = 139, y = 53
17: x = 133, y = 52
18: x = 127, y = 52
19: x = 121, y = 50
20: x = 115, y = 49
21: x = 109, y = 46
22: x = 103, y = 44
23: x = 97, y = 40
24: x = 91, y = 37
25: x = 84, y = 33
26: x = 78, y = 29
27: x = 70, y = 25
28: x = 62, y = 20
29: x = 52, y = 16
30: x = 39, y = 12
31: x = 17, y = 8
> sun
Current time:   2022-05-18 13:12:19

Sun -> Lat: 19.513544 Lon: -19.451998 (MAP 320x240: x = 142,y = 93) Az: 227.37 El: 56.92

Sunlight footprint map coordinates:
 0: x = 302, y = 26
 1: x = 275, y = 29
 2: x = 257, y = 39
 3: x = 246, y = 51
 4: x = 239, y = 64
 5: x = 233, y = 77
 6: x = 229, y = 91
 7: x = 226, y = 105
 8: x = 222, y = 119
 9: x = 219, y = 134
10: x = 215, y = 148
11: x = 211, y = 162
12: x = 206, y = 175
13: x = 199, y = 188
14: x = 188, y = 200
15: x = 170, y = 210
16: x = 142, y = 213
17: x = 115, y = 210
18: x = 97, y = 200
19: x = 86, y = 188
20: x = 79, y = 175
21: x = 73, y = 162
22: x = 69, y = 148
23: x = 66, y = 134
24: x = 62, y = 119
25: x = 59, y = 105
26: x = 55, y = 91
27: x = 51, y = 77
28: x = 46, y = 64
29: x = 39, y = 51
30: x = 28, y = 39
31: x = 10, y = 29
> 

```

## TODO

* [ ] tle_command shell command (list, set name <name>, set line1 <line1>, set line2 <line2> ...)
* [ ] observer_command shell command (set lat lon alt)
* [ ] add TFT pushImage and draw if device such as [STM32F769I-DISCO](https://www.st.com/en/evaluation-tools/32f469idiscovery.html) HAS_SCREEN like in https://github.com/dl9sec/AioP13/blob/master/examples/PredictISS_TFT/PredictISS_TFT.ino