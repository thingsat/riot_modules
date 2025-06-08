/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <string.h>

#include "endpoints.h"
#include "mission.h"
#include "lgw_cmd.h"
#include "lorawan_mac.h"

#include "parse_nmea.h"
#include "rtc_utilities.h"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    LGW_HAL_SUCCESS
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE    LGW_HAL_ERROR
#endif

// For lgw_bench
static uint32_t _fCntUp = 0;

// APRS (https://www.aprs.org/doc/APRS101.PDF) like text message.
const uint8_t fpayload[] =
		"@sx1302@world :This is a simple test$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62";

static bool _set_phy_payload(uint8_t *phypayload, uint8_t *phypayload_size) {
	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;

	const uint8_t fpayload_size = sizeof(fpayload);
	const uint8_t fPort = 100; // For Thingsat decoder (Plaintext)

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	return true;
}

static bool rtc_set_since_reboot = false;

/**
 * TODO add periodic resynchronization of RTC
 */
void mission_gnss_print(void) {

	float latitude, longitude, altitude;
	if (gps_get_position(&latitude, &longitude, &altitude)) {
		printf("GNSS: lat=%.6f lat=%.6f, lat=%.1f\n", latitude, longitude,
				altitude);

		if (rtc_set_since_reboot == false) {
			// Set RTC with GPS UTC date/time
			if (gps_set_rtc_time()) {
				rtc_set_since_reboot = true;
			}
		}
	} else {
		printf("GNSS: No fix\n");
	}

	print_rtc();

	gps_print();
}

void mission_inst_cnt_print(void) {

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint32_t trig_cnt_us;
	lgw_get_trigcnt(&trig_cnt_us);

	printf("INFO: inst_cnt_us=%ld, trig_cnt_us=%ld\n", inst_cnt_us,
			trig_cnt_us);

	// TODO Compute delta

}

void mission_periodic_cb(struct lgw_pkt_tx_s *lgw_pkt_tx_s) {
	printf("INFO: call mission\n");

	mission_inst_cnt_print();
	mission_gnss_print();

	// For caller
	lgw_pkt_tx_s->size = 0;

	if (lgw_sx130x_endpoint == NULL) {
		puts("ERROR: lgw_sx130x_endpoint is null : Can not transmit");
	} else {

		uint8_t phypayload_size;

		if (!_set_phy_payload(lgw_pkt_tx_s->payload, &phypayload_size)) {
			puts("ERROR: false to set phypayload");
			return;
		}
		lgw_pkt_tx_s->size = phypayload_size;

		lgw_pkt_tx_s->tx_mode = IMMEDIATE;
		lgw_pkt_tx_s->rf_chain = 0; //only rf_chain 0 is able to tx
		lgw_pkt_tx_s->rf_power = MISSION_RF_POWER; //use the single entry of the txlut TODO Should be check

		lgw_pkt_tx_s->modulation = MOD_LORA;	// ONLY LoRa (No FSK)

		// use _fCntUp for changing the frequency each transmit
		lgw_pkt_tx_s->freq_hz = lgw_get_freq_hz(
				_fCntUp % lgw_frequency_plan_size());
		lgw_pkt_tx_s->bandwidth = BW_125KHZ;
		lgw_pkt_tx_s->datarate = MISSION_SF;
		lgw_pkt_tx_s->preamble = MISSION_PREAMBLE_LEN;	//  8 for LoRaWAN
		lgw_pkt_tx_s->coderate = CR_LORA_4_5; // LoRaWAN

		lgw_pkt_tx_s->no_header = false; 		// Beacons have not header

		bool crc_on = true;
		bool invert_pol = false;

		lgw_pkt_tx_s->no_crc = (crc_on != true); // LoRaWAN : on for uplink and off for downlink
		lgw_pkt_tx_s->invert_pol = invert_pol;
	}

}
