/*
 * endpoints.c
 *
 *  Created on: Jun 7, 2025
 *      Author: donsez
 */

#include <stdio.h>
#include "endpoints.h"

#define DEFAULT_NWKSKEY 	{0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78,0xBA,0xBE,0xCA,0xFE,0x12,0x34,0x56,0x78}
#define DEFAULT_APPSKEY 	{0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78,0xCA,0xFE,0xBA,0xBE,0x12,0x34,0x56,0x78}


static lgw_sx130x_endpoint_t lgw_sx130x_endpoints[] = {
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af5e,
				.gweui = 0x0016c001f136e359,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00ac92,
				.gweui = 0x0016C001F136EAC8,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
		{
				.module = RAK5146_SPI_GNSS,
				.devaddr = 0xfc00af2b,
				.gweui = 0x0016c001f136effd,
				// BABECAFE12345678BABECAFE12345678
				.nwkskey = DEFAULT_NWKSKEY,
				// CAFEBABE12345678CAFEBABE12345678
				.appskey = DEFAULT_APPSKEY,
		},
};

lgw_sx130x_endpoint_t *lgw_sx130x_endpoint = NULL;

#include "loragw_hal.h"
#include "endpoints.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

bool set_endpoint(void) {

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return false;
	}

	uint64_t eui;
	int err = lgw_get_eui(&eui);
	if (err != 0) {
		printf("ERROR: failed to get SX130x EUI\n");
		return false;
	}
	lgw_sx130x_endpoint = NULL;

	for (unsigned int i = 0; i < ARRAY_SIZE(lgw_sx130x_endpoints); i++) {
		if (eui == lgw_sx130x_endpoints[i].gweui) {
			lgw_sx130x_endpoint = lgw_sx130x_endpoints + i;
		}
	}

	if (lgw_sx130x_endpoint == NULL) {
		printf("ERROR: no endpoint defined for SX130x EUI\n");
		return false;
	} else {
		printf("INFO: endpoint with devaddr=%08lx defined for SX130x EUI\n",
				lgw_sx130x_endpoint->devaddr);
	}

	return true;
}


