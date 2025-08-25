/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>
#include <string.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_TELEMETRY
#include "debug.h"

#include "lorawan_telemetry_payload.h"
#include "common_payload.h"

bool telemetry_get_fpayload(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size,
		/**
		 * @brief Tx Power in dBm
		 * Useful for ADR
		 */
		const uint8_t txpower
) {

	GNSSTelemetryPayload_t* telemetry_payload = (GNSSTelemetryPayload_t*) fpayload;
	*fpayload_size = sizeof(GNSSTelemetryPayload_t);

	// reset buffer
	memset(telemetry_payload,0,*fpayload_size);

	common_set_common_tx(&telemetry_payload->tx, txpower);

	common_set_common_location(&telemetry_payload->location);

	common_set_common_location_extra(&telemetry_payload->location_extra);

	return true;
}
