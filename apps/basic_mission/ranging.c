/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>
#include <string.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_RANGING
#include "debug.h"


#define MAX(a,b) (((a)>(b))?(a):(b))

#include "loragw_hal.h"
#include "lgw_utils.h"
#include "lorawan_crypto.h"
#include "lorawan_mac.h"
#include "lorawan_printf.h"
#include "parse_nmea.h"
#include "pack_coord.h"
#include "common_payload.h"
#include "ranging_payload.h"

#include "mission.h"
#include "ranging.h"
#include "stat.h"

static bool basic_mission_ranging_set_common_tx(common_tx_t *tx,
		const uint8_t txpower, const bool ftime_received) {

	memset(tx, 0, sizeof(common_tx_t));

	int fix_quality;
	int satellites_tracked;
	if (!gps_get_quality(&fix_quality, &satellites_tracked)) {
		fix_quality = 0;
	}
	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint32_t trig_cnt_us;
	lgw_get_trigcnt(&trig_cnt_us);

	//if(fix_quality == 0) {
	if (trig_cnt_us == 0) {
		tx->status.tx_mode = TIMESTAMPED;
		tx->tx_uscount = lgw_incr_instcnt(inst_cnt_us, TX_US_MARGIN);

	} else {
		tx->status.fix = 1;

		uint32_t delta = lgw_get_delta_instcnt(trig_cnt_us, inst_cnt_us);

		if (delta > TX_US_MARGIN) {
			tx->tx_uscount = lgw_incr_instcnt(trig_cnt_us, 1000000); // Number of seconds between 2 PPS
			tx->status.tx_mode = ON_GPS;
		} else {
			tx->status.tx_mode = TIMESTAMPED;
			tx->tx_uscount = lgw_incr_instcnt(inst_cnt_us, TX_US_MARGIN);
		}
	}

	if (ftime_received) {
		tx->status.ftime = 1;
	}

	tx->txpower = txpower;

	return true;
}

bool basic_mission_ranging_get_fpayload_1(
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
const uint8_t txpower

) {
	printf("INFO: Preparing ranging 1 frame:\n");

	Ranging01Payload_t *ranging1_payload = (Ranging01Payload_t*) fpayload;
	*fpayload_size = sizeof(Ranging01Payload_t);
	// reset buffer
	memset(ranging1_payload, 0, *fpayload_size);

	basic_mission_ranging_set_common_tx(&ranging1_payload->tx, txpower, false);

	common_set_common_packed_location(&ranging1_payload->location);
#if 0
	ranging_payload_range1_printf(ranging1_payload);
#endif
	stat_ranging.range1_tx++;

	return true;
}

static bool basic_mission_ranging_process_range1(
		const struct lgw_pkt_rx_s *pkt_rx,
		const lorawan_endpoint_t *rx_endpoint, const uint8_t txpower,
		const Ranging01Payload_t *ranging1_payload,
		Ranging2Payload_t *ranging2_payload) {


	const uint8_t *rx_payload = pkt_rx->payload;
	const uint16_t rx_payload_size = pkt_rx->size;

	stat_ranging.range1_rx++;

	const uint32_t rx_devaddr = rx_endpoint->devaddr;
	const uint16_t rx_fcnt = lorawan_get_fcnt(rx_payload, rx_payload_size);

	printf("INFO: Ranging 1 from %08lx (fnct=%u):\n", rx_devaddr, rx_fcnt);
	ranging_payload_range1_printf(ranging1_payload);

	// build the frame

	basic_mission_ranging_set_common_tx(&ranging2_payload->tx, txpower,
			pkt_rx->ftime_received);

	common_set_common_packed_location(&ranging2_payload->location);

	ranging2_payload->devaddr1 = rx_devaddr;
	ranging2_payload->fcnt1 = rx_fcnt;
	ranging2_payload->rssi1 = floor(pkt_rx->rssic * -1);
	ranging2_payload->snr1 = floor(pkt_rx->snr);

	ranging2_payload->rx_uscount = pkt_rx->count_us;
	ranging2_payload->rx_ftime =
			pkt_rx->ftime_received ? pkt_rx->ftime : RANGING_NO_FTIME;

	// TODO compute ranging2_payload->distance12
	ranging2_payload->distance12 = 0;

#if 0
	printf("[INFO] Preparing ranging 2 frame:\n");
	ranging_payload_range2_printf(ranging2_payload);
#endif

	//stat_ranging.range2_tx++;
	return true;
}

