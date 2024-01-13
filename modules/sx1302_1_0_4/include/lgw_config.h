/*
 SX1302 LGW commands
 Copyright (c) 2021-2022 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LGW_CONFIG_H
#define _LGW_CONFIG_H

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//#if ENABLE_SX1250 == 1
////for SX1250 on EU868 band
//#define RSSI_OFFSET			(-215.4)
//#define RSSI_TCOMP_COEFF_A	(0)
//#define RSSI_TCOMP_COEFF_B	(0)
//#define RSSI_TCOMP_COEFF_C	(20.41)
//#define RSSI_TCOMP_COEFF_D	(2162.56)
//#define RSSI_TCOMP_COEFF_E	(0)
//
//#endif
//#if ENABLE_SX125X == 1
//	#error "Can not configure radio for RX according to global_conf.json.sx1257.EU868 parameters"
//#endif
//

struct lgw_config {

	// TODO what is single_input_mode ?
	bool single_input_mode;

	uint32_t fa; // Frequency Radio A
	uint32_t fb; // Frequency Radio B

	int32_t channel_if[9];
	uint8_t channel_rfchain[9];

	uint8_t pa_gain[27-12+1];
	uint8_t pwr_idx[27-12+1];

	float rssi_offset;
	float rssi_tcomp_coeff_a;
	float rssi_tcomp_coeff_b;
	float rssi_tcomp_coeff_c;
	float rssi_tcomp_coeff_d;
	float rssi_tcomp_coeff_e;

};

typedef struct lgw_config lgw_config_t;

void lgw_set_lgw_config(lgw_config_t *lgw_config);

lgw_config_t* lgw_get_lgw_config(void);

uint8_t lgw_get_pa_gain(uint8_t rf_power);

uint8_t lgw_get_pwr_idx(uint8_t rf_power);

//void lgw_get_freq_hz_plan(uint32_t* freq_hz_plan);

#endif //_LGW_CONFIG_H
