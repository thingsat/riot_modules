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


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define FPORT_TELEMETRY_PAYLOAD					(4U)
#define FPORT_APRS_PAYLOAD						(14U)
#define FPORT_RANGE1_PAYLOAD					(21U)
// Emergency Warning Satellite Service (Galileo like)
#define FPORT_EWSS_PAYLOAD						(17U)


// For lgw_bench
static uint32_t _fCntUp = 0;

static uint32_t counter = 0;



const uint8_t message_types[] = MISSION_MESSAGES_TYPE;

static uint8_t phypayload[256];

static bool _set_phy_payload_range1(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_RANGE1_PAYLOAD;

	printf("INFO: set payload for range1 (devaddr=%8lx fport=%d)\n", devaddr, fPort);

	(void)phypayload;
	(void)phypayload_size;

/*
	lorawan_prepare_up_dataframe(
			false,
			devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			fpayload,
			fpayload_size,
			lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey,
			phypayload,
			phypayload_size);
*/


	(void)devaddr;
	(void)fPort;

	phypayload_size = 0;
	return false;
}

static bool _set_phy_payload_telemetry(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_TELEMETRY_PAYLOAD;

	printf("INFO: set payload for telemetry (devaddr=%8lx fport=%d)\n", devaddr, fPort);

	(void)phypayload;
	(void)phypayload_size;

	phypayload_size = 0;


	(void)devaddr;
	(void)fPort;

	return false;
}

// APRS (https://www.aprs.org/doc/APRS101.PDF) like text message.
const uint8_t fpayload[] =
		"@sx1302@world :This is a simple test$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62";

static bool _set_phy_payload_aprs(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_APRS_PAYLOAD; // For Thingsat decoder (Plaintext)

	printf("INFO: set payload for aprs (devaddr=%8lx fport=%d)\n", devaddr, fPort);

	const uint8_t fpayload_size = sizeof(fpayload);
	memcpy(phypayload, fpayload, fpayload_size);

	lorawan_prepare_up_dataframe(
	false, devaddr,
			0x00, // FCTrl (FOptLen = 0)
			_fCntUp,
			fPort, // fPort
			fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, phypayload, phypayload_size);

	return true;
}

static bool _set_phy_payload_ewss(uint8_t *phypayload, uint8_t *phypayload_size) {

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;
	const uint8_t fPort = FPORT_EWSS_PAYLOAD;

	printf("INFO: set payload for ewss (devaddr=%8lx fport=%d)\n", devaddr, fPort);

	(void)phypayload;
	(void)phypayload_size;

	phypayload_size = 0;


	(void)devaddr;
	(void)fPort;

	return false;
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

		bool res;
		switch(counter++ % ARRAY_SIZE(message_types)) {
		case RANGE1_PAYLOAD:
			res = _set_phy_payload_range1(phypayload, &phypayload_size);
			if(!res) {
				printf("ERROR: can not build payload for range1\n");
			}
			break;
		case TELEMETRY_PAYLOAD:
			res = _set_phy_payload_telemetry(phypayload, &phypayload_size);
			if(!res) {
				printf("ERROR: can not build payload for telemetry\n");
			}
			break;
		case APRS_PAYLOAD:
			res = _set_phy_payload_aprs(phypayload, &phypayload_size);
			if(!res) {
				printf("ERROR: can not build payload for aprs\n");
			}
			break;
		case EWSS_PAYLOAD:
			res = _set_phy_payload_ewss(phypayload, &phypayload_size);
			if(!res) {
				printf("ERROR: can not build payload for ewss\n");
			}
			break;
		default:
			printf("ERROR: unknown payload\n");
			res = false;
			break;
		}

		if(!res){
			return;
		}

		memcpy(lgw_pkt_tx_s->payload, phypayload, phypayload_size);
		lgw_pkt_tx_s->size = phypayload_size;

		lgw_pkt_tx_s->tx_mode = IMMEDIATE;
		lgw_pkt_tx_s->rf_chain = 0; //only rf_chain 0 is able to tx
		lgw_pkt_tx_s->rf_power = MISSION_RF_POWER; //use the single entry of the txlut TODO Should be check

		lgw_pkt_tx_s->modulation = MOD_LORA;	// ONLY LoRa (No FSK)

		// use _fCntUp for changing the frequency each transmit
		lgw_pkt_tx_s->freq_hz = lgw_get_freq_hz(_fCntUp % lgw_frequency_plan_size());
		lgw_pkt_tx_s->bandwidth = BW_125KHZ;
		lgw_pkt_tx_s->datarate = MISSION_SF;
		lgw_pkt_tx_s->preamble = MISSION_PREAMBLE_LEN;	//  8 for LoRaWAN
		lgw_pkt_tx_s->coderate = CR_LORA_4_5; // LoRaWAN

		lgw_pkt_tx_s->no_header = false; 		// Beacons have not header

		bool crc_on = true;
		bool invert_pol = false;

		lgw_pkt_tx_s->no_crc = (crc_on != true); // LoRaWAN : on for uplink and off for downlink
		lgw_pkt_tx_s->invert_pol = invert_pol;

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

	printf("INFO: inst_cnt_us=%ld, trig_cnt_us=%ld\n", inst_cnt_us,
			trig_cnt_us);

	// TODO Compute delta

}

