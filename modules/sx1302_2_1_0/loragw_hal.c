/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
 (______/|_____)_|_|_| \__)_____)\____)_| |_|
 (C)2019 Semtech

 Description:
 LoRa concentrator Hardware Abstraction Layer

 License: Revised BSD License, see LICENSE.TXT file include in the project
 */

/*
 * Port of SX1302 driver for RIOT (v2.1.0)
 *
 * Author:
 *   Didier DONSEZ, Université Grenoble Alpes, 2021-2023.
 */

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

/* fix an issue between POSIX and C99 */
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif

#define _GNU_SOURCE     /* needed for qsort_r to be defined */
#include <stdlib.h>     /* qsort_r */

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf */
#include <string.h>     /* memcpy */
#ifndef RIOT_APPLICATION
#include <unistd.h>     /* symlink, unlink */
#else
#include <math.h>		/* pow */
#endif

#include <inttypes.h>

#ifdef RIOT_APPLICATION
#include "board.h"
#endif
#include "loragw_reg.h"
#include "loragw_hal.h"
#include "loragw_aux.h"
#include "loragw_com.h"

#if ENABLE_LBT == 1
#include "loragw_lbt.h"
#include "loragw_sx1261.h"
#endif

#ifndef RIOT_APPLICATION
#include "loragw_i2c.h"
#endif

#if ENABLE_SX1250 == 1
#include "loragw_sx1250.h"
#endif

#if ENABLE_SX125X == 1
#include "loragw_sx125x.h"
#endif

#ifndef RIOT_APPLICATION
#include "loragw_sx1261.h"
#endif
#include "loragw_sx1302.h"
#include "loragw_sx1302_timestamp.h"
#ifndef RIOT_APPLICATION
#include "loragw_stts751.h"
#include "loragw_ad5338r.h"
#endif
#include "loragw_debug.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if DEBUG_HAL == 1
#define DEBUG_MSG(str)              printf(str)
#define DEBUG_PRINTF(fmt, args...)  printf(fmt, args)
#define CHECK_NULL(a)               if(a==NULL){printf("[%s:%d] ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_REG_ERROR;}
#define DEBUG_ARRAY(a,b,c)           for(a=0;a<b;++a) printf("%x.",c[a]);fprintf(stdout,"end\n", __FUNCTION__, __LINE__)
#else
#define DEBUG_MSG(str)
#define DEBUG_PRINTF(fmt, args...)
#define DEBUG_ARRAY(a,b,c)            for(a=0;a!=0;){}
#define CHECK_NULL(a)                 if(a==NULL){return LGW_HAL_ERROR;}
#endif

#define TRACE()             fprintf(stderr, "@ %s %d\n", __FUNCTION__, __LINE__);

#define CONTEXT_STARTED         lgw_context.is_started
#define CONTEXT_COM_TYPE        lgw_context.board_cfg.com_type
#define CONTEXT_COM_PATH        lgw_context.board_cfg.com_path
#define CONTEXT_LWAN_PUBLIC     lgw_context.board_cfg.lorawan_public
#define CONTEXT_BOARD           lgw_context.board_cfg
#define CONTEXT_RF_CHAIN        lgw_context.rf_chain_cfg
#define CONTEXT_IF_CHAIN        lgw_context.if_chain_cfg
#define CONTEXT_DEMOD           lgw_context.demod_cfg
#define CONTEXT_LORA_SERVICE    lgw_context.lora_service_cfg
#define CONTEXT_FSK             lgw_context.fsk_cfg
#define CONTEXT_TX_GAIN_LUT     lgw_context.tx_gain_lut
#define CONTEXT_FINE_TIMESTAMP  lgw_context.ftime_cfg
#define CONTEXT_SX1261          lgw_context.sx1261_cfg
#define CONTEXT_DEBUG           lgw_context.debug_cfg

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */

#define FW_VERSION_AGC_SX1250   10 /* Expected version of AGC firmware for sx1250 based gateway */
/* v10 is same as v6 with improved channel check time for LBT */
#define FW_VERSION_AGC_SX125X   6  /* Expected version of AGC firmware for sx1255/sx1257 based gateway */
#define FW_VERSION_ARB          2  /* Expected version of arbiter firmware */

/* Useful bandwidth of SX125x radios to consider depending on channel bandwidth */
/* Note: the below values come from lab measurements. For any question, please contact Semtech support */
#define LGW_RF_RX_BANDWIDTH_125KHZ  1600000     /* for 125KHz channels */
#define LGW_RF_RX_BANDWIDTH_250KHZ  1600000     /* for 250KHz channels */
#define LGW_RF_RX_BANDWIDTH_500KHZ  1600000     /* for 500KHz channels */

#define LGW_RF_RX_FREQ_MIN          100E6
#define LGW_RF_RX_FREQ_MAX          1E9

/* Version string, used to identify the library version/options once compiled */
const char lgw_version_string[] =
		"Version: " LIBLORAGW_VERSION "; " LIBLORAGW_RELEASE_DATE "; (RIOT port)";

#include "arb_fw.var"           /* text_arb_sx1302_13_Nov_3 */
#if ENABLE_SX1250 == 1
#include "agc_fw_sx1250.var"    /* text_agc_sx1250_05_Juillet_2019_3 */
#endif
#if ENABLE_SX125X == 1
#include "agc_fw_sx1257.var"    /* text_agc_sx1257_19_Nov_1 */
#endif

#include "lgw_context.var"

#ifndef RIOT_APPLICATION
/* File handle to write debug logs */
FILE * log_file = NULL;

/* I2C temperature sensor handles */
static int     ts_fd = -1;
static uint8_t ts_addr = 0xFF;

/* I2C AD5338 handles */
static int     ad_fd = -1;
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */

int32_t lgw_sf_getval(int x);
int32_t lgw_bw_getval(int x);

static bool is_same_pkt(struct lgw_pkt_rx_s *p1, struct lgw_pkt_rx_s *p2);
static int remove_pkt(struct lgw_pkt_rx_s *p, uint8_t *nb_pkt,
		uint8_t pkt_index);
static int merge_packets(struct lgw_pkt_rx_s *p, uint8_t *nb_pkt);

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

