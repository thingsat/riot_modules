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

#ifndef _LORAWAN_STAT_PAYLOAD_H
#define _LORAWAN_STAT_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>

#include "lorawan_common_payload.h"

/*
 * FPort definition for typing the payload format
 */

#define FPORT_STAT_PAYLOAD	(6U)

/**
 * Struct for Stat for Ranging
 */
struct __attribute__((__packed__)) stat_ranging {


	/**
	 * @brief counter of sent Range1 frames
	 */
	uint16_t range1_tx;

	/**
	 * @brief counter of received Range1 frames counter from friends (LSB of uint32_t since boot time)
	 */
	uint16_t range1_rx;

	/**
	 * @brief counter of received Range2 frames counter from friends (LSB of uint32_t since boot time)
	 */
	uint16_t range2_rx;

	/**
	 * @brief counter of received Range2 frames counter from friends (LSB of uint32_t since boot time)
	 */
	uint16_t range3_rx;

	/**
	 * @brief counter of received Range2 frames counter from friends for myself (LSB of uint32_t since boot time)
	 */
	uint16_t range2_rx_replies;


};

typedef struct stat_ranging stat_ranging_t;

/**
 * Struct for Stat for LGW
 */
struct __attribute__((__packed__)) stat_lgw {
	/**
	 * @brief counter of received frames (LSB of uint32_t since boot time)
	 */
	uint16_t rx;

	/**
	 * @brief counter of received frames with bad CRC (LSB of uint32_t since boot time)
	 */
	uint16_t rx_bad_crc;

	/**
	 * @brief counter of received frames from friends (LSB of uint32_t since boot time)
	 */
	uint16_t rx_friends;

	/**
	 * @brief counter of received frames from friends with bad MIC (LSB of uint32_t since boot time)
	 */
	uint16_t rx_bad_mic;

	/**
	 * @brief counter of received frames from friends with older fcnts
	 */
	uint16_t rx_replay;

	/**
	 * @brief counter of transmitted frames counter including repeated (LSB of uint32_t since boot time)
	 */
	uint16_t tx;

	/**
	 * @brief counter of repeated frames counter including repeated (LSB of uint32_t since boot time)
	 */
	uint16_t tx_repeat;
};

typedef struct stat_lgw stat_lgw_t;

/**
 * Struct for Stat message
 */
struct __attribute__((__packed__)) StatPayload {

	common_tx_t tx;

	common_time_t time;

	stat_lgw_t lgw;

	stat_ranging_t ranging;

	common_packed_location_t location;

	common_sensors_t sensors;
};

typedef struct StatPayload StatPayload_t;

#endif // _LORAWAN_STAT_PAYLOAD_H