static bool basic_mission_ranging_process_range2(
		const struct lgw_pkt_rx_s *pkt_rx,
		const lorawan_endpoint_t *rx_endpoint, const uint8_t txpower,
		const Ranging2Payload_t *ranging2_payload,
		Ranging3Payload_t *ranging3_payload) {
	(void) rx_endpoint;

	const uint8_t *rx_payload = pkt_rx->payload;
	const uint16_t rx_payload_size = pkt_rx->size;

	stat_ranging.range2_rx++;

	const uint32_t rx_devaddr = rx_endpoint->devaddr;
	const uint16_t rx_fcnt = lorawan_get_fcnt(rx_payload, rx_payload_size);

	printf("[INFO] Ranging 2 from %08lx (fnct=%u):\n", rx_devaddr, rx_fcnt);
	ranging_payload_range2_printf(ranging2_payload);

	if (ranging2_payload->devaddr1 == lgw_sx130x_endpoint->devaddr) {
		printf("[INFO] This is an answer to my previous range1 message\n");
		// This is answer to my previous range1 message
		stat_ranging.range2_rx_replies++;

		// Answer by message type 3
		basic_mission_ranging_set_common_tx(&ranging3_payload->tx, txpower,
				pkt_rx->ftime_received);

		common_set_common_packed_location(&ranging3_payload->location);

		ranging3_payload->devaddr2 = rx_devaddr;
		ranging3_payload->fcnt2 = rx_fcnt;
		ranging3_payload->rssi2 = floor(pkt_rx->rssic * -1);
		ranging3_payload->snr2 = floor(pkt_rx->snr);

		ranging3_payload->rx_uscount = pkt_rx->count_us;
		ranging3_payload->rx_ftime =
				pkt_rx->ftime_received ? pkt_rx->ftime : RANGING_NO_FTIME;

		// TODO compute ranging3_payload->distance12
		ranging3_payload->distance12 = 0;
		// TODO compute ranging3_payload->distances

#if 0
		printf("[INFO] Preparing ranging 3 frame:\n");
		ranging_payload_range3_printf(ranging3_payload);
#endif

	} else {
		printf("[INFO] This is an answer to a friend's range1 message\n");
		// This is answer to a friend
	}

	return true;
}

static bool basic_mission_ranging_process_range3(
		const struct lgw_pkt_rx_s *pkt_rx,
		const lorawan_endpoint_t *rx_endpoint,
		const Ranging3Payload_t *ranging3_payload) {
	(void) rx_endpoint;

	const uint8_t *rx_payload = pkt_rx->payload;
	const uint16_t rx_payload_size = pkt_rx->size;

	stat_ranging.range3_rx++;

	const uint32_t rx_devaddr = rx_endpoint->devaddr;
	const uint16_t rx_fcnt = lorawan_get_fcnt(rx_payload, rx_payload_size);

	DEBUG("[INFO] Ranging 3 from %08lx (fnct=%u):\n", rx_devaddr, rx_fcnt);
	ranging_payload_range3_printf(ranging3_payload);

	if (ranging3_payload->devaddr2 == lgw_sx130x_endpoint->devaddr) {
		DEBUG("[INFO] This is an answer to my previous range2 message\n");

		// TODO compute ranging3_payload->distance12
		// TODO compute ranging2_payload->distances

	} else {
		DEBUG("[INFO] This is an answer to a friend's range2 message\n");
		// This is answer to a friend
	}

	return true;
}

static void fill_pkt_tx(struct lgw_pkt_tx_s *pkt_tx, const common_tx_t *tx) {
	pkt_tx->tx_mode = tx->status.tx_mode;
	if (tx->status.tx_mode == TIMESTAMPED) {
		pkt_tx->count_us = tx->tx_uscount;
	}
}

