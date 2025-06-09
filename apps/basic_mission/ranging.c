/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>
#include <string.h>

#include "ranging.h"
#include "loragw_hal.h"
#include "lgw_utils.h"
#include "parse_nmea.h"
#include "pack_coord.h"

// Nota Bene : console trace slow sending message
#define TX_US_MARGIN			10000

static uint32_t		tx_uscount_prev = 0;


bool ranging_get_fpayload_1(
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
		const uint8_t		txpower,
		/**
		 * @brief Tx Power in dBm
		 * Useful for ADR
		 */
		const bool		ftime

) {
	Ranging01Payload_t* ranging1_payload = (Ranging01Payload_t*) fpayload;
	*fpayload_size = sizeof(Ranging01Payload_t);
	// reset buffer
	memset(ranging1_payload,0,*fpayload_size);


	int fix_quality;
	int satellites_tracked;
	if(!gps_get_quality(&fix_quality, &satellites_tracked)) {
		fix_quality = 0;
	}

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint32_t trig_cnt_us;
	lgw_get_trigcnt(&trig_cnt_us);

	uint32_t tx_uscount = 0;
	uint8_t status = 0;
	//if(fix_quality == 0) {
	if(trig_cnt_us == 0) {
		status |= TIMESTAMPED;
		tx_uscount = lgw_incr_instcnt(inst_cnt_us, TX_US_MARGIN);

	} else {
		status |= RANGING_GNSS_STATUS_FIX;

		uint32_t delta = lgw_get_delta_instcnt(trig_cnt_us, inst_cnt_us);

		if(delta > TX_US_MARGIN) {
			tx_uscount = lgw_incr_instcnt(trig_cnt_us, 1000000); // Number of seconds between 2 PPS
			status |= ON_GPS;
		} else {
			status |= TIMESTAMPED;
			tx_uscount = lgw_incr_instcnt(inst_cnt_us, TX_US_MARGIN);
		}
	}

	if(ftime) {
		status |= RANGING_GNSS_STATUS_FTIME;
	}

	ranging1_payload->tx_uscount = tx_uscount;
	ranging1_payload->tx_uscount_prev = tx_uscount_prev;
	tx_uscount_prev = tx_uscount;

	ranging1_payload->txpower = txpower;
	ranging1_payload->ranging_status = status;


	// Fill location
	ranging_location_t* location = &ranging1_payload->location;

	if(fix_quality==0) {
			location->latitude = 0;
			location->longitude = 0;
			location->altitude = 0;
	} else {
			float latitude, longitude, altitude;
			gps_get_position(&latitude, &longitude, &altitude);

			location->latitude = gps_pack_latitude_f_to_i24(latitude);
			location->longitude = gps_pack_longitude_f_to_i24(longitude);
			location->altitude = floor(altitude);
	}

	return true;
}
