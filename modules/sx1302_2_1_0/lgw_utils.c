/*
 SX1302 LGW Utilities
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>
#include <time.h>

#ifndef ENABLE_DEBUG_LGW_UTILS
#define ENABLE_DEBUG_LGW_UTILS		1
#endif
#define ENABLE_DEBUG ENABLE_DEBUG_LGW_UTILS
#include "debug.h"

#include "rtc_utilities.h"

#define DEF2STR(x) #x
#define STR(x) DEF2STR(x)

#include "periph/rtc.h"
#include "lgw_utils.h"

#include "lorawan_printf.h"
#ifdef MODULE_LORA_MESH
#include "lora_mesh.h"
#endif

static uint32_t _last_count_us = 0;

void lgw_print_info(void) {

#if ENABLE_DEBUG == 1
	printf("SX1302 Lib %s\n", lgw_version_info());
	printf("LGW_LISTEN_STACKSIZE: " STR(LGW_LISTEN_STACKSIZE) "\n");
	printf("ENDPOINT_DEVADDR    : " STR(ENDPOINT_DEVADDR) "\n");
	printf("ENABLE_MOD_FSK      : " STR(ENABLE_MOD_FSK) "\n");
	printf("ENABLE_MOD_CW       : " STR(ENABLE_MOD_CW) "\n");
	printf("ENABLE_SX125X       : " STR(ENABLE_SX125X) "\n");
	printf("ENABLE_SX1250       : " STR(ENABLE_SX1250) "\n");
	printf("ENABLE_SX1261       : " STR(ENABLE_SX1261) "\n");
	printf("ENABLE_STTS751      : " STR(ENABLE_STTS751) "\n");
	printf("ENABLE_AD5338R      : " STR(ENABLE_AD5338R) "\n");
	printf("ENABLE_LBT          : " STR(ENABLE_LBT) "\n");
	printf("ENABLE_GPS          : " STR(ENABLE_GPS) "\n");
	printf("ENABLE_REGTEST      : " STR(ENABLE_REGTEST) "\n");
#endif
}

uint8_t lgw_bandwidthToBwLoRaCode(uint32_t bandwidthInHz) {
	switch (bandwidthInHz) {
	case 125000:
		return BW_125KHZ;
	case 250000:
		return BW_250KHZ;
	case 500000:
		return BW_500KHZ;
	default:
		return BW_UNDEFINED;
	}
}

uint32_t lgw_bandwidthToBwLoRaHz(uint8_t bwCode) {
	switch (bwCode) {
	case BW_125KHZ:
		return 125000;
	case BW_250KHZ:
		return 250000;
	case BW_500KHZ:
		return 500000;
	default:
		return 0;
	}
}

const char* lgw_get_status_str(uint8_t status) {
	switch (status) {
	case STAT_CRC_OK:
		return "CRC_OK";
	case STAT_CRC_BAD:
		return "BAD_CRC";
	case STAT_NO_CRC:
		return "NO_CRC";
	case STAT_UNDEFINED:
	default:
		return "UNDEFINED";
	}
}

const char* lgw_get_modulation_str(uint8_t modulation) {
	switch (modulation) {
	case MOD_LORA:
		return "LoRa";
	case MOD_FSK:
		return "FSK";
	case MOD_CW:
		return "CW";
	default:
		return "Unknown modulation";
	}
}

const char* lgw_get_tx_mode_str(const uint8_t tx_mode) {
	switch (tx_mode) {
	case IMMEDIATE:
		return "IMMEDIATE";
		break;
	case TIMESTAMPED:
		return "TIMESTAMPED";
		break;
	case ON_GPS:
		return "ON_GPS";
		break;
	default:
		return "Unknown TX mode";
		break;
	}
}



// Imported from sys/shell/commands/sc_rtc.c
static int _print_time(struct tm *time) {
	printf("%04i-%02i-%02i %02i:%02i:%02i", time->tm_year + 1900,
			time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min,
			time->tm_sec);
	return 0;
}

// Imported from sys/shell/commands/sc_rtc.c
int lgw_print_rtc(void) {
	struct tm t;
	if (rtc_get_time(&t) == 0) {
		_print_time(&t);
		return 0;
	} else {
		puts("rtc: error getting time");
		return 1;
	}
}

uint32_t lgw_get_delta_instcnt(const uint32_t start, const uint32_t end) {
	uint32_t delta_count_us;
	if (start == 0 && end == 0) {
		delta_count_us = 0;
	} else if (end > start) {
		delta_count_us = end - start;
	} else {
		delta_count_us = end + (UINT32_MAX - start);
	}
	return delta_count_us;
}


void lgw_printf_rxpkt(const struct lgw_pkt_rx_s *rxpkt) {
#if ENABLE_DEBUG == 1

	// TODO print the delta in us between rxpkt->count_us;

	uint32_t count_us = rxpkt->count_us;
	const uint32_t delta_count_us = lgw_get_delta_instcnt(count_us,
			_last_count_us);
	_last_count_us = count_us;

	const uint32_t time_on_air = lgw_time_on_air_params(rxpkt->modulation,
			rxpkt->bandwidth, // 0x04 for 125000 KHz
			rxpkt->coderate, // 1 for CR4/5
			rxpkt->datarate, // SF
			rxpkt->modulation == MOD_LORA ? 8 : 5, // default in LoRaWAN
			true, /* header is always enabled, except for beacons */
			(rxpkt->status != STAT_NO_CRC), rxpkt->size);

	printf("\n----- %s packet - TimeOnAir: %lu msec (",
			lgw_get_modulation_str(rxpkt->modulation), time_on_air);
	lgw_print_rtc();
	printf(") -----\n");

	printf("  freq_hz    : %lu\n", rxpkt->freq_hz);
	printf("  freq_offset: %ld\n", rxpkt->freq_offset);
	printf("  count_us   : %lu (delta: %lu)\n", rxpkt->count_us,
			delta_count_us);
	printf("  chan       : %u\n", rxpkt->if_chain);
	printf("  rf_chain   : %u\n", rxpkt->rf_chain);
	printf("  modem_id   : %u\n", rxpkt->modem_id);
	printf("  status     : 0x%02XX %s\n", rxpkt->status,
			lgw_get_status_str(rxpkt->status));
	if (rxpkt->modulation == MOD_LORA) {
		printf("  datr       : %lu\n", rxpkt->datarate);
		printf("  bw         : %lu (0x%02X)\n",
				lgw_bandwidthToBwLoRaHz(rxpkt->bandwidth), rxpkt->bandwidth);
		printf("  codr       : %u\n", rxpkt->coderate);
	}

	if (rxpkt->modulation == MOD_LORA) {
		printf("  snr_avg    : %.1f\n", rxpkt->snr); /*!> average packet SNR, in dB (LoRa only) */
		printf("  snr_min    : %.1f\n", rxpkt->snr_min); /*!> minimum packet SNR, in dB (LoRa only) */
		printf("  snr_max    : %.1f\n", rxpkt->snr_max); /*!> maximum packet SNR, in dB (LoRa only) */
	}

	printf("  rssi_chan  : %.1f\n", rxpkt->rssic); /*!> average RSSI of the channel in dB */
	printf("  rssi_sig   : %.1f\n", rxpkt->rssis); /*!> average RSSI of the signal in dB */
#ifdef RIOT_APPLICATION
	printf("  rssi_off   : %.1f\n", rxpkt->rssi_temperature_offset);
	printf("  temp       : %.1f\n", rxpkt->temperature);
#endif

	/*!> a fine timestamp has been received */
	if (rxpkt->ftime_received) {
		/*!> packet fine timestamp (nanoseconds since last PPS) */
		printf("  ftime      : %ld\n", rxpkt->ftime);
	} else {
		printf("  ftime      : na\n");
	}
	printf("  crc        : 0x%04X\n", rxpkt->crc);
	printf("  size       : %u\n", rxpkt->size);
	for (int j = 0; j < rxpkt->size; j++) {
		printf("%02x ", rxpkt->payload[j]);
	}
	printf("\n");
#ifdef MODULE_LORA_MESH
	if(lora_mesh_check_valid_frame(rxpkt->payload,rxpkt->size)) {
		lora_mesh_printf_frame(rxpkt->payload,rxpkt->size);
	} else {
		lorawan_printf_payload(rxpkt->payload,rxpkt->size);
	}
#else
	lorawan_printf_payload(rxpkt->payload, rxpkt->size);
#endif
#else
	(void)rxpkt;
#endif
}
