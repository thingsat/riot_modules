/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_MISSION
#include "debug.h"

#include "endpoints.h"
#include "mission.h"
#include "lgw_cmd.h"
#include "lgw_utils.h"
#include "lorawan_mac.h"
#include "lorawan_printf.h"
#include "loragw_hal.h"

#include "pack_coord.h"
#include "parse_nmea.h"
#include "rtc_utilities.h"

#include "ztimer.h"

#include "repeat.h"
#include "ranging.h"
#include "mission.h"
#include "stat.h"
#include "common_payload.h"

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
	DEBUG("INFO: set fpayload for ranging1 (devaddr=%8lx fport=%d)\n", devaddr,
			fPort);

	uint8_t fpayload_size;
	bool res = basic_mission_ranging_get_fpayload_1(fpayload, &fpayload_size,
			MISSION_RF_POWER);
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

	_fCntUp++;

	return true;
}

static bool _set_phy_payload_telemetry(uint8_t *phypayload,
		uint8_t *phypayload_size) {

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint8_t fpayload_size;
	bool res = telemetry_get_fpayload(fpayload, &fpayload_size,
			MISSION_RF_POWER);
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

	DEBUG("INFO: set fpayload for telemetry (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	_fCntUp++;

	return true;
}

static bool _set_phy_payload_aprs_lorawan(uint8_t *phypayload,
		uint8_t *phypayload_size) {

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

	DEBUG(
			"INFO: set fpayload for aprs lorawan (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	_fCntUp++;

	return true;
}

static bool _set_phy_payload_aprs_ax25(uint8_t *phypayload,
		uint8_t *phypayload_size) {

	uint8_t fpayload_size;
	bool res = aprs_get_ax25(fpayload, &fpayload_size);
	if (!res) {
		return false;
	}

	memcpy(phypayload, fpayload, fpayload_size);
	*phypayload_size = fpayload_size;

	DEBUG("INFO: set fpayload for aprs ax25 (size=%d)\n", *phypayload_size);

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

	DEBUG("INFO: set fpayload for ewss (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	_fCntUp++;

	return true;
}

static bool _set_phy_payload_stat(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_STAT_PAYLOAD;

	StatPayload_t stat_payload;
	memset(&stat_payload, 0, sizeof(stat_payload));

	common_set_common_tx(&stat_payload.tx, MISSION_RF_POWER);
	common_set_common_time(&stat_payload.time);
	common_set_common_packed_location(&stat_payload.location);
	common_set_common_sensors(&stat_payload.sensors);
	stat_payload.lgw = stat_lgw;
	stat_payload.ranging = stat_ranging;

	const uint8_t fpayload_size = sizeof(stat_payload);
	memcpy(fpayload, &stat_payload, sizeof(stat_payload));

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			(uint8_t*) &fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	DEBUG("INFO: set fpayload for stat (devaddr=%8lx fport=%d size=%d)\n",
			devaddr, fPort, *phypayload_size);

	_fCntUp++;

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

void mission_period_cb(struct lgw_pkt_tx_s *lgw_pkt_tx_s) {

#if ENABLE_DEBUG == 1
	printf("INFO: %s enter\n", __FUNCTION__);
	mission_inst_cnt_print();
	mission_gnss_print();
#else
	mission_gnss_print_short();
#endif

	// For caller
	lgw_pkt_tx_s->size = 0;

	if (lgw_sx130x_endpoint == NULL) {
		printf("ERROR: lgw_sx130x_endpoint is null : Can not transmit\n");
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
#if PROD == 1
			// TODO set sync_word with MISSION_APRS_SYNCWORD
			lgw_pkt_tx_s->freq_hz = MISSION_APRS_FREQUENCY; 		// RX2 (duty cycle is 10%)
			lgw_pkt_tx_s->bandwidth = lgw_bandwidthToBwLoRaCode(MISSION_APRS_BANDWIDTH);
			lgw_pkt_tx_s->datarate = MISSION_APRS_SPREADING_FACTOR;
			lgw_pkt_tx_s->preamble = MISSION_APRS_PREAMBLE_LEN;			//  8 for LoRaWAN
			lgw_pkt_tx_s->coderate = MISSION_APRS_CODING_RATE; 		// LoRaWAN
			lgw_pkt_tx_s->rf_power = MISSION_APRS_RF_POWER;   		// Max power
			// TODO private syncword
#else
			// LoRa Service Channel (DR6)
			lgw_pkt_tx_s->freq_hz = 868300000;
			lgw_pkt_tx_s->bandwidth = BW_250KHZ;
			lgw_pkt_tx_s->rf_power = 14;
			GNSS Stats
			// Max power at RX1
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
		case STAT_PAYLOAD:
			res = _set_phy_payload_stat(phypayload, &phypayload_size);
			if (!res) {
				printf("ERROR: can not build payload for stat\n");
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

		stat_lgw.tx++;
	}
}

/**
 * Check pkt_rx
 */
bool basic_mission_check_pkt_rx(const struct lgw_pkt_rx_s *pkt_rx) {

	stat_lgw.rx++;

	if (pkt_rx->status == STAT_CRC_BAD) {
		// skip received frame
		DEBUG("INFO: status is BAD_CRC : skip received frame\n");
		stat_lgw.rx_bad_crc++;
		return false;
	}

	if (pkt_rx->size == 0) {
		// skip received frame
		DEBUG("INFO: Payload is empty : skip received frame\n");
		return false;
	}

	if (!(pkt_rx->modulation == MOD_LORA || pkt_rx->modulation == MOD_FSK)) {
		// skip received frame
		printf("INFO: modulation is not LoRa or FSK: skip received frame\n");
		return false;
	}

	return true;
}

/**
 * Rx callback
 */
void mission_rx_cb(const struct lgw_pkt_rx_s *pkt_rx,
		struct lgw_pkt_tx_s *pkt_tx) {
#if ENABLE_DEBUG_MISSION == 1
	mission_inst_cnt_print();
	mission_gnss_print();
#endif

	if (!basic_mission_check_pkt_rx(pkt_rx)) {
		return;
	}

	memset(pkt_tx, 0, sizeof(struct lgw_pkt_tx_s));
	pkt_tx->size = 0;

	const uint8_t *payload = pkt_rx->payload;
	const uint16_t payload_size = pkt_rx->size;

	if (!(lorawan_check_valid_frame_size(payload, payload_size)
			&& lorawan_is_dataframe(payload, payload_size))) {
		return;
	}

	const uint32_t rx_devaddr = lorawan_get_devaddr(payload, payload_size);

	if (rx_devaddr == lgw_sx130x_endpoint->devaddr) {
		printf("INFO: %08lx is my devaddr. Discarding ...\n", rx_devaddr);
		return;
	}

	const lorawan_endpoint_t *rx_endpoint = endpoint_get_endpoint(rx_devaddr);
	if (rx_endpoint == NULL) {
		printf("INFO: %08lx is not a friend's devaddr\n", rx_devaddr);

		// Try to process for repeating
		if (basic_mission_repeat_process(pkt_rx, pkt_tx, NULL)) {
			return;
		}

		return;
	}

	printf("INFO: %08lx is a friend's devaddr\n", rx_devaddr);

	stat_lgw.rx_friends++;

	const uint16_t fCnt = lorawan_get_fcnt(payload, payload_size);

	// Check fCnt > last fCnt
	//stat_lgw.rx_replay++;

	// check MIC
	if (!lorawan_check_mic_up(rx_devaddr, fCnt, rx_endpoint->nwkskey, payload,
			payload_size)) {
		stat_lgw.rx_bad_mic++;
		printf("INFO: Bad MIC for devaddr %08lx\n", rx_devaddr);
		return;
	}

	// Try to process for ranging (Ranging1, Ranging2, Ranging3)
	printf("INFO: try to process ranging for devaddr %08lx\n", rx_devaddr);
	if (basic_mission_ranging_process(pkt_rx, pkt_tx, rx_endpoint, _fCntUp)) {
		if (pkt_tx->size > 0) {
			_fCntUp++;
		}
		return;
	}

	// Try to process for repeating
	printf("INFO: try to process repeat for devaddr %08lx\n", rx_devaddr);
	if (basic_mission_repeat_process(pkt_rx, pkt_tx, rx_endpoint)) {
		/*		if(pkt_tx->size > 0) {
		 _fCntUp++;
		 }
		 */return;
	}

}

static bool rtc_set_since_reboot = false;

void mission_gnss_print_short(void) {

	if (gps_get_fix()) {
		if (rtc_set_since_reboot == false) {
			// Set RTC with GPS UTC date/time
			if (gps_set_rtc_time()) {
				rtc_set_since_reboot = true;
			}
		}

		print_rtc();

		float latitude, longitude, altitude;
		if (gps_get_position(&latitude, &longitude, &altitude)) {
			printf("INFO: GNSS lat=%.6f lat=%.6f, alt=%.1f", latitude,
					longitude, altitude);

			//TODO add speed and bearing
			float speed_kph;
			float true_track_degrees;
			if (gps_get_speed_direction(&speed_kph, &true_track_degrees)) {
				printf(" speed=%.1f m/s (%.1f km/h), track=%.0f°",
						speed_kph / 3.6, speed_kph, true_track_degrees);
			}

			int fix_quality, satellites_tracked;
			if (gps_get_quality(&fix_quality, &satellites_tracked)) {
				printf(" qual=%d, sat=%d",
						fix_quality, satellites_tracked);
			}
			printf("\n");
			return;
		}
	}
	printf("INFO: GNSS No fix (since=%d)\n", gps_get_seconds_since_last_fix());
}

/**
 * TODO add periodic resynchronization of RTC
 */
void mission_gnss_print(void) {

	mission_gnss_print_short();

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

