/*
 SX1302/SX1303 LGW commands
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * This file contains the identity of SX1302/3 boards used for testing
 */

#ifndef TESTS_DRIVER_SX1302_ENDPOINTS_H_
#define TESTS_DRIVER_SX1302_ENDPOINTS_H_

#define DEFAULT_NWKSKEY 	{0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78,0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78}
#define DEFAULT_APPSKEY 	{0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78,0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78}

#include "lgw_endpoint.h"

lorawan_endpoint_t lgw_sx130x_endpoints[] = {
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af5e,
				.deveui = 0x0016c001f136e359,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ac92,
				.deveui = 0x0016C001F136EAC8,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af2b,
				.deveui = 0x0016c001f136effd,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
};


lorawan_endpoint_t *lgw_sx130x_endpoint = NULL;

#endif /* TESTS_DRIVER_SX1302_ENDPOINTS_H_ */
