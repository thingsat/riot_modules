/*
 SX1302 Driver
 Copyright (c) 2021-2022 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LORAGW_CONFIGURATION_H
#define _LORAGW_CONFIGURATION_H

#define LIBLORAGW_VERSION "2.1.0"
#define LIBLORAGW_RELEASE_DATE "Apr 22, 2021"
#define LIBLORAGW_RELEASE "https://github.com/Lora-net/sx1302_hal/releases/tag/V2.1.0"


#define DEBUG_AUX 				0
#define DEBUG_COM 				0 // SPI and USB
#define DEBUG_I2C 				0 
#define DEBUG_REG 				0
#define DEBUG_HAL 				0
#define DEBUG_GPS 				0
#define DEBUG_GPIO 				0
#define DEBUG_LBT 				0
#define DEBUG_RAD 				0
#define DEBUG_CAL 				0
#define DEBUG_SX1302 			0

#ifndef ENABLE_REGTEST
#define ENABLE_REGTEST			1
#endif

#ifndef ENABLE_MOD_FSK
#define ENABLE_MOD_FSK			1
#endif

#ifndef ENABLE_MOD_CW
#define ENABLE_MOD_CW			0
#endif

#define ENABLE_GPS				0
#define ENABLE_LBT				0
#define ENABLE_SX1261			0
#define ENABLE_STTS751			0
#define ENABLE_SAUL_TEMP_SENSOR	1
#define ENABLE_AD5338R			0
#define ENABLE_DEBUG_FILE_LOG	0


#ifndef ENABLE_SX125X
#define ENABLE_SX125X			0
#endif

#ifndef ENABLE_SX1250
#define ENABLE_SX1250			1
#endif

#endif //_LORAGW_CONFIGURATION_H
