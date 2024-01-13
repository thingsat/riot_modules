/*
    SX1302 RF test firmware
    Copyright (c) 2021 UGA CSUG LIG

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.

    Port to RIOT by Didier DONSEZ & Olivier ALPHAND, Université Grenoble-Alpes

*/

#include "config.h"
#define ENABLE_DEBUG (DEBUG_SX1302)
#include "debug.h"

#include "periph/gpio.h"
#include "xtimer.h"

#include "loragw_sx1302.h"
#include "loragw_sx1302.h"
#include "sx1302_pinmapping.h"


#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	LGW_HAL_SUCCESS
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE	LGW_HAL_ERROR
#endif

#define DELAY_FOR_GPIO	(100000)

int sx1302_reset(void)
{

#if SX1302_PARAM_POWER_EN_PIN == GPIO_UNDEF
	printf("Set reset (no GPIO for power)\n");
	DEBUG("[sx1302_reset] set reset (no GPIO for power)\n");
    gpio_init(SX1302_PARAM_RESET_PIN, GPIO_OUT);     // SX1302_PARAM_RESET_PIN
    xtimer_usleep(DELAY_FOR_GPIO);
#else
    printf("Set power and reset\n");
    DEBUG("[sx1302_reset] set power and reset\n");
    gpio_init(SX1302_PARAM_POWER_EN_PIN, GPIO_OUT);  // SX1302_PARAM_POWER_EN_PIN
    gpio_init(SX1302_PARAM_RESET_PIN, GPIO_OUT);     // SX1302_PARAM_RESET_PIN
    xtimer_usleep(DELAY_FOR_GPIO);

    // write output for SX1302 CoreCell power_disable
    gpio_clear(SX1302_PARAM_POWER_EN_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);

    // write output for SX1302 CoreCell power_enable
    gpio_set(SX1302_PARAM_POWER_EN_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);
#endif
    // write output for SX1302 CoreCell reset
    gpio_clear(SX1302_PARAM_RESET_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);
    gpio_set(SX1302_PARAM_RESET_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);
    gpio_clear(SX1302_PARAM_RESET_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);

    return EXIT_SUCCESS;
}

int sx1302_poweroff(void)
{
#if SX1302_PARAM_POWER_EN_PIN != GPIO_UNDEF
	printf("Disable power\n");
    DEBUG("[sx1302_poweroff] disable power\n");
    gpio_init(SX1302_PARAM_POWER_EN_PIN, GPIO_OUT);  // SX1302_PARAM_POWER_EN_PIN
    // write output for SX1302 CoreCell power_disable
    gpio_clear(SX1302_PARAM_POWER_EN_PIN);
    xtimer_usleep(DELAY_FOR_GPIO);
#endif
    return EXIT_SUCCESS;
}

