/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include "endpoints.h"


#if PROD == 1
#include "endpoints_prod.inc"
#else
#include "endpoints_dev.inc"
#endif

lorawan_endpoint_t *lgw_sx130x_endpoint = NULL;

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

	uint64_t gweui;
	int err = lgw_get_eui(&gweui);
	if (err != 0) {
		printf("ERROR: failed to get SX130x EUI\n");
		return false;
	}
	lgw_sx130x_endpoint = NULL;

	for (unsigned int i = 0; i < ARRAY_SIZE(lorawan_endpoints); i++) {
		if (gweui == lorawan_endpoints[i].deveui) {
			lgw_sx130x_endpoint = lorawan_endpoints + i;
		}
	}

	if (lgw_sx130x_endpoint == NULL) {
		printf("ERROR: no endpoint defined for SX130x EUI\n");
		return false;
	} else {
		printf("INFO: endpoint with devaddr=%08lx \"%s\" defined for SX130x EUI (%08lX%08lX)\n",
				lgw_sx130x_endpoint->devaddr, lgw_sx130x_endpoint->label,
				(uint32_t)(lgw_sx130x_endpoint->deveui>>32),
				(uint32_t)(lgw_sx130x_endpoint->deveui&0xFFFFFFFF)
				);
		printf("INFO: initial fcntup=%ld\n",
				lgw_sx130x_endpoint->fcntup);

	}

	return true;
}


const lorawan_endpoint_t* endpoint_get_endpoint(const uint32_t devaddr) {

	for (unsigned int i = 0; i < ARRAY_SIZE(lorawan_endpoints); i++) {
		if (devaddr == lorawan_endpoints[i].devaddr) {
			return lorawan_endpoints + i;
		}
	}
	return NULL;
}

