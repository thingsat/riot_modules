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


#ifndef CAMF_LORAWAN_H
#define CAMF_LORAWAN_H

#include <stdint.h>
#include <stdbool.h>

#include "lorawan_service.h"

#include "camf_payload.h"

/*
 * FPort definition for typing the payload format
 */

// Emergency Warning Satellite Service (Galileo like)
#define FPORT_DN_EWSS_PAYLOAD					(17U)

/*
 *  @brief Common Alert Message LoRa Frame
 * Transport: Data Frame Dn (Ground -> Sat) : CRC=on, IQ=inverted
 */
struct __attribute__((__packed__)) CommonAlertMessageFrame
{
	/*
	 * @brief Header
	 */
	ServiceMessageHeader_t header;

	/*
	 * @brief Common Alert Message
	 */
	CommonAlertMessageFormat_t cam;

};

typedef struct CommonAlertMessageFrame CommonAlertMessageFrame_t;


/*
 * EWSSDnMessagePayload
 * @brief EWSS Dn Message Payload
 * Transport: Data Frame Up (Ground -> Sat)
 */
struct __attribute__((__packed__)) EWSSDnMessagePayload
{
	/*
	 * @brief Id of the order for CommonAlertMessageFormat broadcast
	 * Note: idempotent number
	 * Note: if a new payload wiith the same id but a newer start_time, the new message cancels and replaces the previous one
	 */
	uint32_t id;

    /*
     * @brief Start time (epoch in seconds)
     * 0 for cancelling the broadcast
     */
    uint32_t  start_time;

    /*
     * @brief Duration (in minutes)
     */
    uint16_t  duration;

    /*
     * @brief Geohash of the ground square to broadcast
     * TODO should be improved with ellipse
     * @see https://en.wikipedia.org/wiki/Geohash
     */
	uint32_t geohash;

    /*
     * @brief Precision of the geohash
     */
	uint8_t geohash_precision;

    /*
     * @brief Period of message repetition (in seconds)
     */
	uint8_t period;

    /*
     * @brief  alert message to broadcast
     */
	CommonAlertMessageFormat_t alert_message;

};

typedef struct EWSSDnMessagePayload EWSSDnMessagePayload_t;

void CommonAlertMessageFormat_printf(const CommonAlertMessageFormat_t* p);

void EWSSDnMessagePayload_printf(const EWSSDnMessagePayload_t* p, const uint8_t payload_size);


#endif // CAMF_LORAWAN_H
