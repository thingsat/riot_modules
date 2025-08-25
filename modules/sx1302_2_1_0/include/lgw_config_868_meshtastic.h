/*
 SX1302/SX1303 Driver (2.1.0)
 Copyright (c) 2021-2024 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LGW_CONFIG_868_MESHTASTIC_H
#define _LGW_CONFIG_868_MESHTASTIC_H

#include "config.h"

#ifndef LGW_CONFIG_FOR_LORAWAN_EU868_WITH_SX1250

// Default configuration for LoRaWAN EU868 with SX1250

/*
static lgw_config_t _lorawan_lgw_config = {
	// single_input_mode
	true,
	// fa
	867500000UL, // Radio A
	// fb
	868500000UL, // Radio B
	// channel_if
	{
			-400000, // Radio B : 868.1MHz
			-200000, // Radio B : 868.3MHz
			0,       // Radio B : 868.5MHz
			-400000, // Radio A : 867.1MHz
			-200000, // Radio A : 867.3MHz
			0,       // Radio A : 867.5MHz
			200000,  // Radio A : 867.7MHz
			400000,  // Radio A : 867.9MHz
			-200000  // Radio B : 868.3MHz (lora service)
			},
	// channel_rfchain
	{ 1, 1, 1, 0, 0, 0, 0, 0, 1 },

	 // Front-end power amplifier enable (1) or disable (0)
	 // From global_conf.json.sx1250.EU868
	 // rfpower: RF power in dBm (12 .. 27).
	 // pa_gain
	{0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
	// pwr_idx
	{15,16,17,19,20,22,1,2,3,4,5,6,7,9,11,14},

	//for SX1250 on EU868 band
	RSSI_OFFSET,
	RSSI_TCOMP_COEFF_A,
	RSSI_TCOMP_COEFF_B,
	RSSI_TCOMP_COEFF_C,
	RSSI_TCOMP_COEFF_D,
	RSSI_TCOMP_COEFF_E
};
 */

#if ENABLE_SX1250 == 1
// From https://github.com/Lora-net/sx1302_hal/blob/master/packet_forwarder/global_conf.json.sx1250.EU868
// Configuration for SX1250 on EU868 band

#define RSSI_OFFSET			(-215.4)
#define RSSI_TCOMP_COEFF_A	(0)
#define RSSI_TCOMP_COEFF_B	(0)
#define RSSI_TCOMP_COEFF_C	(20.41)
#define RSSI_TCOMP_COEFF_D	(2162.56)
#define RSSI_TCOMP_COEFF_E	(0)


#ifndef LGW_PA_GAIN
#define LGW_PA_GAIN 0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1
#endif

#ifndef LGW_PWR_IDX
#define LGW_PWR_IDX 15,16,17,19,20,22,1,2,3,4,5,6,7,9,11,14
#endif

/*
https://meshtastic.org/docs/overview/radio-settings/#europe-frequency-bands
Long Moderate SF11 BW125
Slot 1 : 869.462500
Slot 2 : 869.587500
*/

#define LGW_CONFIG_FOR_LORAWAN_EU868_WITH_SX1250 \
{ \
	.single_input_mode = true, \
	.fa = 867500000UL, \
	.fb = 869587500UL, \
	.channel_if = { \
			-250000, \
			-125000, \
			0,       \
			-400000, \
			-200000, \
			0,       \
			200000,  \
			400000,  \
			-62500, \
			-62500 \
			}, \
	.channel_rfchain = { 1, 1, 1, 0, 0, 0, 0, 0, 1, 1 }, \
	.pa_gain = {LGW_PA_GAIN}, \
	.pwr_idx = {LGW_PWR_IDX}, \
	.rssi_offset = RSSI_OFFSET, \
	.rssi_tcomp_coeff_a = RSSI_TCOMP_COEFF_A, \
	.rssi_tcomp_coeff_b = RSSI_TCOMP_COEFF_B, \
	.rssi_tcomp_coeff_c = RSSI_TCOMP_COEFF_C, \
	.rssi_tcomp_coeff_d = RSSI_TCOMP_COEFF_D, \
	.rssi_tcomp_coeff_e = RSSI_TCOMP_COEFF_E, \
	.lorastd_enable = true, \
	.lorastd_bw = 250000, \
	.lorastd_datarate = 7, \
	.lorastd_implicit_hdr = false, \
	.lorastd_implicit_payload_length = 17, \
	.lorastd_implicit_crc_en = false, \
	.lorastd_implicit_coderate = 1, \
	.fsk_enable = true, \
	.fsk_bw = 125000, \
	.fsk_datarate = 50000, \
	.fsk_fdev = 0 \
}

#endif

#if ENABLE_SX125X == 1
	#error "Can not configure radio for RX according to global_conf.json.sx1257.EU868 parameters"
#endif


#endif

#endif
