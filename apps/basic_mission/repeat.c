/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>

#include "repeat.h"
#include "lgw_cmd.h"

#include "lgw_config.h"
#include "lgw_cmd.h"
#include "lgw_endpoint.h"
#include "lorawan_mac.h"
#include "lorawan_printf.h"

//#define CHIRPSTACK_MESH_ENABLE  1

#ifdef CHIRPSTACK_MESH_ENABLE
#include "lora_mesh.h"
#endif

/**
 * Devaddr to repeat
 */
#define DEVADDR_MASK_UNASSIGNED							(0xFFFFFFFF)
#define DEVADDR_MASK_ALL								(0x00000000)
#define DEVADDR_MASK_NETID1								(0xFE000000)
#define DEVADDR_MASK_NETID3								(0xFFFE0000)
#define DEVADDR_MASK_NETID6								(0xFFFFFC00)

static uint32_t _devaddr_subnet = DEVADDR_MASK_ALL;
static uint32_t _devaddr_mask = DEVADDR_MASK_ALL;

#define IS_BELONGING_TO_NETWORK(devaddr,devaddr_subnet,devaddr_mask) ( devaddr_subnet == ( devaddr & devaddr_mask ))

/**
 * SNR threshold to repeat
 */
static int _snr_threshold = 20;

#if CHIRPSTACK_MESH_ENABLE == 1

static uint16_t uplink_id = 1;
static const uint8_t signing_key[] = CHIRPSTACK_MESH_SIGNING_KEY;


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

// TODO set the frequency_plan from the concentrator configuration
static const uint32_t frequency_plan[] = CHIRPSTACK_MESH_CHANNELS;
static const uint32_t frequency_plan_len = ARRAY_SIZE(frequency_plan);

#endif

/**
 * Repeat callback
 */
static void _basic_mission_repeat_cb(const struct lgw_pkt_rx_s *pkt_rx,
		struct lgw_pkt_tx_s *pkt_tx) {

	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);

	uint32_t trig_cnt_us;
	lgw_get_trigcnt(&trig_cnt_us);

	printf("INFO: inst_cnt_us=%ld, trig_cnt_us=%ld\n", inst_cnt_us,
			trig_cnt_us);

	// for skipping received frame
	pkt_tx->size = 0;

	if (pkt_rx->status == STAT_CRC_BAD) {
		// skip received frame
		printf("INFO: status is CRC_BAD : skip received frame\n");
		return;
	}

	if (pkt_rx->size == 0) {
		// skip received frame
		printf("INFO: Payload is empty : skip received frame\n");
		return;
	}

	if (pkt_rx->modulation != MOD_LORA) {
		// skip received frame
		printf("INFO: modulation is not LoRa : skip received frame\n");
		return;
	}

	// filter on SNR
	if (pkt_rx->modulation == MOD_LORA && pkt_rx->snr > _snr_threshold) {
		printf(
				"INFO: SNR (%.1f)  is higher than snr_threshold (%d) : skip received frame\n",
				pkt_rx->snr, _snr_threshold);
		return;
	}

	printf(
			"INFO: SNR (%.1f) is lower than snr_threshold (%d)\n",
			pkt_rx->snr, _snr_threshold);

#if MESHTASTIC == 1
		// check meshtastic_check_valid_frame_size
		// filter meshtastic_get_srcid
		// filter meshtastic_get_destid
#else

#if CHIRPSTACK_MESH_ENABLE == 1

	if (lora_mesh_check_valid_frame(pkt_rx->payload, pkt_rx->size)) {

		uint32_t relay_id = lora_mesh_get_relay_id(pkt_rx->payload,
				pkt_rx->size);
		if (relay_id == CHIRPSTACK_MESH_RELAY_ID) {
			printf(
					"INFO: mesh frame from my relay_id=0x%8lx: skip received frame\n",
					relay_id);
			return;
		}

		printf(
				"INFO: mesh frame from relay_id 0x%8lx\n",
				relay_id);

		const uint8_t hop_count = lora_mesh_get_hop_count(pkt_rx->payload,
				pkt_rx->size);
		if (hop_count > CHIRPSTACK_MESH_MAX_HOP) {
			printf(
					"INFO: frame with hop_count too high %d: skip received frame\n",
					hop_count);
			return;
		}
		printf(
				"INFO: frame with hop_count (%d) lower to max (%d)\n",
				hop_count, CHIRPSTACK_MESH_MAX_HOP);

		// note: for saving cpu cycles, mic is checked after previous tests
		if (!lora_mesh_check_mic(pkt_rx->payload, pkt_rx->size, signing_key)) {
			printf(
					"INFO: mic of relayed mesh frame is not valid : skip received frame\n");
			return;
		}

		printf(
				"INFO: mic of relayed mesh frame is valid\n");

		if (lora_mesh_is_uplink(pkt_rx->payload, pkt_rx->size)) {
			// build new uplink
			uint8_t size;

			const uint8_t channel = lora_mesh_get_channel(pkt_rx->freq_hz, frequency_plan, frequency_plan_len);

			uint8_t lorawan_phypayload_size;
			const uint8_t *lorawan_phypayload = lora_mesh_get_payload(
					pkt_rx->payload, pkt_rx->size, &lorawan_phypayload_size);

			if (lora_mesh_build_uplink(pkt_tx->payload, &size, hop_count + 1, // first hop
			uplink_id++, 12 - pkt_rx->datarate, pkt_rx->rssic, pkt_rx->snr,
			channel,
					CHIRPSTACK_MESH_RELAY_ID, lorawan_phypayload,
					lorawan_phypayload_size, signing_key)) {
				printf(
						"ERROR: Fail to build uplink frame : skip received frame\n");
				return;
			}
			pkt_tx->size = size;

			lora_mesh_printf_frame(pkt_tx->payload, pkt_tx->size);
			printf("\n");

		} else if (lora_mesh_is_downlink(pkt_rx->payload, pkt_rx->size)) {
			printf(
					"WARN: processing mesh downlink frame is not implemented : skip received frame\n");
			return;
		} else if (lora_mesh_is_relay_heartbeat(pkt_rx->payload,
				pkt_rx->size)) {
			printf(
					"WARN: processing mesh relay heartbeat frame is not implemented : skip received frame\n");
			return;
		} else {
			printf(
					"WARN: unknown relay frame : skip received frame\n");
			return;
		}
	} else
