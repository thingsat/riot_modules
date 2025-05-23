/*
 * Copyright (C) 2017 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_common_nucleo32 STM32 Nucleo-32
 * @ingroup     boards
 * @brief       Support for STM32 Nucleo-32 boards
 * @{
 *
 * @file
 * @brief       Common pin definitions and board configuration options
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef BOARD_H
#define BOARD_H

#include "board_nucleo.h"
#include "arduino_pinmap.h"
#include "periph_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Macros for controlling the on-board LED (LD3).
 * @{
 */
#define LED0_PIN_NUM        3
#define LED0_PORT           GPIO_PORT_B /**< GPIO port of LED 0 */
#define LED0_PORT_NUM       PORT_B


// Chip select Mikrobus Slot
#define CS_MK   GPIO_PIN(PORT_A,5) // A5
// Chip select RAK5134 miniPCI-e
#define CS_RAK  GPIO_PIN(PORT_B,1) // B1


// nucleo L432KC SPI #3 (Lora Core[RAK5146] : SX1303)
// REMARK : leave SX1302_ in define even if the component is a SX1303
#define SX1302_PARAM_SPI             (SPI_DEV(0))
#define SX1302_PARAM_SPI_NSS         CS_RAK
#define SX1302_PARAM_SPI_CLK_SPEED	 SPI_CLK_5MHZ

#define SX1302_PARAM_RESET_PIN       GPIO_PIN(PORT_B,0)


// nucleo L432KC SPI #3 (Mikrobus[LAMBDA80] : SX1280)

#ifndef SX1280_PARAM_SPI
#define SX1280_PARAM_SPI                    SPI_DEV(0)      /**< default SPI device */
#endif

#ifndef SX1280_PARAM_SPI_CLK
#define SX1280_PARAM_SPI_CLK                SPI_CLK_5MHZ    /**< default SPI speed */
#endif

#ifndef SX1280_PARAM_SPI_MODE
#define SX1280_PARAM_SPI_MODE               SPI_MODE_0      /**< default SPI mode for sx1280 */
#endif

#ifndef SX1280_PARAM_SPI_NSS
#define SX1280_PARAM_SPI_NSS                CS_MK           /**< SPI NSS pin */
#endif

#ifndef SX1280_PARAM_RESET
#define SX1280_PARAM_RESET                  GPIO_UNDEF      /**< Reset pin */
#endif

#ifndef SX1280_PARAM_DIO0
#define SX1280_PARAM_DIO0                   GPIO_UNDEF      /**< DIO0 */
#endif

#ifndef SX1280_PARAM_DIO1
#define SX1280_PARAM_DIO1                   GPIO_PIN(PORT_B, 8) /**< DIO1 */
#endif

#ifndef MCP9808_I2C_ADDRESS
// On board temperature sensor
// https://ww1.microchip.com/downloads/en/DeviceDoc/25095A.pdf
#define MCP9808_I2C_ADDRESS                 (0x18)
#endif



#ifndef CAN_RX_PIN
// Nano Pin D10
#define CAN_RX_PIN                 ARDUINO_PIN_10
#endif
#ifndef CAN_TX_PIN
// Nano Pin D2
#define CAN_TX_PIN                 ARDUINO_PIN_2
#endif




/** @} */

#ifdef __cplusplus
}
#endif

#include "stm32_leds.h"

#endif /* BOARD_H */
/** @} */
