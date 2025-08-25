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

#ifndef _COMMON_PAYLOAD_H
#define _COMMON_PAYLOAD_H

#include "lorawan_common_payload.h"


// Nota Bene : console trace slow sending message
#ifndef TX_US_MARGIN
#define TX_US_MARGIN			10000
#endif

void common_set_common_time(common_time_t *time);

void common_set_common_sensors(common_sensors_t *sensors);

void common_set_common_location(common_location_t *location);

void common_set_common_location_extra(common_location_extra_t *location_extra);

void common_set_common_packed_location(common_packed_location_t *location);

void common_set_common_tx(common_tx_t *tx, uint8_t txpower);

void common_payload_tx_printf(const common_tx_t *tx);

void common_payload_packed_location_printf(const common_packed_location_t *l);


#endif // _COMMON_PAYLOAD_H