#endif
	// case of LoRaWAN frame
	if (lorawan_check_valid_frame_size(pkt_rx->payload, pkt_rx->size)
			&& lorawan_is_dataframe(pkt_rx->payload, pkt_rx->size)) {
		const uint32_t devaddr = lorawan_get_devaddr(pkt_rx->payload,
				pkt_rx->size);
		// filter on devaddr
		if (!IS_BELONGING_TO_NETWORK(devaddr, _devaddr_subnet, _devaddr_mask)) {
			printf(
					"INFO: devaddr %8lx is not belonging to filter : skip received frame\n",
					devaddr);
			return;
		}
		printf(
				"INFO: devaddr %8lx is belonging to filter\n",
				devaddr);


#if CHIRPSTACK_MESH_ENABLE == 1
		printf(
				"INFO: Repeat the received frame into a Chirpstack Mesh uplink\n");

		const uint8_t channel = lora_mesh_get_channel(pkt_rx->freq_hz, frequency_plan, frequency_plan_len);

		uint8_t size;
		if (!lora_mesh_build_uplink(pkt_tx->payload, &size,
				1, // first hop
				uplink_id++, 12 - pkt_rx->datarate, pkt_rx->rssic, pkt_rx->snr,
				channel,
				CHIRPSTACK_MESH_RELAY_ID, pkt_rx->payload, pkt_rx->size,
				signing_key)) {
			printf("ERROR: Fail to build uplink frame : skip received frame\n");
			return;
		}
		pkt_tx->size = size;

		lora_mesh_printf_frame(pkt_tx->payload, pkt_tx->size);
		printf("\n");

#else
		printf("INFO: Repeat the received frame\n");

		memcpy(pkt_tx->payload, pkt_rx->payload, pkt_rx->size);
		pkt_tx->size = pkt_rx->size;

#endif

	} else {
		printf("INFO: frame is not valid data frames : skip received frame\n");
		return;
	}
#endif

	pkt_tx->tx_mode = IMMEDIATE;
	pkt_tx->rf_chain = 0; //only rf_chain 0 is able to tx

	pkt_tx->freq_hz = pkt_rx->freq_hz;
	pkt_tx->datarate = REPEAT_SF; //
	pkt_tx->bandwidth = REPEAT_BW;

#if MESHTASTIC == 1
		if(pkt_rx->freq_hz == 869525000) {
			pkt_tx->rf_power = 27;
		} else {
			pkt_tx->rf_power = REPEAT_TXPOWER;
		}
		pkt_tx->modulation = MOD_LORA;	// ONLY LoRa (No FSK)
		pkt_tx->preamble = 16;
		pkt_tx->coderate = CR_LORA_4_8;
#else
	pkt_tx->rf_power = REPEAT_TXPOWER; //use the single entry of the txlut TODO Should be check
	pkt_tx->modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt_tx->preamble = 8;	//  8 for LoRaWAN
	pkt_tx->coderate = CR_LORA_4_5; // 4/5 for LoRaWAN
#endif

	pkt_tx->no_header = false; 		// Beacons have not header
	pkt_tx->no_crc = false; 		// LoRaWAN : on for uplink and off for downlink
	pkt_tx->invert_pol = false;
}

/**
 * Filter
 */
void basic_mission_filter(uint32_t devaddr_subnet, uint32_t devaddr_mask) {

	_devaddr_subnet = devaddr_subnet;
	_devaddr_mask = devaddr_mask;

	printf("Filter devaddr in subnet: %8lx mask: %8lx\n", _devaddr_subnet,
			_devaddr_mask);
}

/**
 * SNR threshold
 */
void basic_mission_snr_threshold(int snr_threshold) {
	_snr_threshold = snr_threshold;
	printf("SNR threshold : %d\n", _snr_threshold);
}

/**
 * Repeat command
 */
void basic_mission_repeat(bool enable) {

	if (enable) {
		pkt_rx_cb = _basic_mission_repeat_cb;
	} else {
		pkt_rx_cb = NULL;
	}
}
