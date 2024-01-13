/*
 * sx1302_pinmapping.h
 *
 *  Created on: 6 juin 2023
 *      Author: donsez
 */

#ifndef _INCLUDE_SX1302_PINMAPPING_H_
#define _INCLUDE_SX1302_PINMAPPING_H_

/* -------------------------------------------------------------------------- */
/* --- SX1302 PIN MAP ------------------------------------------------------- */

#if CORECELL_ON_NUCLEO == 1

// Pins are adapted to STM32 Nucleo-64 boards and ESP32 Wroom 32U/D
#include "arduino_board.h"

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	ARDUINO_PIN_10
#endif

#ifndef SX1302_PARAM_RESET_PIN
#if BOARD==esp32-wroom-32
#define SX1302_PARAM_RESET_PIN                  ARDUINO_PIN_2
#else
#define SX1302_PARAM_RESET_PIN                  ARDUINO_PIN_9
#endif
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               ARDUINO_PIN_8
#endif

#else

// TODO add test

// SX1302 transceiver on thingsat-f4
// branch stess_driver_sx1302  https://github.com/pi76r/RIOT/tree/stess_driver_sx1302 commit: ed9171014b7c236bb93c67da120183d89a4364ca

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    (SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                GPIO_PIN(PORT_A,4)
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN              GPIO_PIN(PORT_C,5)
#endif

// No GPIO for controlling the Corecell power on the thingsat-f4 board
#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN           SX1302_PARAM_POWER_EN
#endif

#endif


#endif /* _INCLUDE_SX1302_PINMAPPING_H_ */
