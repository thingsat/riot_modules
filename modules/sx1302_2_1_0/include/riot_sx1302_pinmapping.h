/*
 SX1302/3 v2.1.0 Driver Pin Mapping

 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _INCLUDE_RIOT_SX1302_PINMAPPING_H_
#define _INCLUDE_RIOT_SX1302_PINMAPPING_H_

/* -------------------------------------------------------------------------- */
/* --- SX1302/3 PIN MAP ------------------------------------------------------- */

#if defined(BOARD_THINGSAT_UP4_V2)

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    (SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                GPIO_PIN(PORT_A,4)
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN              GPIO_PIN(PORT_C,5)
#endif

// GPIO for controlling the power of the SX1303 on the thingsat-f4 board
#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN           GPIO_PIN(PORT_B,15)
#endif

#elif defined(BOARD_THINGSAT_UP4)

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    (SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                GPIO_PIN(PORT_A 4)
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN              GPIO_PIN(PORT_C,5)
#endif

// GPIO for controlling the power of the SX1303 on the thingsat-f4 board
#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN           GPIO_PIN(PORT_B,15)
#endif

#elif defined(BOARD_THINGSAT_UP1_F4)

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

// Note: No GPIO for controlling the Corecell power on the thingsat-f4 board
#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN           GPIO_UNDEF
#endif

#elif RAK5146_ON_NUCLEO == 1

#include "arduino_pinmap.h"

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	ARDUINO_PIN_10
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN                  ARDUINO_PIN_9
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               ARDUINO_PIN_7
#endif

#elif RAK5146_ON_NUCLEO32 == 1

//#include "arduino_board.h"

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	GPIO_PIN(PORT_A,3)
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN                  GPIO_PIN(PORT_A,1)
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               GPIO_PIN(PORT_A,0)
#endif

#elif RAK5146_ON_INISAT == 1

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	GPIO_PIN(PORT_B,4) // D9
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN                  GPIO_PIN(PORT_B,5) // D11
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               GPIO_PIN(PORT_A,3) // A2
#endif


#elif CORECELL_ON_NUCLEO == 1

// Pins are adapted to STM32 Nucleo-64 boards and ESP32 Wroom 32U/D

#include "arduino_pinmap.h"

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	ARDUINO_PIN_10
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN                  ARDUINO_PIN_9
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               ARDUINO_PIN_8
#endif

#elif BOARD == esp32-wroom-32

// Pins are adapted to STM32 Nucleo-64 boards and ESP32 Wroom 32U/D

#include "arduino_board.h"

#ifndef SX1302_PARAM_SPI
#define SX1302_PARAM_SPI                    	(SPI_DEV(0))
#endif

#ifndef SX1302_PARAM_SPI_NSS
#define SX1302_PARAM_SPI_NSS                	ARDUINO_PIN_10
#endif

#ifndef SX1302_PARAM_RESET_PIN
#define SX1302_PARAM_RESET_PIN                  ARDUINO_PIN_2
#endif

#ifndef SX1302_PARAM_POWER_EN_PIN
#define SX1302_PARAM_POWER_EN_PIN               ARDUINO_PIN_8
#endif

#elif BOARD == rpi-pico

#error SX1302/3 pin mapping for RPI Pico is not defined

#else

#error Missing SX1302/3 pin mapping

#endif


#endif /* _INCLUDE_RIOT_SX1302_PINMAPPING_H_ */