bool basic_mission_ranging_process(const struct lgw_pkt_rx_s *pkt_rx,
		struct lgw_pkt_tx_s *pkt_tx, const lorawan_endpoint_t *rx_endpoint, const uint32_t fCntUp) {

	mission_set_default_lgw_pkt_tx(pkt_tx);

	// no packet to send
	pkt_tx->size = 0;

	const uint32_t rx_devaddr = rx_endpoint->devaddr;

	const uint8_t *rx_payload = pkt_rx->payload;
	const uint16_t rx_payload_size = pkt_rx->size;

	const uint8_t rx_fport = lorawan_get_fport(rx_payload, rx_payload_size);
	const uint8_t rx_fcnt = lorawan_get_fcnt(rx_payload, rx_payload_size);
	const uint8_t rx_fpayload_size = lorawan_get_fpayload_size(rx_payload, rx_payload_size);

	uint8_t tx_fpayload[MAX(sizeof(Ranging2Payload_t),sizeof(Ranging3Payload_t))];
	uint8_t tx_fpayload_size;

	uint8_t tx_fport;

	switch (rx_fport) {

	case FPORT_RANGE0_PAYLOAD:
	case FPORT_RANGE1_PAYLOAD: {
		DEBUG(
				"INFO: process RANGE%d message from endpoint 0x%8lx (fport=%d) size=%d\n",
				rx_fport - FPORT_RANGE0_PAYLOAD, rx_devaddr, rx_fport, rx_payload_size);
		if (rx_fpayload_size != sizeof(Ranging01Payload_t)) {
			printf("[ERROR] Bad size for ranging1 payload\n");
			return false;
		} else {
			Ranging01Payload_t ranging1_payload;
			lorawan_payload_decrypt(rx_payload, rx_payload_size, rx_endpoint->appskey,
					rx_devaddr, LORAMAC_DIR_UPLINK, rx_fcnt,
					(uint8_t*) &ranging1_payload);

			Ranging2Payload_t* ranging2_payload =
					(Ranging2Payload_t*) tx_fpayload;
			if (!basic_mission_ranging_process_range1(pkt_rx, rx_endpoint,
					RANGING_TXPOWER, &ranging1_payload, ranging2_payload)) {
				return false;
			}
			stat_ranging.range1_rx++;
			fill_pkt_tx(pkt_tx, &ranging2_payload->tx);
			tx_fpayload_size = sizeof(Ranging2Payload_t);
			tx_fport = FPORT_RANGE2_PAYLOAD;
		}
	}
		break;
	case FPORT_RANGE2_PAYLOAD: {
		DEBUG(
				"INFO: process RANGE%d message from endpoint 0x%8lx (fport=%d) size=%d\n",
				rx_fport - FPORT_RANGE0_PAYLOAD, rx_devaddr, rx_fport, rx_payload_size);
		if (rx_fpayload_size != sizeof(Ranging2Payload_t)) {
			printf("[ERROR] Bad size for ranging2 payload\n");
			return false;
		} else {
			Ranging2Payload_t ranging2_payload;
			lorawan_payload_decrypt(rx_payload, rx_payload_size, rx_endpoint->appskey,
					rx_devaddr, LORAMAC_DIR_UPLINK, rx_fcnt,
					(uint8_t*) &ranging2_payload);

			Ranging3Payload_t* ranging3_payload =
					(Ranging3Payload_t*) tx_fpayload;
			if (!basic_mission_ranging_process_range2(pkt_rx, rx_endpoint,
					RANGING_TXPOWER, &ranging2_payload, ranging3_payload)) {
				return false;
			}
			stat_ranging.range2_rx++;
			fill_pkt_tx(pkt_tx, &ranging3_payload->tx);
			tx_fpayload_size = sizeof(Ranging3Payload_t);
			tx_fport = FPORT_RANGE3_PAYLOAD;
		}

	}
		break;
	case FPORT_RANGE3_PAYLOAD: {
		DEBUG(
				"INFO: process RANGE%d message from endpoint 0x%8lx (fport=%d) size=%d\n",
				rx_fport - FPORT_RANGE0_PAYLOAD, rx_devaddr, rx_fport, rx_payload_size);
		if (rx_fpayload_size != sizeof(Ranging3Payload_t)) {
			printf("[ERROR] Bad size for ranging3 payload\n");
			return false;
		} else {
			Ranging3Payload_t ranging3_payload;
			lorawan_payload_decrypt(rx_payload, rx_payload_size, rx_endpoint->appskey,
					rx_devaddr, LORAMAC_DIR_UPLINK, rx_fcnt,
					(uint8_t*) &ranging3_payload);
			if (!basic_mission_ranging_process_range3(pkt_rx, rx_endpoint,
					&ranging3_payload)) {
				return false;
			}
			stat_ranging.range3_rx++;
			return true;
		}
	}
		break;

	default: {
		DEBUG("INFO: ignore message from endpoint 0x%8lx (fport=%d)\n",
				rx_devaddr, rx_fport);
		return false;
	}
	}

	const uint32_t tx_devaddr = lgw_sx130x_endpoint->devaddr;

	uint8_t pkt_tx_size;
	lorawan_prepare_up_dataframe(
			false, tx_devaddr,
			0x00, // FCTrl (FOptLen = 0)
			fCntUp,
			tx_fport, // fPort
			tx_fpayload, tx_fpayload_size, lgw_sx130x_endpoint->nwkskey,
			lgw_sx130x_endpoint->appskey, pkt_tx->payload, &pkt_tx_size);
			pkt_tx->size = pkt_tx_size;
#if 0
	DEBUG("INFO: set fpayload for ranging (tx_devaddr=%8lx fport=%d size=%d)\n",
			tx_devaddr, tx_fport, pkt_tx_size);
#endif

	return true;

}

