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

#include <string.h>
#include <math.h>
#include <stdio.h>

#include "ztimer.h"

#include "lorawan_common_payload.h"

#include "lgw_utils.h"
#include "loragw_hal.h"

#include "pack_coord.h"
#include "parse_nmea.h"
#include "rtc_utilities.h"

void common_set_common_tx(common_tx_t *tx, uint8_t txpower) {

	memset(tx,0,sizeof(common_tx_t));

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint32_t trig_cnt_us;
	lgw_get_trigcnt(&trig_cnt_us);

	tx->status.tx_mode = IMMEDIATE;
	tx->status.fix = 0;
	tx->status.ftime = 0;

	tx->tx_trigcount = trig_cnt_us;
	tx->tx_uscount = inst_cnt_us;
	tx->txpower = txpower;
}

void common_set_common_time(common_time_t *t) {

	memset(t,0,sizeof(common_time_t));

	t->localtime = get_time_since_epoch();
	t->uptime = ztimer_now(ZTIMER_SEC);
}

void common_set_common_location(common_location_t *location) {

	memset(location,0,sizeof(common_location_t));

	if(gps_get_fix()) {
		float latitude, longitude, altitude;
		gps_get_position(&latitude, &longitude, &altitude);

		location->latitude = latitude;
		location->longitude = longitude;
		location->altitude = floor(altitude);
	} else {
		location->latitude = 0;
		location->longitude = 0;
		location->altitude = 0;
	}

}

void common_set_common_location_extra(common_location_extra_t *location_extra) {

	memset(location_extra,0,sizeof(common_location_extra_t));

	// Il faut changer le decoder.js
	location_extra->seconds_since_last_fix = gps_get_seconds_since_last_fix();

	int fix_quality;
	int satellites_tracked;

	if(gps_get_quality(&fix_quality, &satellites_tracked)) {
		location_extra->satellites_tracked = (uint8_t)satellites_tracked;
		location_extra->fix_quality = (uint8_t)fix_quality;

		float speed_kph;
		float true_track_degrees;

		if(gps_get_speed_direction(&speed_kph, &true_track_degrees)){
			location_extra->speed_kph = floor(speed_kph);
			location_extra->true_track_degrees =  floor(true_track_degrees);
		}

	} else {
		location_extra->speed_kph = 0;
		location_extra->true_track_degrees = 0;
		location_extra->fix_quality = 0;
		location_extra->satellites_tracked = 0;
	}
}

void common_set_common_packed_location(common_packed_location_t *location) {

	memset(location,0,sizeof(common_packed_location_t));

	if(gps_get_fix()) {

		float latitude, longitude, altitude;
		gps_get_position(&latitude, &longitude, &altitude);

		location->latitude = gps_pack_latitude_f_to_i24(latitude);
		location->longitude = gps_pack_longitude_f_to_i24(longitude);
		location->altitude = floor(altitude);
	} else {
		location->latitude = 0;
		location->longitude = 0;
		location->altitude = 0;
	}
}

void common_set_common_sensors(common_sensors_t *sensors) {

	memset(sensors,0,sizeof(common_sensors_t));

	sensors->ts = 0;
	sensors->vbat = 0;
}


void common_payload_tx_printf(const common_tx_t *tx){
	printf("  status         : %s, %s, %s\n",
			tx->status.tx_mode == IMMEDIATE ? "IMMEDIATE": (tx->status.tx_mode == TIMESTAMPED ? "TIMESTAMPED" : "ON_GPS"),
			(tx->status.fix == 1 ? "fix":"no fix"),
					(tx->status.ftime == 1 ? "ftime":"no ftime")
	);
	printf("  tx_uscount     : %lu  \n", tx->tx_uscount);
	printf("  tx_trigcount   : %lu\n", tx->tx_trigcount);
	printf("  txpower        : %d\n", tx->txpower);
}

void common_payload_packed_location_printf(const common_packed_location_t *l){
	printf("  latitude       : %0.5f\n", gps_unpack_latitude_i24_to_f(l->latitude));
	printf("  longitude      : %0.5f\n", gps_unpack_longitude_i24_to_f(l->longitude));
	printf("  altitude       : %d\n", l->altitude);
}
