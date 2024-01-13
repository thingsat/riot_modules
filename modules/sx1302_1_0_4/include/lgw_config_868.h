/*
 SX1302 LGW commands
 Copyright (c) 2021-2022 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LGW_CONFIG_868_H
#define _LGW_CONFIG_868_H


#ifndef LGW_CONFIG_FOR_LORAWAN_EU868_WITH_SX1250

#if ENABLE_SX1250 == 1
//for SX1250 on EU868 band
#define RSSI_OFFSET			(-215.4)
#define RSSI_TCOMP_COEFF_A	(0)
#define RSSI_TCOMP_COEFF_B	(0)
#define RSSI_TCOMP_COEFF_C	(20.41)
#define RSSI_TCOMP_COEFF_D	(2162.56)
#define RSSI_TCOMP_COEFF_E	(0)

#endif
#if ENABLE_SX125X == 1
	#error "Can not configure radio for RX according to global_conf.json.sx1257.EU868 parameters"
#endif

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


#define LGW_CONFIG_FOR_LORAWAN_EU868_WITH_SX1250 \
{ \
	true, \
	867500000UL, \
	868500000UL, \
	{ \
			-400000, \
			-200000, \
			0,       \
			-400000, \
			-200000, \
			0,       \
			200000,  \
			400000,  \
			-200000  \
			}, \
	{ 1, 1, 1, 0, 0, 0, 0, 0, 1 }, \
	{0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1}, \
	{15,16,17,19,20,22,1,2,3,4,5,6,7,9,11,14}, \
	RSSI_OFFSET, \
	RSSI_TCOMP_COEFF_A, \
	RSSI_TCOMP_COEFF_B, \
	RSSI_TCOMP_COEFF_C, \
	RSSI_TCOMP_COEFF_D, \
	RSSI_TCOMP_COEFF_E \
}


#endif

#endif
