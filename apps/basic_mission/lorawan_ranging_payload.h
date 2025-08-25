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
 * Types for the format of the payload into the LoRaWAN frames sent and received by the Thingsat payload
 */


#ifndef _LORAWAN_RANGING_PAYLOAD_H
#define _LORAWAN_RANGING_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>

#include "lorawan_common_payload.h"

/*
 * FPort definition for typing the payload format
 */

// Ranging Service
#define FPORT_RANGE0_PAYLOAD					(20U)
#define FPORT_RANGE1_PAYLOAD					(21U)
#define FPORT_RANGE2_PAYLOAD					(22U)
#define FPORT_RANGE3_PAYLOAD					(23U)


#define RANGING_NO_FTIME 						(0xFFFFFFFFU)

/**
 * @brief Up Text Message Payload
 * The payload size should not be excced 51 bytes
 * Transport: Data Frame Dn
 */

/**
 * Struct for Ranging message for step #0 and #1
 * Initiating the TWR protocol
 */
struct __attribute__((__packed__)) Ranging01Payload {

	common_tx_t tx;

	/**
	 * @brief Location
	 * optional field but useful for checking distance when debugging
	 */
	common_packed_location_t location;
};

typedef struct Ranging01Payload Ranging01Payload_t;

/**
 * Struct for Ranging message for step #2 for reply to step #1
 */
struct __attribute__((__packed__)) Ranging2Payload {

	common_tx_t tx;

	/**
	 * @brief DevAddr of sender of the message #1
	 */
	uint32_t devaddr1;

	/**
	 * @brief the two least significant bytes of 32 bits fCnt of the message #1
	 */
	uint16_t fcnt1;

	/**
	 * @brief us counter at RX of the message #1
	 */
	uint32_t	rx_uscount;

	/**
	 * @brief fine timestamp at RX of the message #1
	 */
	uint32_t	rx_ftime;

	/**
	 * @brief SNR of message #1
	 * Useful for ADR
	 */
	int8_t		snr1;

	/**
	 * @brief RSSI (negative) of message #1
	 * Useful for ADR
	 */
	uint8_t		rssi1;

	/**
	 * @brief Distance between the sender and the receiver identified with the devaddr field (in meter)
	 * 0 meter if the distance can not be computed by the receiver
	 * optional field
	 */
	uint32_t distance12;

	/**
	 * @brief Location
	 * optional field but useful for checking distance when debugging
	 */
	common_packed_location_t location;
};

typedef struct Ranging2Payload Ranging2Payload_t;



/**
 * Struct for Ranging message for step #3 for reply to step #2
 */
struct __attribute__((__packed__)) Ranging3Payload {

	common_tx_t tx;

	/**
	 * @brief DevAddr of sender of the message #2
	 */
	uint32_t devaddr2;

	/**
	 * @brief the two least significant bytes of 32 bits fCnt of the message #2
	 */
	uint16_t fcnt2;

	/**
	 * @brief us counter at RX of the message #2
	 */
	uint32_t	rx_uscount;

	/**
	 * @brief fine timestamp at RX of the message #2
	 */
	uint32_t	rx_ftime;

	/**
	 * @brief SNR of message #2
	 * Useful for ADR
	 */
	int8_t		snr2;

	/**
	 * @brief RSSI (negative) of message #2
	 * Useful for ADR
	 */
	uint8_t		rssi2;

	/**
	 * @brief Distance between the sender and the receiver identified with the devaddr field (in meter)
	 */
	uint32_t distance12;

	/**
	 * @brief Location
	 * optional field but useful for checking distance when debugging
	 */
	common_packed_location_t location;

	/**
	 * @brief Distances between the sender and the others receivers in the swarm
	 * 0 means unknown distance
	 * optional field but useful for checking distance during debug
	 */
	uint32_t distances[1];
};

typedef struct Ranging3Payload Ranging3Payload_t;


#endif // LORAWAN_RANGING_PAYLOAD_H
