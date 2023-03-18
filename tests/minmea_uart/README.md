# MINMEA on UART : reading and parse NMEA sentences for a GNSS module

simple application for reading and parse NMEA sentences for a GNSS module connected to one UART


```bash
cd app/minmea_uart/
gmake BOARD=lora-e5-dev
cp bin/lora-e5-dev/tests_minmea_uart.bin  /Volumes/NUCLEO/
```

Si vous ne possédez pas de module, vous pouvez tester en croissant les fils comme à l'exercice précédent et en envoyant la phrase NMEA GLL `$GNGLL,5229.0178,N,01326.7605,E,114350.000,A,A*45` à la deuxième UART.

```
UARD_DEV(0): test uart_poweron() and uart_poweroff()  ->  [OK]

UART INFO:
Available devices:               3
UART used for STDIO (the shell): UART_DEV(0)

> help
Command              Description                                                
---------------------------------------                                         
init                 Initialize a UART device with a given baudrate             
mode                 Setup data bits, stop bits and parity for a given UART deve
send                 Send a string through given UART device                    
test                 Run an automated test on a UART with RX and TX connected   
> init 1 9600                                                                   
Success: Initialized UART_DEV(1) at BAUD 9600                                   
UARD_DEV(1): test uart_poweron() and uart_poweroff()  ->  [OK]                  
> init 2 9600                                                                   
Success: Initialized UART_DEV(2) at BAUD 9600                                   
UARD_DEV(2): test uart_poweron() and uart_poweroff()  ->  [OK]                  
> send 2 TEST                                                                   
UART_DEV(2) TX: TEST                                                            
Success: UART_DEV(1) RX: [TEST]\n                                               
> send 1 $GNGLL,5229.0178,N,01326.7605,E,114350.000,A,A*45
UART_DEV(1) TX: $GNGLL,5229.0178,N,01326.7605,E,114350.000,A,A*45               
parsed coordinates: lat=52.483631 lon=13.446008                                 
SUCCESS                                                                         
Success: UART_DEV(2) RX: [$GNGLL,5229.0178,N,01326.7605,E,114350.000,A,A*45]\n  
> send 1 $GPZDA,024611.08,25,03,2002,00,00*6A
UART_DEV(1) TX: $GPZDA,024611.08,25,03,2002,00,00*6A
parsed date: 2002-3-25 2:46:11.80000 offset=0:0
SUCCESS
Success: UART_DEV(2) RX: [$GPZDA,024611.08,25,03,2002,00,00*6A]\n

> send 1 $GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18
UART_DEV(1) TX: $GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18
TODO: parse NMEA sentence 2.
Success: UART_DEV(2) RX: [$GPGGA,161229.487,3723.2475,N,12158.3416,W,1,07,1.0,9.0,M,,,,0000*18]\n

> send 1 $GPRMC,092204.999,A,4250.5589,S,14718.5084,E,0.00,89.68,211200,,*25
UART_DEV(1) TX: $GPRMC,092204.999,A,4250.5589,S,14718.5084,E,0.00,89.68,211200,,*25
parsed Position and time: valid=1 date=0-12-21 9:22:4.999000
 lat=-42.842647 lng=147.308471 speed=0 course=1.494666 variation=0
SUCCESS
Success: UART_DEV(2) RX: [$GPRMC,092204.999,A,4250.5589,S,14718.5084,E,0.00,89.68,211200,,*25]\n

> send 1 $GPGSV,1,1,01,21,00,000,*4B
UART_DEV(1) TX: $GPGSV,1,1,01,21,00,000,*4B
TODO: parse NMEA sentence 6.
Success: UART_DEV(2) RX: [$GPGSV,1,1,01,21,00,000,*4B]\n

> send 1 $GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
UART_DEV(1) TX: $GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70
TODO: parse NMEA sentence 6.
Success: UART_DEV(2) RX: [$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70]\n
```

Terminez le traitement des autres phrases NMEA. Vous pouvez essayer d'[autres exemples de phrases NMEA](https://www.satsleuth.com/GPS_NMEA_sentences.aspx) pour mettre au point votre programme.

> Il existe des modules GNSS beaucoup plus performant que le module SIM-28 du module Grove.

> A noter: ce programme est l'embryon d'un suiveur (véhicule, ballon stratosphérique ...).
