# RIOT port of the Semtech SX1302+SX1250 V2.1.0 driver

https://github.com/Lora-net/sx1302_hal

## TODO

* [x] add I2C STTS751
* [x] add temperature thru SAUL sensor
* [ ] add [sx1261/sx1262](https://doc.riot-os.org/group__drivers__sx126x__internal.html)
* [ ] add LBT
* [ ] add spectral scan
* [x] add LR-FHSS TX (seperate driver based on [sx126x_driver](https://github.com/Lora-net/sx126x_driver))

## Pinout

## Nucleo64 on RAK5146
| Pin	| Func		| Wire		| F446RE Riot board | 
| ------- | ------  | --------- | ----------------- |


## Nucleo32 on RAK5146

| Pin	| Func		| Wire		| L432KC Riot board | 
| ----- | --------- | --------- | ----------------- |
| 4		| 5V		| Red		| 5V 		|
| 6		| GND		| Blue		| GND 		|
| 8		| URXD		| White1	| D1 rxtx 1 (PA10) |
| 10	| UTXD		| Grey		| D0 rxtx 1(PA9) |
| 22	| RESET_GPS	| ----		|       	|
| 24	| SPI_CE	| White2	| A2 (PA3) 	|
| 26	| GPIOSX1302| Brown		| A0 (PA0) 	|
| 32	| STDBY_GPS	| ----		| 			|
| 9		| GND		|			| 			|
| 11	| RESET		| Purple	| A1 (PA1) 	|
| 19	| SPI_MOSI	| Green		| D11 (PB5) |
| 21	| SPI_MISO	| Yellow	| D12 (PB4) |
| 23	| SPI_CLK	| Orange	| D13 (PB3) |


