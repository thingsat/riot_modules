# Thingsat :: Basic Mission

This application illustrates a simple mission for the [Thingsat in-orbit gateway](https://gricad-gitlab.univ-grenoble-alpes.fr/thingsat/public/-/tree/master/cubesat_mission_3).

* [x] GNSS on UART parsing when a GNSS module is present on Mikrobus or on MiniPCIe module (as into the RAK5146 module).
* [x] LoRaWAN repeater (with devaddr filtering)
* [ ] add App Clock Sync
* [ ] add 5.9 DeviceTime commands (DeviceTimeReq, DeviceTimeAns) introduced in lorawan specification v1.0.3
* [x] Chirpstack Mesk Relay (Very simple implementation) (SHOULD BE TESTED)
* [x] Meshtastic Router (Very simple implementation)
* [ ] Periodic telemetry (with GNSS position and temperature) using LoRaWAN with Cayenne LPP (and XOR redundancy)
* [ ] Periodic telemetry (with GNSS position and temperature) using Meshtastic telemetry and position (protobuf)
* [ ] Periodic telemetry (with GNSS position and temperature) using APRS format (not fake)
* [ ] Add shell command for printing GNSS data (position, speed, quality, cog ...)
* [ ] Periodic two-way ranging with other Thingsat boards
* [ ] LoRa 2G4 backhaul when the SX1280 module is present
* [ ] Send [AIS](https://en.wikipedia.org/wiki/Automatic_identification_system) messages over LoRa/LoRaWAN/Meshtastic ([see spec](https://www.itu.int/dms_pubrec/itu-r/rec/m/R-REC-M.1371-5-201402-I!!PDF-F.pdf))
* [ ] Receive and decode [AIS](https://en.wikipedia.org/wiki/Automatic_identification_system) messages over LoRa/LoRaWAN/Meshtastic ([see spec](https://www.itu.int/dms_pubrec/itu-r/rec/m/R-REC-M.1371-5-201402-I!!PDF-F.pdf))

## Setup

```bash
export RIOTBASE=~/github/RIOT-OS/RIOT
cd ~/github/thingsat/riot_modules
cd tests/driver_sx1302
```

For Thingsat UP4 (default board)
```bash
export BOARD=thingsat-up4
```

For Thingsat UP1 F4 (SatRev Stork-1)
```bash
export BOARD=thingsat-up1-f4
```

For Nucleo F446RE with Corecell
```bash
export BOARD=nucleo-f446re
```

For Nucleo L476RG With RAK5146
```bash
export BOARD=nucleo-l476rg
```

For Nucleo L432KC With RAK5146
```bash
export BOARD=nucleo-l432kc
```

For ESP32 WROOM (With RAK5146)
```bash
gmake BUILD_IN_DOCKER=1 BOARD=esp32-wroom-32 -j 8 flash term
```

For  Arduino Nano ESP32 (With RAK5146)
```bash
gmake BUILD_IN_DOCKER=1 BOARD=esp32-nano -j 8 flash term
```

### Setup for [Thingsat ProtoSEED](https://gricad-gitlab.univ-grenoble-alpes.fr/thingsat/seed/-/tree/main/seed_thingsat_protoseed) 📡📡📡📡📡🎈🎈🎈🎈🎈🎈

For Nucleo L432KC With RAK5146 on ProtoSEED board
```bash
export BOARD=nucleo-l432kc-protoseed
gmake BOARD=$BOARD -j 8 flash term
```
### Setup for [Thingsat INISAT](https://github.com/csu-grenoble/flatsat/tree/main/Hardware/Thingsat_INISAT#carte-thingsat--inisat----obc--communication-avec-nucleo-l432kc--gateway-rak5146) 📡📡📡📡📡🎈🎈🎈🎈🎈🎈

For Nucleo L432KC With RAK5146 on INISAT board
```bash
export BOARD=nucleo-l432kc-inisat
gmake BOARD=$BOARD -j 8 flash term
```

### Setup for [high altitude balloon flights](https://gricad-gitlab.univ-grenoble-alpes.fr/thingsat/public/-/blob/master/balloons/README.md) 📡📡📡📡📡🎈🎈🎈🎈🎈🎈

Nucleo L432KC With RAK5146 on INISAT board with [OpenLog](https://github.com/CampusIoT/tutorial/tree/master/openlogger)

```bash
export PROD=1
export BOARD=nucleo-l432kc-inisat
gmake BOARD=$BOARD OPENLOG_BAUDRATE=9600 PROD=$PROD GPS_UART_ENABLE=0 GPS_UART_ENABLE_TRACE=0 -j 8 flash term
```

> Nota Bene: when OPENLOG_BAUDRATE is 9600 (ie slow), the GNSS parsing misses some characters in the ring buffer : `Bad Checksum` traces are then printed into the console

### Setup for [Meshtastic](https://meshtastic.org)

```bash
export BOARD=nucleo-l432kc-inisat
gmake BOARD=$BOARD MESHTASTIC_ENABLE=1 OPENLOG_BAUDRATE=9600 GPS_UART_ENABLE=1 GPS_UART_ENABLE_TRACE=0 -j 8 flash term
```
> Nota Bene: when OPENLOG_BAUDRATE is 9600 (ie slow), the GNSS parsing misses some characters in the ring buffer : `Bad Checksum` traces are then printed into the console

### Setup for [Chirpstach Mesh Relay](https://www.chirpstack.io/docs/chirpstack-gateway-mesh/index.html)

```bash
export BOARD=nucleo-l432kc-inisat
gmake BOARD=$BOARD CHIRPSTACK_MESH_ENABLE=1 OPENLOG_BAUDRATE=9600 GPS_UART_ENABLE=1 GPS_UART_ENABLE_TRACE=0 -j 8 flash term
```

## Disable the gateway autostart 

Add `LGW_AUTOSTART_ENABLE=0` into the arguments list of the `make` command in order to disable the gateway at boot time.

## Disable the shell 

Add `NO_SHELL=1` into the arguments list of the `make` command in order to disable the shell.

## Console
```bash
tio -L
tio -b 115200 -m INLCRNL /dev/tty.usbmodem1xxx
tio -b 9600 -m INLCRNL /dev/tty.usbmodem1xxx
```

## OpenLog

[OpenLog](https://github.com/CampusIoT/tutorial/tree/master/openlogger) board enables to store the console traces into a MicroSD card with the board is running standalone (ie not connected to a PC).

Since OpenLog UART baudrate should be `9600` in order to have reliable writes of the console traces.

You should add `OPENLOG_BAUDRATE=9600` into the `gmake` command line.

> Nota Bene: when OPENLOG_BAUDRATE is 9600 (ie slow), the GNSS parsing misses some characters in the ring buffer : `Bad Checksum` traces are then printed into the console

## RX Thread Stack

In case of **"RIOT kernel panic: MEM MANAGE HANDLER" error**, enlarge the stack of the thread running the RX cmd 


```
> lgw rx 10000 4
rxpkt buffer size is set to 4
Waiting for packets...
WARNING: not enough space allocated, fetched 11 packet(s), 7 will be left in RX buffer
*** RIOT kernel panic:
MEM MANAGE HANDLER

*** halted.

Inside isr -12
```

```makefile

CFLAGS += -DTHREAD_STACKSIZE_MAIN=65536U

```

