/*
 SX1302 LGW commands
 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#define ENABLE_DEBUG 1
#include "debug.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lgw_config.h"
#include "lgw_config_868.h"

/**
 * Default configuration for LoRaWAN EU868 with SX1250
 */
static lgw_config_t _lorawan_lgw_config = LGW_CONFIG_FOR_LORAWAN_EU868_WITH_SX1250;

static lgw_config_t* _lgw_config = &_lorawan_lgw_config;

void lgw_set_lgw_config(lgw_config_t *lgw_config){
	_lgw_config = lgw_config;
}

lgw_config_t* lgw_get_lgw_config(void){
	return _lgw_config;
}

uint8_t lgw_get_pa_gain(uint8_t rf_power) {
	assert(rf_power >= 12 && rf_power <= 27);
	return _lgw_config->pa_gain[rf_power-12];
}

uint8_t lgw_get_pwr_idx(uint8_t rf_power) {
	assert(rf_power >= 12 && rf_power <= 27);
	return _lgw_config->pwr_idx[rf_power-12];
}

void lgw_get_freq_hz_plan(uint32_t* freq_hz_plan) {
	for (int i = 0; i < 8; i++) {
		freq_hz_plan[i] = _lgw_config->channel_rfchain[i] ?
						(double) (_lgw_config->fb + _lgw_config->channel_if[i]) :
						(double) (_lgw_config->fa + _lgw_config->channel_if[i]);
	}
}
