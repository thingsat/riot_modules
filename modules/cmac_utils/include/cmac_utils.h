/*
 * Copyright (C) 2020-2021 Université Grenoble Alpes
 */

/*
 * Utils for the MIC control
 *
 * @Remark: the MIC is similar to the LoRaWAN frames MIC (which is based on CMAC).
 *
 */

#ifndef _INCLUDE_CMAC_UTILS_H
#define _INCLUDE_CMAC_UTILS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#define CMAC_KEY_LEN			(16U)

typedef uint32_t				mic_t;

/*
 * @brief Compute the MIC
 */
void Cmac_calculate_mic(const uint8_t *buf, const uint8_t *key, const size_t buf_len, mic_t *mic);

/*
 * @brief Check the MIC
 */
bool Cmac_check_mic(const uint8_t *buf, const uint8_t *key, const size_t buf_len, const mic_t mic_to_check);

#endif
