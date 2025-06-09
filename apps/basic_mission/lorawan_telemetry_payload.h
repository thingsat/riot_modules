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

#define FPORT_GNSS_TELEMETRY_PAYLOAD	(5U)

#define RANGING_GNSS_STATUS_FIX			0b10000000
#define RANGING_GNSS_STATUS_FTIME		0b01000000
#define RANGING_TX_MODE_STATUS_MASK		0b00000011


/**
 * Struct for GNSS telemetry location (mainly for HA balloon experiments)
 */
struct __attribute__((__packed__)) gnss_telemetry_location {

	// TODO add GPS Fix last time (in seconds), GPS quality, GPS satellite number ...

	/**
	 * @brief Latitude	(in degree)
	 * optimization on int24 with gps_get_latitude_to_i24(latitude)
	 */
	//uint32_t latitude:24;
	float latitude;

	/**
	 * @brief Longitude (in degree)
	 * optimization on int24 with gps_get_longitude_to_i24(longitude)
	 */
	//uint32_t longitude:24;
	float longitude;

	/**
	 * @brief Altitude
	 */
	uint16_t altitude;

	/**
	 * @brief Seconds since last fix
	 */
	uint16_t seconds_since_last_fix;

	/**
	 * @brief Tracked satellites
	 */
	uint8_t satellites_tracked;

	/**
	 * @brief Fix quality
	 */
	uint8_t fix_quality;

	/**
	 * @brief Speed in kph
	 */
	uint16_t speed_kph;

	/**
	 * @brief Speed in kph
	 */
	int16_t true_track_degrees;

};

typedef struct gnss_telemetry_location gnss_telemetry_location_t;


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
	uint8_t		status;

	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	uint8_t		txpower;

	/**
	 * @brief Location
	 */
	gnss_telemetry_location_t location;


	// TODO add sensors
};

typedef struct GNSSTelemetryPayload GNSSTelemetryPayload_t;

#endif // _LORAWAN_TELEMETRY_PAYLOAD_H
