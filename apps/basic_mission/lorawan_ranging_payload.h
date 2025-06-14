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


#define RANGING_GNSS_STATUS_FIX			0b10000000
#define RANGING_GNSS_STATUS_FTIME		0b01000000
#define RANGING_TX_MODE_STATUS_MASK		0b00000011


/**
 * Struct for Ranging location
 */
struct __attribute__((__packed__)) ranging_location {

	// TODO add GPS Fix last time (in seconds), GPS quality, GPS satellite number ...

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

typedef struct ranging_location ranging_location_t;

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

	/**
	 * @brief us counter at TX
	 */
	uint32_t		tx_uscount;

	/**
	 * @brief us counter at the previous ranging message TX (fcntup - 1)
	 */
	uint32_t		tx_uscount_prev;

	/**
	 * @brief Ranging Status
	 * 1 for GPS_FIX, 2 for FTIME, TxMode ()
	 */
	uint8_t		ranging_status;

	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	uint8_t		txpower;

	/**
	 * @brief Location
	 * optional field but useful for checking distance when debugging
	 */
	ranging_location_t location;
};

typedef struct Ranging01Payload Ranging01Payload_t;

/**
 * Struct for Ranging message for step #2 for reply to step #1
 */
struct __attribute__((__packed__)) Ranging2Payload {

	/**
	 * @brief us counter at TX
	 */
	uint32_t		tx_uscount;

	/**
	 * @brief us counter at the previous ranging message TX (fcntup - 1)
	 */
	uint32_t		tx_uscount_prev;

	/**
	 * @brief Ranging Status
	 * 1 for GPS_FIX, 2 for FTIME, TxMode ()
	 */
	uint8_t		ranging_status;

	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	uint8_t		txpower;

	/**
	 * @brief the two least significant bytes of 32 bits fCnt of the message #1
	 */
	uint16_t fcnt1;

	/**
	 * @brief DevAddr of sender of the message #1
	 */
	uint32_t devaddr1;

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
	ranging_location_t location;
};

typedef struct Ranging2Payload Ranging2Payload_t;



/**
 * Struct for Ranging message for step #3 for reply to step #2
 */
struct __attribute__((__packed__)) Ranging3Payload {

	/**
	 * @brief us counter at TX
	 */
	uint32_t		tx_uscount;

	/**
	 * @brief us counter at the previous ranging message TX (fcntup - 1)
	 */
	uint32_t		tx_uscount_prev;

	/**
	 * @brief Ranging Status
	 * 1 for GPS_FIX, 2 for FTIME, TxMode ()
	 */
	uint8_t		ranging_status;

	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	uint8_t		txpower;

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
	ranging_location_t location;

	/**
	 * @brief Distances between the sender and the others receivers in the swarm
	 * 0 means unknown distance
	 * optional field but useful for checking distance during debug
	 */
	uint32_t distances[1];
};

typedef struct Ranging3Payload Ranging3Payload_t;


#endif // LORAWAN_RANGING_PAYLOAD_H
