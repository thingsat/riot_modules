/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/**
 * @file
 * @brief       shell commands for using the SX1280 as a LoRa 2.4 GHz modem.
 *
 * @author      Nicolas Albarel
 * @author      Didier Donsez
 * @author      Olivier Alphand
 */

#ifndef SX1280_CMD_H
#define SX1280_CMD_H

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "ral_defs.h"

#ifndef DELAY_BETWEEN_TX
#define DELAY_BETWEEN_TX                    (2000000U)
#endif

#ifndef SECONDS_IF_INIT_FAILED
#define SECONDS_IF_INIT_FAILED              (5U)
#endif


/*
 * @brief Initialize the SX1280 driver
 */
int sx1280_init(void);

/*
 * @brief Initialize the SX1280 driver and reboot on initialization failure
 */
void sx1280_init_and_reboot_on_failure(void);


/*
 * @brief Lock the mutex that protect low level access to sx1280 (useful when calling the driver directly)
 */
void sx1280_lock(void);

/*
 * @brief Unlock the mutex that protect low level access to sx1280 (useful when calling the driver directly)
 */
void sx1280_unlock(void);


/*
 * @brief The LoRa modulation parameters for Rx and Tx
 */
ral_params_lora_t * sx1280_get_params(void);

/*
 * @brief Get the frequency in Hz from the driver lora definition
 */
uint32_t sx1280_getBW(ral_lora_bw_t lora_bw);


/*
 * The `sx1280` shell command
 */
int sx1280_cmd(int argc, char **argv);


#endif
