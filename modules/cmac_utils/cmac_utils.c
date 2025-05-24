/*
 * Copyright (C) 2020-2021 Université Grenoble Alpes
 */

/*
 * Utils for the MIC control
 */

#include <stdlib.h>
#include <string.h>

#include "hashes/aes128_cmac.h"

#include "cmac_utils.h"

static aes128_cmac_context_t CmacContext;

static uint8_t digest[CMAC_KEY_LEN];

void Cmac_calculate_mic(const uint8_t *buf, const uint8_t *key, const size_t buf_len, uint32_t *mic)
{
	aes128_cmac_init(&CmacContext, key, CMAC_KEY_LEN);
	aes128_cmac_update(&CmacContext, buf, buf_len);
	aes128_cmac_final(&CmacContext, digest);
    memcpy(mic, digest, sizeof(uint32_t));
}


bool Cmac_check_mic(const uint8_t *buf, const uint8_t *key, const size_t buf_len, const uint32_t mic_to_check)
{
	aes128_cmac_init(&CmacContext, key, CMAC_KEY_LEN);
	aes128_cmac_update(&CmacContext, buf, buf_len);
	aes128_cmac_final(&CmacContext, digest);
    return (memcmp(&mic_to_check, digest, sizeof(uint32_t)) == 0);
}
