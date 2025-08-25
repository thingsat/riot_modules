/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */

/*
 * Thingsat Mission Scenarii :: Data structures
 */

/*
 * Types for the format of the payload into the LoRaWAN frames sent by the Thingsat payload
 */


#ifndef _LORAWAN_TELEMETRY_PAYLOAD_H
#define _LORAWAN_TELEMETRY_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>

/*
 * FPort definition for typing the payload format
 */

#include "lorawan_common_payload.h"

#define FPORT_GNSS_TELEMETRY_PAYLOAD	(5U)

/**
 * Struct for Telemetry message
 */
struct __attribute__((__packed__)) GNSSTelemetryPayload {


	common_tx_t tx;

	/**
	 * @brief Location
	 */
	common_location_t location;

	/**
	 * @brief Location
	 */
	common_location_extra_t location_extra;


	// TODO add sensors
};

typedef struct GNSSTelemetryPayload GNSSTelemetryPayload_t;

#endif // _LORAWAN_TELEMETRY_PAYLOAD_H
