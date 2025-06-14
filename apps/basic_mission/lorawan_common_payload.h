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
 * Types for the format of the payload into the LoRaWAN frames sent and received by the Thingsat payload
 */


#ifndef _LORAWAN_COMMON_PAYLOAD_H
#define _LORAWAN_COMMON_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>


// Field for status
#define COMMON_GNSS_STATUS_FIX			0b10000000
#define COMMON_GNSS_STATUS_FTIME		0b01000000
#define COMMON_TX_MODE_STATUS_MASK		0b00000011

/**
 * @brief Status for GNSS and Tx Mode
 * 1 for GPS_FIX, 2 for FTIME
 * TxMode (IMMEDIATE=0,TIMESTAMPED=1,ON_GPS=2)
 *
 * TODO change for struct __attribute__((__packed__)) common_status
 *
 */

typedef uint8_t common_status_t;

struct __attribute__((__packed__)) common_status_s {
	uint8_t		tx_mode:2; 	// lgw tx mode IMMEDIATE=0,TIMESTAMPED=1,ON_GPS=2
	uint8_t		rfu:4;
	uint8_t		ftime:1;	// flag for fine timestamping
	uint8_t		fix:1;		// flag for GNSS fix
};

typedef struct common_status_s common_status_s_t;

/**
 * Struct for time fields
 */
struct __attribute__((__packed__)) common_time {

	/**
	 * @brief Uptime (in seconds since boot)
	 * optimization on uint24
	 */
	uint32_t uptime;

	/**
	 * @brief local_time (RTC in seconds since 1/1/1970)
	 */
	uint32_t localtime;
};

typedef struct common_time common_time_t;


/**
 * Struct for location (mainly for HA balloon experiments)
 */
struct __attribute__((__packed__)) common_location {


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
};

typedef struct common_location common_location_t;


/**
 * Struct for optimized location
 */
struct __attribute__((__packed__)) common_packed_location {

	/**
	 * @brief Latitude	(in degree)
	 * optimization on int24 with pack_coord(latitude,longitude)
	 */
	uint32_t latitude:24;

	/**
	 * @brief Longitude (in degree)
	 * optimization on int24 with pack_coord()
	 */
	uint32_t longitude:24;

	/**
	 * @brief Altitude (in 10 meters)
	 */
	uint16_t altitude;
};

typedef struct common_packed_location common_packed_location_t;

/**
 * Struct for extra fields related to location (mainly for HA balloon experiments)
 */
struct __attribute__((__packed__)) common_location_extra {

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

typedef struct common_location_extra common_location_extra_t;

// TODO add struct for HDOP, VDOP, GDOP (as uint8_t)

/**
 * Struct for sensors
 */
struct __attribute__((__packed__)) common_sensors {

	// TODO VBAT and TS https://community.st.com/t5/stm32-mcus-products/adc-is-weird-too/td-p/120816

	uint8_t vbat;

	uint8_t ts;
};

typedef struct common_sensors common_sensors_t;


#endif // _LORAWAN_COMMON_PAYLOAD_H
