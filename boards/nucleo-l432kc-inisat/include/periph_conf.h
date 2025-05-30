/*
 * Copyright (C) 2017  Inria
 *               2017  OTA keys
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     boards_nucleo-l432kc-inisat
 * @{
 *
 * @file
 * @brief       Peripheral MCU configuration for the nucleo-l432kc board for INISAT with RAK5146 LoRa Gateway module 
 * @see 		https://os.mbed.com/platforms/ST-Nucleo-L432KC/#arduino-nano-compatible-headers
 * @author      Didier Donsez <didier.donsez@univ-grenoble-alpes.fr>
 */

#ifndef PERIPH_CONF_H
#define PERIPH_CONF_H


/* Add specific clock configuration (HSE, LSE) for this board here */
#ifndef CONFIG_BOARD_HAS_LSE
#define CONFIG_BOARD_HAS_LSE            1
#endif

#include "periph_cpu.h"
#include "clk_conf.h"
#include "cfg_rtt_default.h"
#include "cfg_timer_tim2.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
	// Console (or OpenLog)
    {
        .dev        = USART2,
        .rcc_mask   = RCC_APB1ENR1_USART2EN,
        .rx_pin     = GPIO_PIN(PORT_A, 15), // Nano A2
        //.rx_pin     = GPIO_PIN(PORT_A, 3),  // Nano A2
        .tx_pin     = GPIO_PIN(PORT_A, 2),    // Nano A7
        .rx_af      = GPIO_AF3,
        .tx_af      = GPIO_AF7,
        .bus        = APB1,
        .irqn       = USART2_IRQn,
        .type       = STM32_USART,
        .clk_src    = 0, /* Use APB clock */
    },
	// MiniPCIe xor Mikrobus (according JP3 and JP4 jumpers)
	{
        .dev        = USART1,
        .rcc_mask   = RCC_APB2ENR_USART1EN,
        .rx_pin     = GPIO_PIN(PORT_B, 6),  // Nano D4
        .tx_pin     = GPIO_PIN(PORT_B, 7),	// Nano D5
        .rx_af      = GPIO_AF7,
        .tx_af      = GPIO_AF7,
        .bus        = APB2,
        .irqn       = USART1_IRQn,
        .type       = STM32_USART,
        .clk_src    = 0, /* Use APB clock */
    },
};

#define UART_0_ISR          (isr_usart2)
#define UART_1_ISR          (isr_usart1)

#define UART_NUMOF          ARRAY_SIZE(uart_config)
/** @} */

/**
 * @name   PWM configuration
 * @{
 */
static const pwm_conf_t pwm_config[] = {
    {
        .dev      = TIM1,
        .rcc_mask = RCC_APB2ENR_TIM1EN,
        .chan     = { { .pin = GPIO_PIN(PORT_A, 8) /* D9 */, .cc_chan = 0 },
                      { .pin = GPIO_UNDEF,                   .cc_chan = 0 },
                      { .pin = GPIO_UNDEF,                   .cc_chan = 0 },
                      { .pin = GPIO_UNDEF,                   .cc_chan = 0 } },
        .af       = GPIO_AF1,
        .bus      = APB2
    }
};

#define PWM_NUMOF           ARRAY_SIZE(pwm_config)
/** @} */

/**
 * @name   SPI configuration
 * @{
 */
static const spi_conf_t spi_config[] = {
    {
        .dev      = SPI3,
        .mosi_pin = GPIO_PIN(PORT_B, 5), // D11
        .miso_pin = GPIO_PIN(PORT_B, 4), // D12
        .sclk_pin = GPIO_PIN(PORT_B, 3), // D13
        .cs_pin   = GPIO_UNDEF,             // There is 2 Chip select so we need to manipulate them by software
        .mosi_af  = GPIO_AF6,
        .miso_af  = GPIO_AF6,
        .sclk_af  = GPIO_AF6,
        .rccmask  = RCC_APB1ENR1_SPI3EN,
        .apbbus   = APB1
    }
};

#define SPI_NUMOF           ARRAY_SIZE(spi_config)

/**
 * @name I2C configuration
 * @{
 */
static const i2c_conf_t i2c_config[] = {
    {
        .dev            = I2C1,
        .speed          = I2C_SPEED_NORMAL,
        .scl_pin        = GPIO_PIN(PORT_A, 9),
        .sda_pin        = GPIO_PIN(PORT_A, 10),
        .scl_af         = GPIO_AF4,
        .sda_af         = GPIO_AF4,
        .bus            = APB1,
        .rcc_mask       = RCC_APB1ENR1_I2C1EN,
        .rcc_sw_mask    = RCC_CCIPR_I2C1SEL_1,          /* HSI (16 MHz) */
        .irqn           = I2C1_ER_IRQn,
    }
};

#define I2C_0_ISR           isr_i2c1_er

#define I2C_NUMOF           ARRAY_SIZE(i2c_config)
/** @} */

#define RAK5146_ON_NUCLEO32 1
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H */
/** @} */
