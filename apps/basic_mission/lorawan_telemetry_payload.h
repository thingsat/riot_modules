/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
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

	/**
	 * @brief us counter at TX
	 */
	uint32_t		tx_uscount;

	/**
	 * @brief Status
	 * 1 for GPS_FIX, 2 for FTIME, TxMode
	 */
	common_status_t		status;

	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	uint8_t		txpower;

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