int32_t lgw_bw_getval(int x) {
	switch (x) {
	case BW_500KHZ:
		return 500000;
	case BW_250KHZ:
		return 250000;
	case BW_125KHZ:
		return 125000;
	default:
		return -1;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t lgw_sf_getval(int x) {
	switch (x) {
	case DR_LORA_SF5:
		return 5;
	case DR_LORA_SF6:
		return 6;
	case DR_LORA_SF7:
		return 7;
	case DR_LORA_SF8:
		return 8;
	case DR_LORA_SF9:
		return 9;
	case DR_LORA_SF10:
		return 10;
	case DR_LORA_SF11:
		return 11;
	case DR_LORA_SF12:
		return 12;
	default:
		return -1;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static bool is_same_pkt(struct lgw_pkt_rx_s *p1, struct lgw_pkt_rx_s *p2) {
	if ((p1 != NULL) && (p2 != NULL)) {
		/* Criterias to determine if packets are identical:
		 -- count_us should be equal or can have up to 24µs of difference (3 samples)
		 -- channel should be same
		 -- datarate should be same
		 -- payload should be same
		 */
		if ((abs((int32_t) p1->count_us - (int32_t) p2->count_us) <= 24)
				&& (p1->if_chain == p2->if_chain)
				&& (p1->datarate == p2->datarate) && (p1->size == p2->size)
				&& (memcmp(p1->payload, p2->payload, p1->size) == 0)) {

			return true;
		}
	}

	return false;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int remove_pkt(struct lgw_pkt_rx_s *p, uint8_t *nb_pkt,
		uint8_t pkt_index) {
	/* Check input parameters */
	CHECK_NULL(p);
	CHECK_NULL(nb_pkt);
	if (pkt_index > ((*nb_pkt) - 1)) {
		printf("[%s:%d] ERROR: failed to remove packet index %u\n",
				__FUNCTION__, __LINE__, pkt_index);
		return -1;
	}

	/* Remove pkt from array, by replacing it with last packet of array */
	if (pkt_index == ((*nb_pkt) - 1)) {
		/* If we remove last element, just decrement nb packet counter */
		/* Do nothing */
	} else {
		/* Copy last packet onto the packet to be removed */
		memcpy(p + pkt_index, p + (*nb_pkt) - 1, sizeof(struct lgw_pkt_rx_s));
	}

	*nb_pkt -= 1;

	return 0;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int compare_pkt_tmst(const void *a, const void *b, void *arg) {
	struct lgw_pkt_rx_s *p = (struct lgw_pkt_rx_s*) a;
	struct lgw_pkt_rx_s *q = (struct lgw_pkt_rx_s*) b;
	int *counter = (int*) arg;
	int p_count, q_count;

	p_count = p->count_us;
	q_count = q->count_us;

	if (p_count > q_count) {
		*counter = *counter + 1;
	}

	return (p_count - q_count);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static int merge_packets(struct lgw_pkt_rx_s *p, uint8_t *nb_pkt) {
	uint8_t cpt;
	int j, k, pkt_dup_idx, x;
#if DEBUG_HAL == 1
	int pkt_idx;
#endif
	bool dup_restart = false;
	int counter_qsort_swap = 0;

	/* Check input parameters */
	CHECK_NULL(p);
	CHECK_NULL(nb_pkt);

	/* Init number of packets in array before merge */
	cpt = *nb_pkt;

	/* --------------------------------------------- */
	/* ---------- For Debug only - START ----------- */
	if (cpt > 0) {
		DEBUG_PRINTF("[%s:%d] <----- Searching for DUPLICATEs ------\n", __FUNCTION__, __LINE__);
	}
	for (j = 0; j < cpt; j++) {
		DEBUG_PRINTF("[%s:%d]   %d: tmst=%lu SF=%lu CRC_status=%d freq=%lu chan=%u", __FUNCTION__, __LINE__, j, p[j].count_us, p[j].datarate, p[j].status, p[j].freq_hz, p[j].if_chain);
		if (p[j].ftime_received == true) {
			DEBUG_PRINTF("[%s:%d]  ftime=%lu\n", __FUNCTION__, __LINE__, p[j].ftime);
		} else {
			DEBUG_PRINTF("[%s:%d] ftime=NONE\n", __FUNCTION__, __LINE__);
		}
	}
	/* ---------- For Debug only - END ------------- */
	/* --------------------------------------------- */

	/* Remove duplicates */
	j = 0;
	while (j < cpt) {
		for (k = (j + 1); k < cpt; k++) {
			/* Searching for duplicated packets:
			 -- count_us should be equal or can have up to 24µs of difference (3 samples)
			 -- channel should be same
			 -- datarate should be same
			 -- payload should be same
			 */
			if (is_same_pkt(&p[j], &p[k])) {
				/* We keep the packet which has CRC checked */
				if ((p[j].status == STAT_CRC_OK)
						&& (p[k].status == STAT_CRC_BAD)) {
					pkt_dup_idx = k;
#if DEBUG_HAL == 1
					pkt_idx = j;
#endif
				} else if ((p[j].status == STAT_CRC_BAD)
						&& (p[k].status == STAT_CRC_OK)) {
					pkt_dup_idx = j;
#if DEBUG_HAL == 1
					pkt_idx = k;
#endif
				} else {
					/* we keep the packet which has a fine timestamp */
					if (p[j].ftime_received == true) {
						pkt_dup_idx = k;
#if DEBUG_HAL == 1
						pkt_idx = j;
#endif
					} else {
						pkt_dup_idx = j;
#if DEBUG_HAL == 1
						pkt_idx = k;
#endif
					}
					/* sanity check */
					if (((p[j].ftime_received == true)
							&& (p[k].ftime_received == true))
							|| ((p[j].ftime_received == false)
									&& (p[k].ftime_received == false))) {
						DEBUG_PRINTF("[%s:%d] WARNING: both duplicates have fine timestamps, or none has ? TBC\n", __FUNCTION__, __LINE__);
					}
				}
				/* pkt_dup_idx contains the index to be deleted */
				DEBUG_PRINTF("[%s:%d] duplicate found %d:%d, deleting %d\n", __FUNCTION__, __LINE__, pkt_idx, pkt_dup_idx, pkt_dup_idx);
				/* Remove duplicated packet from packet array */
				x = remove_pkt(p, &cpt, pkt_dup_idx);
				if (x != 0) {
					printf(
							"[%s:%d] ERROR: failed to remove packet from array (%d)\n",
							__FUNCTION__, __LINE__, x);
				}
				dup_restart = true;
				break;
			}
		}
		if (dup_restart == true) {
			/* Duplicate found, restart searching for duplicate from first element */
			j = 0;
			dup_restart = false;
#if 0
			DEBUG_PRINTF("[%s:%d] restarting search for duplicate\n", __FUNCTION__, __LINE__); /* Too verbose */
#endif
		} else {
			/* No duplicate found, continue... */
			j += 1;
#if 0
			DEBUG_PRINTF("[%s:%d] no duplicate found\n", __FUNCTION__, __LINE__); /* Too verbose */
#endif
		}
	}

	/* Sort the packet array by ascending counter_us value */
	qsort_r(p, cpt, sizeof(p[0]), compare_pkt_tmst, &counter_qsort_swap);
	DEBUG_PRINTF("[%s:%d] %d elements swapped during sorting...\n", __FUNCTION__, __LINE__, counter_qsort_swap);

	/* --------------------------------------------- */
	/* ---------- For Debug only - START ----------- */
	if (cpt > 0) {
		DEBUG_PRINTF("[%s:%d]--\n\n", __FUNCTION__, __LINE__ );
	}
	for (j = 0; j < cpt; j++) {
		DEBUG_PRINTF("[%s:%d]   %d: tmst=%lu SF=%lu CRC_status=%d freq=%lu chan=%u", __FUNCTION__, __LINE__, j, p[j].count_us, p[j].datarate, p[j].status, p[j].freq_hz, p[j].if_chain);
		if (p[j].ftime_received == true) {
			DEBUG_PRINTF("[%s:%d]  ftime=%lu\n", __FUNCTION__, __LINE__, p[j].ftime);
		} else {
			DEBUG_PRINTF("[%s:%d]  ftime=NONE\n", __FUNCTION__, __LINE__);
		}
	}
	if (cpt > 0) {
		DEBUG_PRINTF("[%s:%d] ------------------------------------>\n\n", __FUNCTION__, __LINE__ );
	}
	/* ---------- For Debug only - END ------------- */
	/* --------------------------------------------- */

	/* Update number of packets contained in packet array */
	*nb_pkt = cpt;

	return 0;
}

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int lgw_board_setconf(struct lgw_conf_board_s *conf) {
	CHECK_NULL(conf);

	/* check if the concentrator is running */
	if (CONTEXT_STARTED == true) {
		printf(
				"[%s:%d] ERROR: CONCENTRATOR IS RUNNING, STOP IT BEFORE TOUCHING CONFIGURATION\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Check input parameters */
	if ((conf->com_type != LGW_COM_SPI) && (conf->com_type != LGW_COM_USB)) {
		printf("[%s:%d] ERROR: WRONG COM TYPE\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* set internal config according to parameters */
	CONTEXT_LWAN_PUBLIC = conf->lorawan_public;
	CONTEXT_BOARD.clksrc = conf->clksrc;
	CONTEXT_BOARD.full_duplex = conf->full_duplex;
	CONTEXT_COM_TYPE = conf->com_type;
	strncpy(CONTEXT_COM_PATH, conf->com_path, sizeof CONTEXT_COM_PATH);
	CONTEXT_COM_PATH[sizeof CONTEXT_COM_PATH - 1] = '\0'; /* ensure string termination */

	DEBUG_PRINTF("[%s:%d] Note: board configuration: com_type: %s, com_path: %s, lorawan_public:%d, clksrc:%d, full_duplex:%d\n", __FUNCTION__, __LINE__, (CONTEXT_COM_TYPE == LGW_COM_SPI) ? "SPI" : "USB",
			CONTEXT_COM_PATH,
			CONTEXT_LWAN_PUBLIC,
			CONTEXT_BOARD.clksrc,
			CONTEXT_BOARD.full_duplex);

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_rxrf_setconf(uint8_t rf_chain, struct lgw_conf_rxrf_s *conf) {
	CHECK_NULL(conf);

	/* check if the concentrator is running */
	if (CONTEXT_STARTED == true) {
		printf(
				"[%s:%d] ERROR: CONCENTRATOR IS RUNNING, STOP IT BEFORE TOUCHING CONFIGURATION\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	if (conf->enable == false) {
		/* nothing to do */
		DEBUG_PRINTF("[%s:%d] Note: rf_chain %d disabled\n", __FUNCTION__, __LINE__, rf_chain);
		return LGW_HAL_SUCCESS;
	}

	/* check input range (segfault prevention) */
	if (rf_chain >= LGW_RF_CHAIN_NB) {
		printf("[%s:%d] ERROR: NOT A VALID RF_CHAIN NUMBER\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* check if radio type is supported */
	if ((conf->type != LGW_RADIO_TYPE_SX1255)
			&& (conf->type != LGW_RADIO_TYPE_SX1257)
			&& (conf->type != LGW_RADIO_TYPE_SX1250)) {
		printf("[%s:%d] ERROR: NOT A VALID RADIO TYPE (%d)\n", __FUNCTION__,
				__LINE__, conf->type);
		return LGW_HAL_ERROR;
	}

	/* check if the radio central frequency is valid */
	if ((conf->freq_hz < LGW_RF_RX_FREQ_MIN)
			|| (conf->freq_hz > LGW_RF_RX_FREQ_MAX)) {
		printf(
				"[%s:%d] ERROR: NOT A VALID RADIO CENTER FREQUENCY, PLEASE CHECK IF IT HAS BEEN GIVEN IN HZ (%lu)\n",
				__FUNCTION__, __LINE__, conf->freq_hz);
		return LGW_HAL_ERROR;
	}

	/* set internal config according to parameters */
	CONTEXT_RF_CHAIN[rf_chain].enable = conf->enable;
	CONTEXT_RF_CHAIN[rf_chain].freq_hz = conf->freq_hz;
	CONTEXT_RF_CHAIN[rf_chain].rssi_offset = conf->rssi_offset;
	CONTEXT_RF_CHAIN[rf_chain].rssi_tcomp.coeff_a = conf->rssi_tcomp.coeff_a;
	CONTEXT_RF_CHAIN[rf_chain].rssi_tcomp.coeff_b = conf->rssi_tcomp.coeff_b;
	CONTEXT_RF_CHAIN[rf_chain].rssi_tcomp.coeff_c = conf->rssi_tcomp.coeff_c;
	CONTEXT_RF_CHAIN[rf_chain].rssi_tcomp.coeff_d = conf->rssi_tcomp.coeff_d;
	CONTEXT_RF_CHAIN[rf_chain].rssi_tcomp.coeff_e = conf->rssi_tcomp.coeff_e;
	CONTEXT_RF_CHAIN[rf_chain].type = conf->type;
	CONTEXT_RF_CHAIN[rf_chain].tx_enable = conf->tx_enable;
	CONTEXT_RF_CHAIN[rf_chain].single_input_mode = conf->single_input_mode;

	DEBUG_PRINTF("[%s:%d] Note: rf_chain %d configuration; en:%d freq:%ld rssi_offset:%f radio_type:%d tx_enable:%d single_input_mode:%d\n", __FUNCTION__, __LINE__, rf_chain,
			CONTEXT_RF_CHAIN[rf_chain].enable,
			CONTEXT_RF_CHAIN[rf_chain].freq_hz,
			CONTEXT_RF_CHAIN[rf_chain].rssi_offset,
			CONTEXT_RF_CHAIN[rf_chain].type,
			CONTEXT_RF_CHAIN[rf_chain].tx_enable,
			CONTEXT_RF_CHAIN[rf_chain].single_input_mode);

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_rxif_setconf(uint8_t if_chain, struct lgw_conf_rxif_s *conf) {
	int32_t bw_hz;
	uint32_t rf_rx_bandwidth;

	CHECK_NULL(conf);

	/* check if the concentrator is running */
	if (CONTEXT_STARTED == true) {
		printf(
				"[%s:%d] ERROR: CONCENTRATOR IS RUNNING, STOP IT BEFORE TOUCHING CONFIGURATION\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* check input range (segfault prevention) */
	if (if_chain >= LGW_IF_CHAIN_NB) {
		printf("[%s:%d] ERROR: %d NOT A VALID IF_CHAIN NUMBER\n", __FUNCTION__,
				__LINE__, if_chain);
		return LGW_HAL_ERROR;
	}

	/* if chain is disabled, don't care about most parameters */
	if (conf->enable == false) {
		CONTEXT_IF_CHAIN[if_chain].enable = false;
		CONTEXT_IF_CHAIN[if_chain].freq_hz = 0;
		DEBUG_PRINTF("[%s:%d] Note: if_chain %d disabled\n", __FUNCTION__, __LINE__, if_chain);
		return LGW_HAL_SUCCESS;
	}

	/* check 'general' parameters */
	if (sx1302_get_ifmod_config(if_chain) == IF_UNDEFINED) {
		printf("[%s:%d] ERROR: IF CHAIN %d NOT CONFIGURABLE\n", __FUNCTION__,
				__LINE__, if_chain);
	}
	if (conf->rf_chain >= LGW_RF_CHAIN_NB) {
		printf(
				"[%s:%d] ERROR: INVALID RF_CHAIN TO ASSOCIATE WITH A LORA_STD IF CHAIN\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
	/* check if IF frequency is optimal based on channel and radio bandwidths */
	switch (conf->bandwidth) {
	case BW_250KHZ:
		rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_250KHZ; /* radio bandwidth */
		break;
	case BW_500KHZ:
		rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_500KHZ; /* radio bandwidth */
		break;
	default:
		/* For 125KHz and below */
		rf_rx_bandwidth = LGW_RF_RX_BANDWIDTH_125KHZ; /* radio bandwidth */
		break;
	}
	bw_hz = lgw_bw_getval(conf->bandwidth); /* channel bandwidth */
	if ((conf->freq_hz + ((bw_hz == -1) ? LGW_REF_BW : bw_hz) / 2)
			> ((int32_t) rf_rx_bandwidth / 2)) {
		printf("[%s:%d] ERROR: IF FREQUENCY %ld TOO HIGH\n", __FUNCTION__,
				__LINE__, conf->freq_hz);
		return LGW_HAL_ERROR;
	} else if ((conf->freq_hz - ((bw_hz == -1) ? LGW_REF_BW : bw_hz) / 2)
			< -((int32_t) rf_rx_bandwidth / 2)) {
		printf("[%s:%d] ERROR: IF FREQUENCY %ld TOO LOW\n", __FUNCTION__,
				__LINE__, conf->freq_hz);
		return LGW_HAL_ERROR;
	}

	/* check parameters according to the type of IF chain + modem,
	 fill default if necessary, and commit configuration if everything is OK */
	switch (sx1302_get_ifmod_config(if_chain)) {
	case IF_LORA_STD:
		/* fill default parameters if needed */
		if (conf->bandwidth == BW_UNDEFINED) {
			conf->bandwidth = BW_250KHZ;
		}
		if (conf->datarate == DR_UNDEFINED) {
			conf->datarate = DR_LORA_SF7;
		}
		/* check BW & DR */
		if (!IS_LORA_BW(conf->bandwidth)) {
			printf(
					"[%s:%d] ERROR: BANDWIDTH NOT SUPPORTED BY LORA_STD IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_LORA_DR(conf->datarate)) {
			printf(
					"[%s:%d] ERROR: DATARATE NOT SUPPORTED BY LORA_STD IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		/* set internal configuration  */
		CONTEXT_IF_CHAIN[if_chain].enable = conf->enable;
		CONTEXT_IF_CHAIN[if_chain].rf_chain = conf->rf_chain;
		CONTEXT_IF_CHAIN[if_chain].freq_hz = conf->freq_hz;
		CONTEXT_LORA_SERVICE.bandwidth = conf->bandwidth;
		CONTEXT_LORA_SERVICE.datarate = conf->datarate;
		CONTEXT_LORA_SERVICE.implicit_hdr = conf->implicit_hdr;
		CONTEXT_LORA_SERVICE.implicit_payload_length =
				conf->implicit_payload_length;
		CONTEXT_LORA_SERVICE.implicit_crc_en = conf->implicit_crc_en;
		CONTEXT_LORA_SERVICE.implicit_coderate = conf->implicit_coderate;

		DEBUG_PRINTF("[%s:%d] Note: LoRa 'std' if_chain %d configuration; en:%d freq:%ld bw:%d dr:%ld\n", __FUNCTION__, __LINE__, if_chain,
				CONTEXT_IF_CHAIN[if_chain].enable,
				CONTEXT_IF_CHAIN[if_chain].freq_hz,
				CONTEXT_LORA_SERVICE.bandwidth,
				CONTEXT_LORA_SERVICE.datarate);
		break;

	case IF_LORA_MULTI:
		/* fill default parameters if needed */
		if (conf->bandwidth == BW_UNDEFINED) {
			conf->bandwidth = BW_125KHZ;
		}
		if (conf->datarate == DR_UNDEFINED) {
			conf->datarate = DR_LORA_SF7;
		}
		/* check BW & DR */
		if (conf->bandwidth != BW_125KHZ) {
			printf(
					"[%s:%d] ERROR: BANDWIDTH NOT SUPPORTED BY LORA_MULTI IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_LORA_DR(conf->datarate)) {
			printf(
					"[%s:%d] ERROR: DATARATE(S) NOT SUPPORTED BY LORA_MULTI IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		/* set internal configuration  */
		CONTEXT_IF_CHAIN[if_chain].enable = conf->enable;
		CONTEXT_IF_CHAIN[if_chain].rf_chain = conf->rf_chain;
		CONTEXT_IF_CHAIN[if_chain].freq_hz = conf->freq_hz;

		DEBUG_PRINTF("[%s:%d] Note: LoRa 'multi' if_chain %d configuration; en:%d freq:%ld\n", __FUNCTION__, __LINE__, if_chain,
				CONTEXT_IF_CHAIN[if_chain].enable,
				CONTEXT_IF_CHAIN[if_chain].freq_hz);
		break;

	case IF_FSK_STD:
		/* fill default parameters if needed */
		if (conf->bandwidth == BW_UNDEFINED) {
			conf->bandwidth = BW_250KHZ;
		}
		if (conf->datarate == DR_UNDEFINED) {
			conf->datarate = 64000; /* default datarate */
		}
		/* check BW & DR */
		if (!IS_FSK_BW(conf->bandwidth)) {
			printf("[%s:%d] ERROR: BANDWIDTH NOT SUPPORTED BY FSK IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_FSK_DR(conf->datarate)) {
			printf("[%s:%d] ERROR: DATARATE NOT SUPPORTED BY FSK IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		/* set internal configuration  */
		CONTEXT_IF_CHAIN[if_chain].enable = conf->enable;
		CONTEXT_IF_CHAIN[if_chain].rf_chain = conf->rf_chain;
		CONTEXT_IF_CHAIN[if_chain].freq_hz = conf->freq_hz;
		CONTEXT_FSK.bandwidth = conf->bandwidth;
		CONTEXT_FSK.datarate = conf->datarate;
		if (conf->sync_word > 0) {
			CONTEXT_FSK.sync_word_size = conf->sync_word_size;
			CONTEXT_FSK.sync_word = conf->sync_word;
		}
		DEBUG_PRINTF("[%s:%d] Note: FSK if_chain %d configuration; en:%ld freq:%d bw:%ld dr:%ld (%d real dr) sync:0x%0llx"  "\n",
				__FUNCTION__, __LINE__,
                CONTEXT_IF_CHAIN[if_chain].enable,
                CONTEXT_IF_CHAIN[if_chain].freq_hz,
                CONTEXT_FSK.bandwidth,
                CONTEXT_FSK.datarate,
                LGW_XTAL_FREQU/(LGW_XTAL_FREQU/CONTEXT_FSK.datarate),
                2*CONTEXT_FSK.sync_word_size,
                CONTEXT_FSK.sync_word);
		break;

	default:
		printf("[%s:%d] ERROR: IF CHAIN %d TYPE NOT SUPPORTED\n", __FUNCTION__,
				__LINE__, if_chain);
		return LGW_HAL_ERROR;
	}

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_demod_setconf(struct lgw_conf_demod_s *conf) {
	CHECK_NULL(conf);

	CONTEXT_DEMOD.multisf_datarate = conf->multisf_datarate;

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static void printf_lut(const struct lgw_tx_gain_s *lut) {
	printf("rf_power=%d, pwr_idx=%d, pa_gain=%d, dig_gain=%d, dac_gain=%d, mix_gain=%d, offset_i=%d, offset_q=%d",
			lut->rf_power, lut->pwr_idx, lut->pa_gain, lut->dig_gain, lut->dac_gain, lut->mix_gain, lut->offset_i, lut->offset_q);
}


int lgw_txgain_setconf(uint8_t rf_chain, struct lgw_tx_gain_lut_s *conf) {
	int i;

	CHECK_NULL(conf);

	/* Check LUT size */
	if ((conf->size < 1) || (conf->size > TX_GAIN_LUT_SIZE_MAX)) {
		printf(
				"[%s:%d] ERROR: TX gain LUT must have at least one entry and  maximum %d entries\n",
				__FUNCTION__, __LINE__, TX_GAIN_LUT_SIZE_MAX);
		return LGW_HAL_ERROR;
	}

	CONTEXT_TX_GAIN_LUT[rf_chain].size = conf->size;

	for (i = 0; i < CONTEXT_TX_GAIN_LUT[rf_chain].size; i++) {
		/* Check gain range */
		if (conf->lut[i].dig_gain > 3) {
			printf(
					"[%s:%d] ERROR: TX gain LUT: SX1302 digital gain must be between 0 and 3\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (conf->lut[i].dac_gain > 3) {
			printf(
					"[%s:%d] ERROR: TX gain LUT: SX1257 DAC gains must not exceed 3\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if ((conf->lut[i].mix_gain < 5) || (conf->lut[i].mix_gain > 15)) {
			printf(
					"[%s:%d] ERROR: TX gain LUT: SX1257 mixer gain must be betwen [5..15]\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (conf->lut[i].pa_gain > 3) {
			printf(
					"[%s:%d] ERROR: TX gain LUT: External PA gain must not exceed 3\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (conf->lut[i].pwr_idx > 22) {
			printf(
					"[%s:%d] ERROR: TX gain LUT: SX1250 power index must not exceed 22\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}


		DEBUG_PRINTF(
				"[%s:%d] INFO: Set internal LUT rfchain=%d i=%d\n",
				__FUNCTION__, __LINE__, rf_chain, i);

		/* Set internal LUT */
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].rf_power = conf->lut[i].rf_power;
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].dig_gain = conf->lut[i].dig_gain;
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].pa_gain = conf->lut[i].pa_gain;
		/* sx125x */
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].dac_gain = conf->lut[i].dac_gain;
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].mix_gain = conf->lut[i].mix_gain;
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].offset_i = 0; /* To be calibrated */
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].offset_q = 0; /* To be calibrated */

		/* sx1250 */
		CONTEXT_TX_GAIN_LUT[rf_chain].lut[i].pwr_idx = conf->lut[i].pwr_idx;

/*		DEBUG_PRINTF(
				"[%s:%d] INFO: Internal LUT rfchain=%d i=%d : ",
				__FUNCTION__, __LINE__, rf_chain, i);
		printf_lut(CONTEXT_TX_GAIN_LUT[rf_chain].lut + i);
		printf("\n");
*/
	}

	return LGW_HAL_SUCCESS;
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_ftime_setconf(struct lgw_conf_ftime_s *conf) {
	CHECK_NULL(conf);

	CONTEXT_FINE_TIMESTAMP.enable = conf->enable;
	CONTEXT_FINE_TIMESTAMP.mode = conf->mode;

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_sx1261_setconf(struct lgw_conf_sx1261_s *conf) {
	int i;

	CHECK_NULL(conf);

	/* Set the SX1261 global conf */
	CONTEXT_SX1261.enable = conf->enable;
	strncpy(CONTEXT_SX1261.spi_path, conf->spi_path,
			sizeof CONTEXT_SX1261.spi_path);
	CONTEXT_SX1261.spi_path[sizeof CONTEXT_SX1261.spi_path - 1] = '\0'; /* ensure string termination */
	CONTEXT_SX1261.rssi_offset = conf->rssi_offset;

	/* Set the LBT conf */
	CONTEXT_SX1261.lbt_conf.enable = conf->lbt_conf.enable;
	CONTEXT_SX1261.lbt_conf.rssi_target = conf->lbt_conf.rssi_target;
	CONTEXT_SX1261.lbt_conf.nb_channel = conf->lbt_conf.nb_channel;
	for (i = 0; i < CONTEXT_SX1261.lbt_conf.nb_channel; i++) {
		if (conf->lbt_conf.channels[i].bandwidth != BW_125KHZ
				&& conf->lbt_conf.channels[i].bandwidth != BW_250KHZ) {
			printf(
					"[%s:%d] ERROR: bandwidth not supported for LBT channel %d\n",
					__FUNCTION__, __LINE__, i);
			return LGW_HAL_ERROR;
		}
		if (conf->lbt_conf.channels[i].scan_time_us != LGW_LBT_SCAN_TIME_128_US
				&& conf->lbt_conf.channels[i].scan_time_us
						!= LGW_LBT_SCAN_TIME_5000_US) {
			printf(
					"[%s:%d] ERROR: scan_time_us not supported for LBT channel %d\n",
					__FUNCTION__, __LINE__, i);
			return LGW_HAL_ERROR;
		}
		CONTEXT_SX1261.lbt_conf.channels[i] = conf->lbt_conf.channels[i];
	}

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_debug_setconf(struct lgw_conf_debug_s *conf) {
	int i;

	CHECK_NULL(conf);

	CONTEXT_DEBUG.nb_ref_payload = conf->nb_ref_payload;
	for (i = 0; i < CONTEXT_DEBUG.nb_ref_payload; i++) {
		/* Get user configuration */
		CONTEXT_DEBUG.ref_payload[i].id = conf->ref_payload[i].id;

		/* Initialize global context */
		CONTEXT_DEBUG.ref_payload[i].prev_cnt = 0;
		CONTEXT_DEBUG.ref_payload[i].payload[0] = (uint8_t)(
				CONTEXT_DEBUG.ref_payload[i].id >> 24);
		CONTEXT_DEBUG.ref_payload[i].payload[1] = (uint8_t)(
				CONTEXT_DEBUG.ref_payload[i].id >> 16);
		CONTEXT_DEBUG.ref_payload[i].payload[2] = (uint8_t)(
				CONTEXT_DEBUG.ref_payload[i].id >> 8);
		CONTEXT_DEBUG.ref_payload[i].payload[3] = (uint8_t)(
				CONTEXT_DEBUG.ref_payload[i].id >> 0);
	}
#ifndef RIOT_APPLICATION
	if (conf->log_file_name != NULL) {
		strncpy(CONTEXT_DEBUG.log_file_name, conf->log_file_name, sizeof CONTEXT_DEBUG.log_file_name);
		CONTEXT_DEBUG.log_file_name[sizeof CONTEXT_DEBUG.log_file_name - 1] = '\0'; /* ensure string termination */
	}
#endif
	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#if ENABLE_REGTEST == 1
int lgw_start_for_regtest(void) {
	int err;

	if (CONTEXT_STARTED == true) {
		DEBUG_PRINTF("[%s:%d] WARNING: LoRa concentrator already started, restarting it now\n", __FUNCTION__, __LINE__);
	}

	err = lgw_connect(CONTEXT_COM_TYPE, CONTEXT_COM_PATH);
	if (err == LGW_REG_ERROR) {
		printf("[%s:%d] ERROR: FAIL TO CONNECT BOARD\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
	/* set hal state */
	CONTEXT_STARTED = true;

	return LGW_HAL_SUCCESS;
}
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_start(void) {
	int i, err;
	uint8_t fw_version_agc;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	if (CONTEXT_STARTED == true) {
		DEBUG_PRINTF("[%s:%d] WARNING: LoRa concentrator already started, restarting it now\n", __FUNCTION__, __LINE__);
	}

	err = lgw_connect(CONTEXT_COM_TYPE, CONTEXT_COM_PATH);
	if (err == LGW_REG_ERROR) {
		printf("[%s:%d] ERROR: FAIL TO CONNECT BOARD\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Set all GPIOs to 0 */
	err = sx1302_set_gpio(0x00);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to set all GPIOs to 0\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Calibrate radios */
	err = sx1302_radio_calibrate(&CONTEXT_RF_CHAIN[0], CONTEXT_BOARD.clksrc,
			&CONTEXT_TX_GAIN_LUT[0]);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: radio calibration failed\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}
	printf("INFO: radio is calibrated\n");

	/* Setup radios for RX */
	for (i = 0; i < LGW_RF_CHAIN_NB; i++) {
		if (CONTEXT_RF_CHAIN[i].enable == true) {
			/* Reset the radio */
			err = sx1302_radio_reset(i, CONTEXT_RF_CHAIN[i].type);
			if (err != LGW_REG_SUCCESS) {
				printf("[%s:%d] ERROR: failed to reset radio %d\n",
						__FUNCTION__, __LINE__, i);
				return LGW_HAL_ERROR;
			}

			/* Setup the radio */
			switch (CONTEXT_RF_CHAIN[i].type) {
			case LGW_RADIO_TYPE_SX1250:
#if ENABLE_SX1250 == 1
				err = sx1250_setup(i, CONTEXT_RF_CHAIN[i].freq_hz,
						CONTEXT_RF_CHAIN[i].single_input_mode);
#else
				printf("[%s:%d] ERROR: LGW_RADIO_TYPE_SX1250 is not supported in this firmware : you should rebuild it\n", __FUNCTION__, __LINE__);
				return LGW_HAL_ERROR;
#endif
				break;
			case LGW_RADIO_TYPE_SX1255:
			case LGW_RADIO_TYPE_SX1257:
#if ENABLE_SX125X == 1
				err = sx125x_setup(i, CONTEXT_BOARD.clksrc, true, CONTEXT_RF_CHAIN[i].type, CONTEXT_RF_CHAIN[i].freq_hz);
#else
				printf(
						"[%s:%d] ERROR: LGW_RADIO_TYPE_SX1255 and LGW_RADIO_TYPE_SX1257 are not supported in this firmware : you should rebuild it\n",
						__FUNCTION__, __LINE__);
				return LGW_HAL_ERROR;
#endif
				break;
			default:
				printf(
						"[%s:%d] ERROR: RADIO TYPE NOT SUPPORTED (RF_CHAIN %d)\n",
						__FUNCTION__, __LINE__, i);
				return LGW_HAL_ERROR;
			}
			if (err != LGW_REG_SUCCESS) {
				printf("[%s:%d] ERROR: failed to setup radio %d\n",
						__FUNCTION__, __LINE__, i);
				return LGW_HAL_ERROR;
			}

			/* Set radio mode */
			err = sx1302_radio_set_mode(i, CONTEXT_RF_CHAIN[i].type);
			if (err != LGW_REG_SUCCESS) {
				printf("[%s:%d] ERROR: failed to set mode for radio %d\n",
						__FUNCTION__, __LINE__, i);
				return LGW_HAL_ERROR;
			}
		}
	}

	/* Select the radio which provides the clock to the sx1302 */
	err = sx1302_radio_clock_select(CONTEXT_BOARD.clksrc);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to get clock from radio %u\n",
				__FUNCTION__, __LINE__, CONTEXT_BOARD.clksrc);
		return LGW_HAL_ERROR;
	}

	/* Release host control on radio (will be controlled by AGC) */
	err = sx1302_radio_host_ctrl(false);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to release control over radios\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Basic initialization of the sx1302 */
	err = sx1302_init(&CONTEXT_FINE_TIMESTAMP);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to initialize SX1302\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Configure PA/LNA LUTs */
	err = sx1302_pa_lna_lut_configure(&CONTEXT_BOARD);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 PA/LNA LUT\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Configure Radio FE */
	err = sx1302_radio_fe_configure();
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 radio frontend\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Configure the Channelizer */
	err = sx1302_channelizer_configure(CONTEXT_IF_CHAIN, false);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 channelizer\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* configure LoRa 'multi-sf' modems */
	err = sx1302_lora_correlator_configure(CONTEXT_IF_CHAIN, &(CONTEXT_DEMOD));
	if (err != LGW_REG_SUCCESS) {
		printf(
				"[%s:%d] ERROR: failed to configure SX1302 LoRa modem correlators\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
	err = sx1302_lora_modem_configure(CONTEXT_RF_CHAIN[0].freq_hz);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 LoRa modems\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* configure LoRa 'single-sf' modem */
	if (CONTEXT_IF_CHAIN[8].enable == true) {
		err = sx1302_lora_service_correlator_configure(&(CONTEXT_LORA_SERVICE));
		if (err != LGW_REG_SUCCESS) {
			printf(
					"[%s:%d] ERROR: failed to configure SX1302 LoRa Service modem correlators\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		err = sx1302_lora_service_modem_configure(&(CONTEXT_LORA_SERVICE),
				CONTEXT_RF_CHAIN[0].freq_hz);
		if (err != LGW_REG_SUCCESS) {
			printf(
					"[%s:%d] ERROR: failed to configure SX1302 LoRa Service modem\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	}

#if ENABLE_MOD_FSK == 1
	/* configure FSK modem */
	if (CONTEXT_IF_CHAIN[9].enable == true) {
		err = sx1302_fsk_configure(&(CONTEXT_FSK));
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to configure SX1302 FSK modem\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	}
	printf("INFO: FSK modem is configured\n");
#endif

	/* configure syncword */
	err = sx1302_lora_syncword(CONTEXT_LWAN_PUBLIC,
			CONTEXT_LORA_SERVICE.datarate);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 LoRa syncword\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* enable demodulators - to be done before starting AGC/ARB */
	err = sx1302_modem_enable();
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to enable SX1302 modems\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Load AGC firmware */
	switch (CONTEXT_RF_CHAIN[CONTEXT_BOARD.clksrc].type) {
	case LGW_RADIO_TYPE_SX1250:
#if ENABLE_SX1250 == 1
		DEBUG_PRINTF("[%s:%d] Loading AGC fw for sx1250\n", __FUNCTION__, __LINE__);
		err = sx1302_agc_load_firmware(agc_firmware_sx1250);
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to load AGC firmware for sx1250\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		fw_version_agc = FW_VERSION_AGC_SX1250;
		break;
#else
		printf("[%s:%d] LGW_RADIO_TYPE_SX1250 is not supported in this firmware : you should rebuild it\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
#endif
		break;
	case LGW_RADIO_TYPE_SX1255:
	case LGW_RADIO_TYPE_SX1257:
#if ENABLE_SX125X == 1
		DEBUG_PRINTF("[%s:%d] Loading AGC fw for sx125x\n", __FUNCTION__, __LINE__);
		err = sx1302_agc_load_firmware(agc_firmware_sx125x);
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to load AGC firmware for sx125x\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		fw_version_agc = FW_VERSION_AGC_SX125X;
		break;
#else
		printf(
				"[%s:%d] ERROR: LGW_RADIO_TYPE_SX1255 and LGW_RADIO_TYPE_SX1257 are not supported in this firmware : you should rebuild it\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
		break;
#endif
	default:
		printf(
				"[%s:%d] ERROR: failed to load AGC firmware, radio type not supported (%d)\n",
				__FUNCTION__, __LINE__,
				CONTEXT_RF_CHAIN[CONTEXT_BOARD.clksrc].type);
		return LGW_HAL_ERROR;
	}
	err = sx1302_agc_start(fw_version_agc,
			CONTEXT_RF_CHAIN[CONTEXT_BOARD.clksrc].type,
			SX1302_AGC_RADIO_GAIN_AUTO, SX1302_AGC_RADIO_GAIN_AUTO,
			CONTEXT_BOARD.full_duplex, CONTEXT_SX1261.lbt_conf.enable);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to start AGC firmware\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Load ARB firmware */
	DEBUG_PRINTF("[%s:%d] Loading ARB fw\n", __FUNCTION__, __LINE__);
	err = sx1302_arb_load_firmware(arb_firmware);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to load ARB firmware\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}
	err = sx1302_arb_start(FW_VERSION_ARB, &CONTEXT_FINE_TIMESTAMP);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to start ARB firmware\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* static TX configuration */
	err = sx1302_tx_configure(CONTEXT_RF_CHAIN[CONTEXT_BOARD.clksrc].type);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to configure SX1302 TX path\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

#if ENABLE_GPS == 1
	/* enable GPS */
	err = sx1302_gps_enable(true);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to enable GPS on sx1302\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
	DEBUG_PRINTF("INFO: GPS is enabled\n");
#else
	/* disable GPS */
	err = sx1302_gps_enable(false);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to disable GPS on sx1302\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}
	DEBUG_PRINTF("INFO: GPS is disabled\n");
#endif
#ifdef GPS_MODEL
	DEBUG_PRINTF("INFO: GPS Model is \"%s\" (on UART #%d @ %d baud)\n", GPS_MODEL, GPS_UART_DEV, GPS_BAUDRATE);
#endif

	/* For debug logging */
#if ENABLE_DEBUG_FILE_LOG == 1
	char timestamp_str[40];
	struct tm *timenow;

	/* Append current time to log file name */
	time_t now = time(NULL);
	timenow = gmtime(&now);
	strftime(timestamp_str, sizeof(timestamp_str), ".%Y-%m-%d_%H%M%S", timenow);
	strncat(CONTEXT_DEBUG.log_file_name, timestamp_str, sizeof CONTEXT_DEBUG.log_file_name);

	/* Open the file for writting */
	log_file = fopen(CONTEXT_DEBUG.log_file_name, "w+"); /* create log file, overwrite if file already exist */
	if (log_file == NULL) {
		printf("[%s:%d] ERROR: impossible to create log file %s\n", __FUNCTION__, __LINE__, CONTEXT_DEBUG.log_file_name);
		return LGW_HAL_ERROR;
	} else {
		DEBUG_PRINTF("[%s:%d] INFO: %s file opened for debug log\n", __FUNCTION__, __LINE__, CONTEXT_DEBUG.log_file_name);

		/* Create "pktlog.csv" symlink to simplify user life */
		unlink("loragw_hal.log");
		i = symlink(CONTEXT_DEBUG.log_file_name, "loragw_hal.log");
		if (i < 0) {
			printf("[%s:%d] ERROR: impossible to create symlink to log file %s\n", __FUNCTION__, __LINE__, CONTEXT_DEBUG.log_file_name);
		}
	}
#endif

	/* Configure the pseudo-random generator (For Debug) */
	dbg_init_random();

	if (CONTEXT_COM_TYPE == LGW_COM_SPI) {
#if ENABLE_STTS751 == 1
		/* Find the temperature sensor on the known supported ports */
		for (i = 0; i < (int)(sizeof I2C_PORT_TEMP_SENSOR); i++) {
			ts_addr = I2C_PORT_TEMP_SENSOR[i];
			err = i2c_linuxdev_open(I2C_DEVICE, ts_addr, &ts_fd);
			if (err != LGW_I2C_SUCCESS) {
				printf("[%s:%d] ERROR: failed to open I2C for temperature sensor on port 0x%02X\n", __FUNCTION__, __LINE__, ts_addr);
				return LGW_HAL_ERROR;
			}

			err = stts751_configure(ts_fd, ts_addr);
			if (err != LGW_I2C_SUCCESS) {
				DEBUG_PRINTF("[%s:%d] INFO: no temperature sensor found on port 0x%02X\n", __FUNCTION__, __LINE__, ts_addr);
				i2c_linuxdev_close(ts_fd);
				ts_fd = -1;
			} else {
				DEBUG_PRINTF("[%s:%d] INFO: found temperature sensor on port 0x%02X\n", __FUNCTION__, __LINE__, ts_addr);
				break;
			}
		}
		if (i == sizeof I2C_PORT_TEMP_SENSOR) {
			printf("[%s:%d] ERROR: no temperature sensor found.\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
#else
		DEBUG_PRINTF("INFO: I2C STTS751 temperature sensor is disabled\n");
#endif
#if ENABLE_AD5338R == 1
		/* Configure ADC AD5338R for full duplex (CN490 reference design) */
		if (CONTEXT_BOARD.full_duplex == true) {
			err = i2c_linuxdev_open(I2C_DEVICE, I2C_PORT_DAC_AD5338R, &ad_fd);
			if (err != LGW_I2C_SUCCESS) {
				printf("[%s:%d] ERROR: failed to open I2C for ad5338r\n", __FUNCTION__, __LINE__);
				return LGW_HAL_ERROR;
			}

			err = ad5338r_configure(ad_fd, I2C_PORT_DAC_AD5338R);
			if (err != LGW_I2C_SUCCESS) {
				printf("[%s:%d] ERROR: failed to configure ad5338r\n", __FUNCTION__, __LINE__);
				i2c_linuxdev_close(ad_fd);
				ad_fd = -1;
				return LGW_HAL_ERROR;
			}

			/* Turn off the PA: set DAC output to 0V */
			uint8_t volt_val[AD5338R_CMD_SIZE] = { 0x39, (uint8_t)VOLTAGE2HEX_H(0), (uint8_t)VOLTAGE2HEX_L(0) };
			err = ad5338r_write(ad_fd, I2C_PORT_DAC_AD5338R, volt_val);
			if (err != LGW_I2C_SUCCESS) {
				printf("[%s:%d] ERROR: AD5338R: failed to set DAC output to 0V\n", __FUNCTION__, __LINE__);
				return LGW_HAL_ERROR;
			}
			DEBUG_PRINTF("[%s:%d] INFO: AD5338R: Set DAC output to 0x%02X 0x%02X\n", __FUNCTION__, __LINE__, (uint8_t)VOLTAGE2HEX_H(0), (uint8_t)VOLTAGE2HEX_L(0));
		}
#else
		DEBUG_PRINTF("INFO: I2C AD5338R ADC sensor is disabled\n");
#endif
	}

#if ENABLE_SX1261 == 1
	/* Connect to the external sx1261 for LBT or Spectral Scan */
	if (CONTEXT_SX1261.enable == true) {
		err = sx1261_connect(CONTEXT_COM_TYPE, (CONTEXT_COM_TYPE == LGW_COM_SPI) ? CONTEXT_SX1261.spi_path : NULL);
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to connect to the sx1261 radio (LBT/Spectral Scan)\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}

		err = sx1261_load_pram();
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to patch sx1261 radio for LBT/Spectral Scan\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}

		err = sx1261_calibrate(CONTEXT_RF_CHAIN[0].freq_hz);
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to calibrate sx1261 radio\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}

		err = sx1261_setup();
		if (err != LGW_REG_SUCCESS) {
			printf("[%s:%d] ERROR: failed to setup sx1261 radio\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	}

	/* Set CONFIG_DONE GPIO to 1 (turn on the corresponding LED) */
	err = sx1302_set_gpio(0x01);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to set CONFIG_DONE GPIO\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
#else
	DEBUG_PRINTF("INFO: SX1261 is disabled\n");
#endif

	/* set hal state */
	CONTEXT_STARTED = true;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_stop(void) {
	int i, x, err = LGW_HAL_SUCCESS;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	if (CONTEXT_STARTED == false) {
		DEBUG_PRINTF("[%s:%d] WARNING: : LoRa concentrator was not started...\n", __FUNCTION__, __LINE__);
		return LGW_HAL_SUCCESS;
	}

	/* Abort current TX if needed */
	for (i = 0; i < LGW_RF_CHAIN_NB; i++) {
		DEBUG_PRINTF("[%s:%d] INFO: aborting TX on chain %u\n", __FUNCTION__, __LINE__, i);
		x = lgw_abort_tx(i);
		if (x != LGW_HAL_SUCCESS) {
			DEBUG_PRINTF("[%s:%d] WARNING: failed to get abort TX on chain %u\n", __FUNCTION__, __LINE__, i);
			err = LGW_HAL_ERROR;
		}
	}
#ifndef RIOT_APPLICATION
	/* Close log file */
	if (log_file != NULL) {
		fclose(log_file);
		log_file = NULL;
	}
#endif

	DEBUG_PRINTF("[%s:%d] INFO: Disconnecting\n", __FUNCTION__, __LINE__);
	x = lgw_disconnect();
	if (x != LGW_HAL_SUCCESS) {
		printf("[%s:%d] ERROR: failed to disconnect concentrator\n",
				__FUNCTION__, __LINE__);
		err = LGW_HAL_ERROR;
	}

#ifndef RIOT_APPLICATION
	if (CONTEXT_COM_TYPE == LGW_COM_SPI) {
		DEBUG_PRINTF("INFO: Closing I2C for temperature sensor\n", __FUNCTION__, __LINE__);
		x = i2c_linuxdev_close(ts_fd);
		if (x != 0) {
			printf("[%s:%d] ERROR: failed to close I2C temperature sensor device (err=%i)\n", __FUNCTION__, __LINE__, x);
			err = LGW_HAL_ERROR;
		}

		if (CONTEXT_BOARD.full_duplex == true) {
			DEBUG_PRINTF("INFO: Closing I2C for AD5338R\n", __FUNCTION__, __LINE__);
			x = i2c_linuxdev_close(ad_fd);
			if (x != 0) {
				printf("[%s:%d] ERROR: failed to close I2C AD5338R device (err=%i)\n", __FUNCTION__, __LINE__, x);
				err = LGW_HAL_ERROR;
			}
		}
	}
#endif

	CONTEXT_STARTED = false;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return err;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_receive(uint8_t max_pkt, struct lgw_pkt_rx_s *pkt_data) {
	int res;
	uint8_t nb_pkt_fetched = 0;
	uint8_t nb_pkt_found = 0;
	uint8_t nb_pkt_left = 0;
	/* performances variables */
	struct timeval tm;

	(void) nb_pkt_left;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	/* Record function start time */
	_meas_time_start(&tm);

	/* Get packets from SX1302, if any */
	res = sx1302_fetch(&nb_pkt_fetched);
	if (res != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: failed to fetch packets from SX1302\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Update internal counter */
	/* WARNING: this needs to be called regularly by the upper layer */
	res = sx1302_update();
	if (res != LGW_REG_SUCCESS) {
		return LGW_HAL_ERROR;
	}

	/* Exit now if no packet fetched */
	if (nb_pkt_fetched == 0) {
		_meas_time_stop(1, tm, __FUNCTION__);
		return 0;
	}
	if (nb_pkt_fetched > max_pkt) {
		nb_pkt_left = nb_pkt_fetched - max_pkt;
		DEBUG_PRINTF("[%s:%d] WARNING: not enough space allocated, fetched %d packet(s), %d will be left in RX buffer\n", __FUNCTION__, __LINE__, nb_pkt_fetched, nb_pkt_left);
	}

	/* Apply RSSI temperature compensation */
	float current_temperature;
	res = lgw_get_temperature(&current_temperature);
	if (res != LGW_HAL_SUCCESS) {
		printf("[%s:%d] ERROR: failed to get current temperature\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* Iterate on the RX buffer to get parsed packets */
	for (nb_pkt_found = 0;
			nb_pkt_found
					< ((nb_pkt_fetched <= max_pkt) ? nb_pkt_fetched : max_pkt);
			nb_pkt_found++) {
		/* Get packet and move to next one */
		res = sx1302_parse(&lgw_context, &pkt_data[nb_pkt_found]);
		if (res == LGW_REG_WARNING) {
			DEBUG_PRINTF("[%s:%d] WARNING: parsing error on packet %d, discarding fetched packets\n", __FUNCTION__, __LINE__, nb_pkt_found);
			return LGW_HAL_SUCCESS;
		} else if (res == LGW_REG_ERROR) {
			printf(
					"[%s:%d] ERROR: fatal parsing error on packet %d, aborting...\n",
					__FUNCTION__, __LINE__, nb_pkt_found);
			return LGW_HAL_ERROR;
		}

		/* Apply RSSI offset calibrated for the board */
		pkt_data[nb_pkt_found].rssic +=
				CONTEXT_RF_CHAIN[pkt_data[nb_pkt_found].rf_chain].rssi_offset;
		pkt_data[nb_pkt_found].rssis +=
				CONTEXT_RF_CHAIN[pkt_data[nb_pkt_found].rf_chain].rssi_offset;

		const float rssi_temperature_offset = sx1302_rssi_get_temperature_offset(
				&CONTEXT_RF_CHAIN[pkt_data[nb_pkt_found].rf_chain].rssi_tcomp,
				current_temperature);
		pkt_data[nb_pkt_found].rssic += rssi_temperature_offset;
		pkt_data[nb_pkt_found].rssis += rssi_temperature_offset;
#ifdef RIOT_APPLICATION
		pkt_data[nb_pkt_found].rssi_temperature_offset = rssi_temperature_offset;
		pkt_data[nb_pkt_found].temperature = current_temperature;
#endif
		DEBUG_PRINTF("[%s:%d] INFO: RSSI temperature offset applied: %.3f dB (current temperature %.1f C)\n", __FUNCTION__, __LINE__, rssi_temperature_offset, current_temperature);
	}

	DEBUG_PRINTF("[%s:%d] INFO: nb pkt found:%u left:%u\n", __FUNCTION__, __LINE__, nb_pkt_found, nb_pkt_left);

	/* Remove duplicated packets generated by double demod when precision timestamp is enabled */
	if ((nb_pkt_found > 0) && (CONTEXT_FINE_TIMESTAMP.enable == true)) {
		res = merge_packets(pkt_data, &nb_pkt_found);
		if (res != 0) {
			DEBUG_PRINTF("[%s:%d] WARNING: failed to remove duplicated packets\n", __FUNCTION__, __LINE__);
		}

		DEBUG_PRINTF("[%s:%d] INFO: nb pkt found:%u (after de-duplicating)\n", __FUNCTION__, __LINE__, nb_pkt_found);
	}

	_meas_time_stop(1, tm, __FUNCTION__);

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return nb_pkt_found;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_send(struct lgw_pkt_tx_s *pkt_data) {
	int err;
#ifndef RIOT_APPLICATION
	bool lbt_tx_allowed;
#else
	bool lbt_tx_allowed = false;
#endif
	/* performances variables */
	struct timeval tm;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	/* Record function start time */
	_meas_time_start(&tm);

	/* check if the concentrator is running */
	if (CONTEXT_STARTED == false) {
		printf(
				"[%s:%d] ERROR: CONCENTRATOR IS NOT RUNNING, START IT BEFORE SENDING\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	CHECK_NULL(pkt_data);

	/* check input range (segfault prevention) */
	if (pkt_data->rf_chain >= LGW_RF_CHAIN_NB) {
		printf("[%s:%d] ERROR: INVALID RF_CHAIN TO SEND PACKETS\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	/* check input variables */
	if (CONTEXT_RF_CHAIN[pkt_data->rf_chain].tx_enable == false) {
		printf(
				"[%s:%d] ERROR: SELECTED RF_CHAIN IS DISABLED FOR TX ON SELECTED BOARD\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}
	if (CONTEXT_RF_CHAIN[pkt_data->rf_chain].enable == false) {
		printf("[%s:%d] ERROR: SELECTED RF_CHAIN IS DISABLED\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}
	if (!IS_TX_MODE(pkt_data->tx_mode)) {
		printf("[%s:%d] ERROR: TX_MODE NOT SUPPORTED\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}
	if (pkt_data->modulation == MOD_LORA) {
		if (!IS_LORA_BW(pkt_data->bandwidth)) {
			printf("[%s:%d] ERROR: BANDWIDTH NOT SUPPORTED BY LORA TX\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_LORA_DR(pkt_data->datarate)) {
			printf("[%s:%d] ERROR: DATARATE NOT SUPPORTED BY LORA TX\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_LORA_CR(pkt_data->coderate)) {
			printf("[%s:%d] ERROR: CODERATE NOT SUPPORTED BY LORA TX\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (pkt_data->size > 255) {
			printf("[%s:%d] ERROR: PAYLOAD LENGTH TOO BIG FOR LORA TX\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	} else if (pkt_data->modulation == MOD_FSK) {
		if ((pkt_data->f_dev < 1) || (pkt_data->f_dev > 200)) {
			printf(
					"[%s:%d] ERROR: TX FREQUENCY DEVIATION OUT OF ACCEPTABLE RANGE\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (!IS_FSK_DR(pkt_data->datarate)) {
			printf("[%s:%d] ERROR: DATARATE NOT SUPPORTED BY FSK IF CHAIN\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		if (pkt_data->size > 255) {
			printf("[%s:%d] ERROR: PAYLOAD LENGTH TOO BIG FOR FSK TX\n",
					__FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	} else if (pkt_data->modulation == MOD_CW) {
		/* do nothing */
	} else {
		printf("[%s:%d] ERROR: INVALID TX MODULATION\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

#if ENABLE_AD5338R == 1
	/* Set PA gain with AD5338R when using full duplex CN490 ref design */
	if (CONTEXT_BOARD.full_duplex == true) {
		uint8_t volt_val[AD5338R_CMD_SIZE] = {0x39, VOLTAGE2HEX_H(2.51), VOLTAGE2HEX_L(2.51)}; /* set to 2.51V */
		err = ad5338r_write(ad_fd, I2C_PORT_DAC_AD5338R, volt_val);
		if (err != LGW_I2C_SUCCESS) {
			printf("[%s:%d] ERROR: failed to set voltage by ad5338r\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
		DEBUG_PRINTF("[%s:%d] INFO: AD5338R: Set DAC output to 0x%02X 0x%02X\n", __FUNCTION__, __LINE__, (uint8_t)VOLTAGE2HEX_H(2.51), (uint8_t)VOLTAGE2HEX_L(2.51));
	}
#else
	DEBUG_PRINTF("INFO: AD5338R is disabled\n");
#endif

#if ENABLE_SX1261 == 1
	/* Start Listen-Before-Talk */
	if (CONTEXT_SX1261.lbt_conf.enable == true) {
		err = lgw_lbt_start(&CONTEXT_SX1261, pkt_data);
		if (err != 0) {
			printf("[%s:%d] ERROR: failed to start LBT\n", __FUNCTION__, __LINE__);
			return LGW_HAL_ERROR;
		}
	}
#else
	DEBUG_PRINTF("INFO: SX1261 is disabled\n");
#endif

	/* Send the TX request to the concentrator */
	err = sx1302_send(CONTEXT_RF_CHAIN[pkt_data->rf_chain].type,
			&CONTEXT_TX_GAIN_LUT[pkt_data->rf_chain], CONTEXT_LWAN_PUBLIC,
			&CONTEXT_FSK, pkt_data);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: %s: Failed to send packet\n", __FUNCTION__,
				__LINE__, __FUNCTION__);

#if ENABLE_SX1261 == 1
		if (CONTEXT_SX1261.lbt_conf.enable == true) {
			err = lgw_lbt_stop();
			if (err != 0) {
				printf("[%s:%d] ERROR: %s: Failed to stop LBT\n", __FUNCTION__, __LINE__, __FUNCTION__);
			}
		}
#else
		DEBUG_PRINTF("INFO: SX1261 is disabled\n");
#endif
		return LGW_HAL_ERROR;
	}

	_meas_time_stop(1, tm, __FUNCTION__);

#if ENABLE_SX1261 == 1
	/* Stop Listen-Before-Talk */
	if (CONTEXT_SX1261.lbt_conf.enable == true) {
		err = lgw_lbt_tx_status(pkt_data->rf_chain, &lbt_tx_allowed);
		if (err != 0) {
			printf("[%s:%d] ERROR: %s: Failed to get LBT TX status, TX aborted\n", __FUNCTION__, __LINE__, __FUNCTION__);
			err = sx1302_tx_abort(pkt_data->rf_chain);
			if (err != 0) {
				printf("[%s:%d] ERROR: %s: Failed to abort TX\n", __FUNCTION__, __LINE__, __FUNCTION__);
			}
			err = lgw_lbt_stop();
			if (err != 0) {
				printf("[%s:%d] ERROR: %s: Failed to stop LBT\n", __FUNCTION__, __LINE__, __FUNCTION__);
			}
			return LGW_HAL_ERROR;
		}
		if (lbt_tx_allowed == true) {
			printf("[%s:%d] LBT: packet is allowed to be transmitted\n", __FUNCTION__, __LINE__);
		} else {
			printf("[%s:%d] LBT: (ERROR) packet is NOT allowed to be transmitted\n", __FUNCTION__, __LINE__);
		}

		err = lgw_lbt_stop();
		if (err != 0) {
			printf("[%s:%d] ERROR: %s: Failed to stop LBT\n", __FUNCTION__, __LINE__, __FUNCTION__);
			return LGW_HAL_ERROR;
		}
	}
#endif

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	if (CONTEXT_SX1261.lbt_conf.enable == true && lbt_tx_allowed == false) {
		return LGW_LBT_NOT_ALLOWED;
	} else {
		return LGW_HAL_SUCCESS;
	}
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_status(uint8_t rf_chain, uint8_t select, uint8_t *code) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	/* check input variables */
	CHECK_NULL(code);
	if (rf_chain >= LGW_RF_CHAIN_NB) {
		printf("[%s:%d] ERROR: NOT A VALID RF_CHAIN NUMBER\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Get status */
	if (select == TX_STATUS) {
		if (CONTEXT_STARTED == false) {
			*code = TX_OFF;
		} else {
			*code = sx1302_tx_status(rf_chain);
		}
	} else if (select == RX_STATUS) {
		if (CONTEXT_STARTED == false) {
			*code = RX_OFF;
		} else {
			*code = sx1302_rx_status(rf_chain);
		}
	} else {
		printf("[%s:%d] ERROR: SELECTION INVALID, NO STATUS TO RETURN\n",
				__FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	//DEBUG_PRINTF("[%s:%d] INFO: STATUS %u\n", __FUNCTION__, __LINE__, *code);
	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_abort_tx(uint8_t rf_chain) {
	int err;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	/* check input variables */
	if (rf_chain >= LGW_RF_CHAIN_NB) {
		printf("[%s:%d] ERROR: NOT A VALID RF_CHAIN NUMBER\n", __FUNCTION__,
				__LINE__);
		return LGW_HAL_ERROR;
	}

	/* Abort current TX */
	err = sx1302_tx_abort(rf_chain);

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return err;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_get_trigcnt(uint32_t *trig_cnt_us) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	CHECK_NULL(trig_cnt_us);

	*trig_cnt_us = sx1302_timestamp_counter(true);

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_get_instcnt(uint32_t *inst_cnt_us) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	CHECK_NULL(inst_cnt_us);

	*inst_cnt_us = sx1302_timestamp_counter(false);

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_get_eui(uint64_t *eui) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	CHECK_NULL(eui);

	if (sx1302_get_eui(eui) != LGW_REG_SUCCESS) {
		return LGW_HAL_ERROR;
	}

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#ifdef RIOT_APPLICATION

#ifndef FAKE_TEMPERATURE
#define FAKE_TEMPERATURE	20.0
#endif

#if defined(MODULE_STTS751) && defined(STTS751_CORECELL_I2C_ADDR)

#include "stts751.h"
#include "stts751_params.h"

extern stts751_t* p_stts751_corecell;

int lgw_get_temperature(float *temperature) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	*temperature = stts751_get_temperature(p_stts751_corecell);

	DEBUG_PRINTF("[%s:%d] INFO: get corecell temperature t=%f\n", __FUNCTION__, __LINE__, *temperature);

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;

}
#elif defined(MODULE_SAUL) && defined(ENABLE_SAUL_TEMPSENSOR)

#include "saul_reg.h"
#include "phydat.h"
#include "senml/phydat.h"

int lgw_get_temperature(float *temperature) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	saul_reg_t *dev = saul_reg;

    if (dev == NULL) {
    	printf("[%s:%d] INFO: No SAUL devices present : get fake temperature\n", __FUNCTION__, __LINE__);
    	*temperature = FAKE_TEMPERATURE;
    } else {
    	while (dev) {
    	    phydat_t res;
			if(dev->driver->type == SAUL_SENSE_TEMP) {
				int dim = saul_reg_read(dev, &res);

//				phydat_dump(&res, dim);
			    senml_value_t val;
				phydat_to_senml_float(&val, &res, dim);
			    *temperature = val.value.value.f;
			    printf("[%s:%d] INFO: get temperature for %s sensor (%f %s)\n", __FUNCTION__, __LINE__, dev->name, *temperature, senml_unit_to_str(val.attr.unit));
				break;
			}
			dev = dev->next;
		}
    	printf("[%s:%d] INFO: No SAUL temperature devices present : get fake temperature\n", __FUNCTION__, __LINE__);
		*temperature = FAKE_TEMPERATURE;
    }

    printf("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;

}

#else

int lgw_get_temperature(float *temperature) {
	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	DEBUG_PRINTF("[%s:%d] INFO: No temperature sensor configured : get fake temperature\n", __FUNCTION__, __LINE__);
	*temperature = FAKE_TEMPERATURE;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return LGW_HAL_SUCCESS;
}

#endif

#else
int lgw_get_temperature(float *temperature) {

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	CHECK_NULL(temperature);

	int err = LGW_HAL_ERROR;

	switch (CONTEXT_COM_TYPE) {
	case LGW_COM_SPI:
		err = stts751_get_temperature(ts_fd, ts_addr, temperature);
		break;
	case LGW_COM_USB:
		err = lgw_com_get_temperature(temperature);
		break;
	default:
		printf("[%s:%d] ERROR: wrong communication type (SHOULD NOT HAPPEN)\n", __FUNCTION__, __LINE__);
		break;
	}

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return err;
}
#endif
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

const char* lgw_version_info(void) {
	return lgw_version_string;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

uint32_t lgw_time_on_air(const struct lgw_pkt_tx_s *packet) {
	double t_fsk;
	uint32_t toa_ms, toa_us;

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "IN");

	if (packet == NULL) {
		printf(
				"[%s:%d] ERROR: Failed to compute time on air, wrong parameter\n",
				__FUNCTION__, __LINE__);
		return 0;
	}

	if (packet->modulation == MOD_LORA) {
		toa_us = lora_packet_time_on_air(packet->bandwidth, packet->datarate,
				packet->coderate, packet->preamble, packet->no_header,
				packet->no_crc, packet->size, NULL, NULL, NULL);
		toa_ms = (uint32_t)((double) toa_us / 1000.0 + 0.5);
		DEBUG_PRINTF("[%s:%d] INFO: LoRa packet ToA: %lu ms\n", __FUNCTION__, __LINE__, toa_ms);
	} else if (packet->modulation == MOD_FSK) {
		/* PREAMBLE + SYNC_WORD + PKT_LEN + PKT_PAYLOAD + CRC
		 PREAMBLE: default 5 bytes
		 SYNC_WORD: default 3 bytes
		 PKT_LEN: 1 byte (variable length mode)
		 PKT_PAYLOAD: x bytes
		 CRC: 0 or 2 bytes
		 */
		t_fsk = (8
				* (double) (packet->preamble + CONTEXT_FSK.sync_word_size + 1
						+ packet->size + ((packet->no_crc == true) ? 0 : 2))
				/ (double) packet->datarate) * 1E3;

		/* Duration of packet */
		toa_ms = (uint32_t) t_fsk + 1; /* add margin for rounding */
	} else {
		toa_ms = 0;
		printf(
				"[%s:%d] ERROR: Cannot compute time on air for this packet, unsupported modulation (0x%02X)\n",
				__FUNCTION__, __LINE__, packet->modulation);
	}

	DEBUG_PRINTF("[%s:%d] --- %s\n", __FUNCTION__, __LINE__, "OUT");

	return toa_ms;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_spectral_scan_start(uint32_t freq_hz, uint16_t nb_scan) {
#if ENABLE_LBT == 1
	int err;

	if (CONTEXT_SX1261.enable != true) {
		printf("[%s:%d] ERROR: sx1261 is not enabled, no spectral scan\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	err = sx1261_set_rx_params(freq_hz, BW_125KHZ);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: Failed to set RX params for Spectral Scan\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	err = sx1261_spectral_scan_start(nb_scan);
	if (err != LGW_REG_SUCCESS) {
		printf("[%s:%d] ERROR: start spectral scan failed\n", __FUNCTION__, __LINE__);
		return LGW_HAL_ERROR;
	}

	return LGW_HAL_SUCCESS;
#else
	(void) freq_hz;
	(void) nb_scan;
	printf("[%s:%d] ERROR: spectral scan is not implemented\n", __FUNCTION__, __LINE__);
	return LGW_HAL_ERROR;
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_spectral_scan_get_status(lgw_spectral_scan_status_t *status) {
#if ENABLE_SX1261 == 1
	return sx1261_spectral_scan_status(status);
#else
	(void) status;
	printf("[%s:%d] ERROR: spectral scan is not implemented\n", __FUNCTION__, __LINE__);
	return LGW_HAL_ERROR;
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_spectral_scan_get_results(
		int16_t levels_dbm[static LGW_SPECTRAL_SCAN_RESULT_SIZE],
		uint16_t results[static LGW_SPECTRAL_SCAN_RESULT_SIZE]) {
#if ENABLE_SX1261 == 1
	return sx1261_spectral_scan_get_results(CONTEXT_SX1261.rssi_offset, levels_dbm, results);
#else
	(void) levels_dbm;
	(void) results;
	printf("[%s:%d] ERROR: spectral scan is not implemented\n", __FUNCTION__, __LINE__);
	return LGW_HAL_ERROR;
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int lgw_spectral_scan_abort(void) {
#if ENABLE_SX1261 == 1
	return sx1261_spectral_scan_abort();
#else
	printf("[%s:%d] ERROR: spectral scan is not implemented\n", __FUNCTION__, __LINE__);
	return LGW_HAL_ERROR;
#endif
}

#ifdef RIOT_APPLICATION

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/**
 * Get the time on air of a packet
 * @brief For RX (since lgw_time_on_air is for TX)
 */
uint32_t lgw_time_on_air_params(uint8_t modulation, uint8_t bandwidth, // 0x04 for 125000 KHz
		uint8_t coderate, // 1 for CR4/5
		uint8_t datarate, // SF
		uint8_t preamble, bool no_header, /* header is always enabled, except for beacons */
		bool no_crc, uint8_t size) {
	int32_t val;
	uint8_t SF, H, DE;
	uint16_t BW;
	uint32_t payloadSymbNb, Tpacket;
	double Tsym, Tpreamble, Tpayload, Tfsk;

	if (modulation == MOD_LORA) {
		/* Get bandwidth */
		val = lgw_bw_getval(bandwidth);
		if (val != -1) {
			BW = (uint16_t)(val / 1E3);
		} else {
			printf(
					"[%s:%d] ERROR: Cannot compute time on air for this packet, unsupported bandwidth (%d)\n",
					__FUNCTION__, __LINE__, bandwidth);
			return 0;
		}

		/* Get datarate */
		val = lgw_sf_getval(datarate);
		if (val != -1) {
			SF = (uint8_t) val;
			/* TODO: update formula for SF5/SF6 */
			if (SF < 7) {
				DEBUG_PRINTF("[%s:%d] WARNING: clipping time on air computing to SF7 for SF5/SF6\n", __FUNCTION__, __LINE__);
				SF = 7;
			}
		} else {
			printf(
					"[%s:%d] ERROR: Cannot compute time on air for this packet, unsupported datarate (%d)\n",
					__FUNCTION__, __LINE__, datarate);
			return 0;
		}

		/* Duration of 1 symbol */
		Tsym = pow(2, SF) / BW;

		/* Duration of preamble */
		Tpreamble = ((double) (preamble) + 4.25) * Tsym;

		/* Duration of payload */
		H = (no_header == false) ? 0 : 1; /* header is always enabled, except for beacons */
		DE = (SF >= 11) ? 1 : 0; /* Low datarate optimization enabled for SF11 and SF12 */

		payloadSymbNb =
				8
						+ (ceil(
								(double) (8 * size - 4 * SF + 28 + 16 - 20 * H)
										/ (double) (4 * (SF - 2 * DE)))
								* (coderate + 4)); /* Explicitely cast to double to keep precision of the division */

		Tpayload = payloadSymbNb * Tsym;

		/* Duration of packet */
		Tpacket = Tpreamble + Tpayload;
	} else if (modulation == MOD_FSK) {
		/* PREAMBLE + SYNC_WORD + PKT_LEN + PKT_PAYLOAD + CRC
		 PREAMBLE: default 5 bytes
		 SYNC_WORD: default 3 bytes
		 PKT_LEN: 1 byte (variable length mode)
		 PKT_PAYLOAD: x bytes
		 CRC: 0 or 2 bytes
		 */
		Tfsk = (8
				* (double) (preamble + CONTEXT_FSK.sync_word_size + 1 + size
						+ ((no_crc == true) ? 0 : 2)) / (double) datarate)
				* 1E3;

		/* Duration of packet */
		Tpacket = (uint32_t) Tfsk + 1; /* add margin for rounding */
	} else {
		Tpacket = 0;
		printf(
				"[%s:%d] ERROR: Cannot compute time on air for this packet, unsupported modulation (0x%02X)\n",
				__FUNCTION__, __LINE__, modulation);
	}

	return Tpacket;
}

/**
 * @brief Status of the LGW
 * @return true if started
 */
bool lgw_is_started(void) {
	return CONTEXT_STARTED;
}

#endif

/* --- EOF ------------------------------------------------------------------ */
