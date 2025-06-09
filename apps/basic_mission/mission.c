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
#include "lgw_utils.h"
#include "lorawan_mac.h"

#include "parse_nmea.h"
#include "rtc_utilities.h"

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS    LGW_HAL_SUCCESS
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE    LGW_HAL_ERROR
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#include "ranging.h"
#include "telemetry.h"
#include "aprs.h"
#include "ewss.h"

// For lgw_bench
static uint32_t _fCntUp = 0;

static uint32_t counter = 0;

const uint8_t message_types[] = MISSION_MESSAGES_TYPE;

static uint8_t fpayload[256];
static uint8_t phypayload[256];


#if 0
static bool _set_phy_payload_lgw_stats(uint8_t *phypayload,
		uint8_t *phypayload_size) {
	uptime
	ftime
	fix
	rx
	repeated
	counter
	location
}
#endif

static bool _set_phy_payload_range1(uint8_t *phypayload,
		uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_RANGE1_PAYLOAD;

	// Should be printed before ranging_get_fpayload_1 --> tx_count may be over
	printf("INFO: set fpayload for ranging (devaddr=%8lx fport=%d)\n",
			devaddr, fPort);

	uint8_t fpayload_size;
	bool res = ranging_get_fpayload_1(fpayload, &fpayload_size, MISSION_RF_POWER, false /* TODO ftime */);
	if (!res) {
		return false;
	}
	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			(uint8_t*) &fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	return true;
}

static bool _set_phy_payload_telemetry(uint8_t *phypayload,
		uint8_t *phypayload_size) {

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint8_t fpayload_size;
	bool res = telemetry_get_fpayload(fpayload, &fpayload_size,
			inst_cnt_us, 		// tx_uscount,
			0,					// tx_uscount_prev,
			IMMEDIATE,		  	// status,
			MISSION_RF_POWER  	// txpower
			);
	if (!res) {
		return false;
	}

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_GNSS_TELEMETRY_PAYLOAD;

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	printf("INFO: set fpayload for telemetry (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	return true;
}


static bool _set_phy_payload_aprs_lorawan(uint8_t *phypayload, uint8_t *phypayload_size) {

	uint8_t fpayload_size;
	bool res = aprs_get_fpayload(fpayload, &fpayload_size);
	if (!res) {
		return false;
	}

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_APRS_PAYLOAD;

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	printf("INFO: set fpayload for aprs lorawan (devaddr=%8lx fport=%d size=%d aprs=%s)\n",
			devaddr, fPort, *phypayload_size, fpayload);

	return true;
}


static bool _set_phy_payload_aprs_ax25(uint8_t *phypayload, uint8_t *phypayload_size) {

	uint8_t fpayload_size;
	bool res = aprs_get_ax25(fpayload, &fpayload_size);
	if (!res) {
		return false;
	}

	memcpy(phypayload, fpayload, fpayload_size);
	*phypayload_size = fpayload_size;

	printf("INFO: set fpayload for aprs ax24 (size=%d)\n",
			*phypayload_size);

	return true;
}


const uint8_t ewss_fpayload[] = MISSION_EWSS_PAYLOAD;

static bool _set_phy_payload_ewss(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
		const uint8_t fPort = FPORT_EWSS_PAYLOAD;

	const uint8_t fpayload_size = sizeof(ewss_fpayload);
	memcpy(fpayload, ewss_fpayload, fpayload_size);

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			(uint8_t*) &fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	printf("INFO: set fpayload for ewss (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	return true;
}


void mission_set_default_lgw_pkt_tx(struct lgw_pkt_tx_s *lgw_pkt_tx_s) {
	lgw_pkt_tx_s->size = 0;

	lgw_pkt_tx_s->tx_mode = IMMEDIATE;
	lgw_pkt_tx_s->rf_chain = 0; //only rf_chain 0 is able to tx
	lgw_pkt_tx_s->rf_power = MISSION_RF_POWER; //use the single entry of the txlut TODO Should be check

	lgw_pkt_tx_s->modulation = MOD_LORA;	// ONLY LoRa (No FSK)

	// use _fCntUp for changing the frequency each transmit
	lgw_pkt_tx_s->freq_hz = lgw_get_freq_hz(
			counter % lgw_frequency_plan_size());
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

void mission_periodic_cb(struct lgw_pkt_tx_s *lgw_pkt_tx_s) {
	printf("INFO: call mission\n");

	mission_inst_cnt_print();
	mission_gnss_print();

	// For caller
	lgw_pkt_tx_s->size = 0;

	if (lgw_sx130x_endpoint == NULL) {
		puts("ERROR: lgw_sx130x_endpoint is null : Can not transmit");
	} else {

		mission_set_default_lgw_pkt_tx(lgw_pkt_tx_s);

		uint8_t phypayload_size;
		bool res;
		switch (counter++ % ARRAY_SIZE(message_types)) {
		case RANGE1_PAYLOAD:
			res = _set_phy_payload_range1(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for range1\n");
			}
			break;
		case TELEMETRY_PAYLOAD:
			res = _set_phy_payload_telemetry(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for telemetry\n");
			}
			break;
		case APRS_LORAWAN_PAYLOAD:
			res = _set_phy_payload_aprs_lorawan(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for aprs lorawan\n");
			}
			break;
		case APRS_AX25_PAYLOAD:
			res = _set_phy_payload_aprs_ax25(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for aprs ax25\n");
			}

			// overload default param
			lgw_pkt_tx_s->datarate = 7;
#if 0
			lgw_pkt_tx_s->freq_hz = 869525000; 	// RX2 (duty cycle is 10%)
			lgw_pkt_tx_s->rf_power = 27; 		// Max power
			// TODO private syncword
#else
			// LoRa Service Channel (DR6)
			lgw_pkt_tx_s->freq_hz = 868300000;
			lgw_pkt_tx_s->bandwidth = BW_250KHZ;
			lgw_pkt_tx_s->rf_power = 14; 		// Max power at RX1
			// public syncword for test
#endif
			res = true;
			break;
		case EWSS_PAYLOAD:
			res = _set_phy_payload_ewss(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for ewss\n");
			}
			break;
		default:
			printf("ERROR: unknown payload\n");
			res = false;
			break;
		}

		if (!res) {
			return;
		}

		memcpy(lgw_pkt_tx_s->payload, phypayload, phypayload_size);
		lgw_pkt_tx_s->size = phypayload_size;


		_fCntUp++;
	}
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

	if (trig_cnt_us == 0) {
		printf("INFO: inst_cnt_us=%ld, trig_cnt_us=%ld\n", inst_cnt_us,
				trig_cnt_us);
	} else {
		printf("INFO: inst_cnt_us=%ld, trig_cnt_us=%ld, delta=%ld\n",
				inst_cnt_us, trig_cnt_us,
				lgw_get_delta_instcnt(trig_cnt_us, inst_cnt_us));
	}
}

