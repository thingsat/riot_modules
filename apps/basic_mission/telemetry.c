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
#include "parse_nmea.h"

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
		 * @brief us counter at TX
		 */
		const uint32_t		tx_uscount,
		/**
		 * @brief us counter at the previous ranging message TX (fcntup - 1)
		 */
		const uint32_t		tx_uscount_prev,
		/**
		 * @brief Ranging Status
		 * 1 for GPS_FIX, 2 for FTIME, TxMode ()
		 */
		const uint8_t		status,
		/**
		 * @brief Tx Power in dBm
		 * Useful for ADR
		 */
		const uint8_t		txpower
) {

	(void)tx_uscount_prev;

	GNSSTelemetryPayload_t* telemetry_payload = (GNSSTelemetryPayload_t*) fpayload;
	*fpayload_size = sizeof(GNSSTelemetryPayload_t);
	// reset buffer
	memset(telemetry_payload,0,*fpayload_size);

	telemetry_payload->tx_uscount = tx_uscount;
	telemetry_payload->status = status;
	telemetry_payload->txpower = txpower;

	// Fill location
	common_location_t* location = &telemetry_payload->location;
	common_location_extra_t* location_extra = &telemetry_payload->location_extra;

	// Il faut changer le decoder.js
	location_extra->seconds_since_last_fix = gps_get_seconds_since_last_fix();

	float latitude, longitude, altitude;
	gps_get_position(&latitude, &longitude, &altitude);

	location->latitude = latitude;
	location->longitude = longitude;
	location->altitude = floor(altitude);

	float speed_kph;
	float true_track_degrees;

	if(gps_get_speed_direction(&speed_kph, &true_track_degrees)){
		location_extra->speed_kph = floor(speed_kph);
		location_extra->true_track_degrees =  floor(true_track_degrees);
	}

	int fix_quality;
	int satellites_tracked;

	if(gps_get_quality(&fix_quality, &satellites_tracked)) {
		location_extra->satellites_tracked = (uint8_t)satellites_tracked;
		location_extra->fix_quality = (uint8_t)fix_quality;
		if(fix_quality==0) {
			location->latitude = 0.0;
			location->longitude = 0.0;
			location->altitude = 0;
			location_extra->speed_kph = 0;
			location_extra->true_track_degrees = 0;
		}

	} else {
		location->latitude = 0.0;
		location->longitude = 0.0;
		location->altitude = 0;
		location_extra->speed_kph = 0;
		location_extra->true_track_degrees = 0;
		location_extra->fix_quality = 0;
		location_extra->satellites_tracked = 0;
	}
	return true;
}
