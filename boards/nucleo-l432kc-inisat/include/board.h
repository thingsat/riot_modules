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


#define SX1280_PARAM_SPI_NSS                GPIO_PIN(PORT_B, 4)  /**< SPI NSS pin D12 */


#define SX1280_PARAM_RESET                  GPIO_PIN(PORT_B, 5)  /**< Reset pin D11 */


#define SX1280_PARAM_DIO0                   GPIO_PIN(PORT_A, 4) /**< DIO0 A3 */

#define SX1280_PARAM_DIO1                   GPIO_UNDEF


/** @} */

#ifdef __cplusplus
}
#endif

#include "stm32_leds.h"

#endif /* BOARD_H */
/** @} */
