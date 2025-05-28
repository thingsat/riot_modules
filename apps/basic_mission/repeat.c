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

uint16_t uplink_id = 1;
const uint8_t signing_key[] = CHIRPSTACK_MESH_SIGNING_KEY;

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
		printf("INFO: Paylaod is empty : skip received frame\n");
		return;
	}

	if (pkt_rx->modulation != MOD_LORA) {
		// skip received frame
		printf("INFO: modulation is not LoRa : skip received frame\n");
		return;
	}

#if MESHTASTIC == 1
		// check meshtastic_check_valid_frame_size
		// filter meshtastic_get_srcid
		// filter meshtastic_get_destid
#else
	// filter on devaddr
	if (lorawan_check_valid_frame_size(pkt_rx->payload, pkt_rx->size)
			&& lorawan_is_dataframe(pkt_rx->payload, pkt_rx->size)) {
		const uint32_t devaddr = lorawan_get_devaddr(pkt_rx->payload,
				pkt_rx->size);

		if (!IS_BELONGING_TO_NETWORK(devaddr, _devaddr_subnet, _devaddr_mask)) {
			printf(
					"INFO: devaddr %8lx is not belonging to filter : skip received frame\n",
					devaddr);
			return;
		}
	} else {
		printf("INFO: frame is not valid data frames : skip received frame\n");
		return;
	}
#endif
	// filter on SNR
	if (pkt_rx->modulation == MOD_LORA && pkt_rx->snr > _snr_threshold) {
		printf(
				"INFO: SNR (%.1f)  is higher than snr_threshold (%d) : skip received frame\n",
				pkt_rx->snr, _snr_threshold);
		return;
	}

#if CHIRPSTACK_MESH_ENABLE == 1
	printf("INFO: Repeat the received frame into a Chirpstack Mesh uplink\n");

	// TODO if mesage is a chirpstack mesh uplink and hop < CHIRPSTACK_MESH_MAX_HOP

	// TODO check pkt_tx->size < 255 - 14

	uint8_t size;
	lora_mesh_build_uplink(
		pkt_tx->payload,
		&size,
		1,
		uplink_id++,
		12 - pkt_rx->datarate,
		-1*floor(pkt_rx->rssic),
		floor(pkt_rx->snr),
		0, //  channel TODO
		CHIRPSTACK_MESH_RELAY_ID,
		pkt_rx->payload,
		pkt_rx->size,
		signing_key
	);
	pkt_tx->size = size;


	lora_mesh_printf_frame(pkt_tx->payload, pkt_tx->size);
	printf("\n");


#else
	printf("INFO: Repeat the received frame\n");

	memcpy(pkt_tx->payload, pkt_rx->payload, pkt_rx->size);
	pkt_tx->size = pkt_rx->size;

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
	pkt_tx->no_crc = false; 	// LoRaWAN : on for uplink and off for downlink
	pkt_tx->invert_pol = false;


}


/**
 * Filter
 */
void basic_mission_filter(uint32_t devaddr_subnet,
		uint32_t devaddr_mask) {

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
