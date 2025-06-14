/*
 SX1302 LGW commands
 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#define ENABLE_DEBUG 1
#include "debug.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "assert.h"

#include "fmt.h"

#include "periph/rtc.h"
#include "xtimer.h"
#include "thread.h"

#include "loragw_hal.h"
#include "loragw_sx1302.h"

#include "lgw_config.h"
#include "lgw_cmd.h"
#include "lgw_endpoint.h"

#include "lgw_utils.h"

#include "lorawan_mac.h"
#include "lorawan_printf.h"
#ifdef MODULE_LORA_MESH
#include "lora_mesh.h"
#endif

#include "loragw_sx1302.h"

#ifdef ENDPOINT_DEVADDR

static const lgw_sx130x_endpoint_t lgw_sx130x_default_endpoint = {
		.module = UNKNOWN_SX130X_MODULE,
		.devaddr = ENDPOINT_DEVADDR,
		.gweui = ENDPOINT_GWEUI,
		.nwkskey = ENDPOINT_NWKSKEY,
		.appskey = ENDPOINT_APPSKEY,
};

lgw_sx130x_endpoint_t* lgw_sx130x_endpoint = &lgw_sx130x_default_endpoint;

#endif

#if INVOKE_CALLBACKS == 1
#ifndef PERIODIC_CALLBACK_IN_SEC
#define PERIODIC_CALLBACK_IN_SEC		(60U)
#endif
void (*pkt_rx_cb)(const struct lgw_pkt_rx_s*, struct lgw_pkt_tx_s*) = NULL;
void (*pkt_period_cb)(struct lgw_pkt_tx_s*) = NULL;
#endif

// Frequency plan : computed from concentrator channel
static uint32_t _freq_hz_plan[LGW_IF_CHAIN_NB - 2];

// Thingsat Frequencies Plan for TX
// static uint32_t freq_hz_plan[] = { 867500000, 869525000 };

static struct lgw_tx_gain_lut_s _txlut; /* TX gain table */

static bool _quit_rx = false;

struct lgw_stat_t {
	uint32_t rx;
	uint32_t rx_ok;
	uint32_t rx_bad;
	uint32_t rx_nocrc;
	uint32_t tx;
	uint32_t tx_abort;
};

static struct lgw_stat_t _lgw_stat = { 0, 0, 0, 0, 0, 0 };

// Imported from sys/shell/commands/sc_rtc.c
static int _print_time(struct tm *time) {
	printf("%04i-%02i-%02i %02i:%02i:%02i", time->tm_year + 1900,
			time->tm_mon + 1, time->tm_mday, time->tm_hour, time->tm_min,
			time->tm_sec);
	return 0;
}

// Imported from sys/shell/commands/sc_rtc.c
static int _print_rtc(void) {
	struct tm t;
	if (rtc_get_time(&t) == 0) {
		_print_time(&t);
		return 0;
	} else {
		puts("rtc: error getting time");
		return 1;
	}
}

static void _lgw_stat_reset(void) {
	_lgw_stat.rx = 0;
	_lgw_stat.rx_ok = 0;
	_lgw_stat.rx_bad = 0;
	_lgw_stat.rx_nocrc = 0;
	_lgw_stat.tx = 0;
	_lgw_stat.tx_abort = 0;
}

static void _lgw_stat_printf(void) {
	printf("\n----- stat (");
	_print_rtc();
	printf(") -----\n");
	printf("  rx       : %lu\n", _lgw_stat.rx);
	printf("  rx_ok    : %lu\n", _lgw_stat.rx_ok);
	printf("  rx_bad   : %lu\n", _lgw_stat.rx_bad);
	printf("  rx_nocrc : %lu\n", _lgw_stat.rx_nocrc);
	printf("  tx       : %lu\n", _lgw_stat.tx);
	printf("  tx_abort : %lu\n", _lgw_stat.tx_abort);
	printf("\n");
}


uint32_t lgw_frequency_plan_size(void) {
 return LGW_IF_CHAIN_NB - 2;
}

uint32_t lgw_get_freq_hz(const uint8_t channel) {
	if(channel < lgw_frequency_plan_size()) {
		return _freq_hz_plan[channel];
	} else {
		return 0;
	}
}

/**
 *
 * @brief Get the frequencies plan
 */
static int lgw_frequencies_plan_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;
	if (lgw_is_started()) {
		for (int i = 0; i < LGW_IF_CHAIN_NB - 2; i++) {
			if (i != 0) {
				printf(", ");
			}
			printf("%ld", _freq_hz_plan[i]);
		}
		printf("\n");
	} else {
		puts("lgw is not_started");
	}

	return EXIT_SUCCESS;
}

/**
 * Status command
 * @brief Get the gateway stats
 */
static int lgw_status_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;
	if (lgw_is_started()) {
		puts("lgw is started");
	} else {
		puts("lgw is not_started");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Stat command
 * @brief Get the gateway stats
 */
static int lgw_stat_cmd(int argc, char **argv) {
	if (argc == 1) {
		_lgw_stat_printf();
	} else if (argc == 2 && strcmp(argv[1], "reset") == 0) {
		_lgw_stat_reset();
	} else {
		puts("usage: lgw_stat [reset]");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
 * Reset command
 * @brief Power on and reset the gateway
 */
static int lgw_reset_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;
	return sx1302_reset();
}

static int _configure_board(void) {

	/* configure board */
	struct lgw_conf_board_s boardconf;
	memset(&boardconf, 0, sizeof boardconf);
	boardconf.lorawan_public = true;
	boardconf.clksrc = 0; //1302 is clocked from radio 0
	boardconf.full_duplex = false;
	boardconf.com_type = LGW_COM_SPI;
	strncpy(boardconf.com_path, "SPI_0\0", sizeof(boardconf.com_path));
	boardconf.com_path[sizeof(boardconf.com_path - 1)] = '\0';
	if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
		printf("ERROR: failed to configure board\r\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

#if 0
static int _configure_txlut(struct lgw_tx_gain_lut_s *txlut, const int8_t rf_power) {

	const uint32_t pwr_idx = lgw_get_pwr_idx(rf_power);
	const uint32_t power_amplifier = lgw_get_pa_gain(rf_power);

	txlut->size = 0;
	memset(txlut->lut, 0, sizeof txlut->lut);
	txlut->size = 1;
	txlut->lut[0].pwr_idx = pwr_idx;
	txlut->lut[0].pa_gain = power_amplifier;
	txlut->lut[0].mix_gain = 5; /* TODO: rework this, should not be needed for sx1250 */
	txlut->lut[0].rf_power = rf_power;
	if (lgw_txgain_setconf(0, txlut) != LGW_HAL_SUCCESS) {
		printf("ERROR: failed to configure txgain lut\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


.rf_power = 14,
.dig_gain = 0,
.pa_gain = 2,
.dac_gain = 3,
.mix_gain = 10,
.offset_i = 0,
.offset_q = 0,
.pwr_idx = 0

#endif

static int _configure_txlut(struct lgw_tx_gain_lut_s *txlut,
		const int8_t rf_power) {

	const uint32_t pwr_idx = lgw_get_pwr_idx(rf_power);
	const uint32_t power_amplifier = lgw_get_pa_gain(rf_power);

	txlut->size = 0;
	memset(txlut->lut, 0, sizeof txlut->lut);
	txlut->size = 1;

	txlut->lut[0].rf_power = rf_power;
	txlut->lut[0].pwr_idx = pwr_idx;

	// PA gain [0..3]
	txlut->lut[0].pa_gain = power_amplifier;

	// sx1302 digital gain [0..3]
	txlut->lut[0].dig_gain = LGW_SX1302_LUT_DIG_GAIN;
	// sx1257 DAC gain [0..3]
	txlut->lut[0].dac_gain = LGW_SX1257_LUT_DAC_GAIN; /* TODO: rework this, should not be needed for sx1250 */
	// sx1257 MIX gain [0..15]
	txlut->lut[0].mix_gain = LGW_SX1257_LUT_MIX_GAIN;

	// TX RFChain = 0
	if (lgw_txgain_setconf(0, txlut) != LGW_HAL_SUCCESS) {
		printf("ERROR: failed to configure txgain lut for rfchain=%d\n", 0);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


/**
 * Configure Radio for RX according to global_conf.json.sx1250.EU868 parameters
 */
static int _configure_radio(bool enable_tx_on_rfchain_0) {

#if ENABLE_SX1250 == 1
//	From global_conf.json.sx1250.EU868
	const lgw_radio_type_t radio_type = LGW_RADIO_TYPE_SX1250;
#endif
#if ENABLE_SX125X == 1
	#error "Can not configure radio for RX according to global_conf.json.sx1257.EU868 parameters"
#endif

	lgw_config_t *lgw_config = lgw_get_lgw_config();

	/* set configuration for RF chains */
	struct lgw_conf_rxrf_s rfconf;

	// RF chain 0
	memset(&rfconf, 0, sizeof rfconf);
	rfconf.enable = true;
	rfconf.freq_hz = lgw_config->fa;
	rfconf.type = radio_type;
	rfconf.tx_enable = enable_tx_on_rfchain_0; // Enable TX on RF chain 0
	rfconf.single_input_mode = lgw_config->single_input_mode;
	rfconf.rssi_offset = lgw_config->rssi_offset;
	rfconf.rssi_tcomp.coeff_a = lgw_config->rssi_tcomp_coeff_a;
	rfconf.rssi_tcomp.coeff_b = lgw_config->rssi_tcomp_coeff_b;
	rfconf.rssi_tcomp.coeff_c = lgw_config->rssi_tcomp_coeff_c;
	rfconf.rssi_tcomp.coeff_d = lgw_config->rssi_tcomp_coeff_d;
	rfconf.rssi_tcomp.coeff_e = lgw_config->rssi_tcomp_coeff_e;

	if (lgw_rxrf_setconf(0, &rfconf) != LGW_HAL_SUCCESS) {
		printf("failed to configure rxrf 0\n");
		return EXIT_FAILURE;
	}

	// RF chain 1
	memset(&rfconf, 0, sizeof rfconf);
	rfconf.enable = true;
	rfconf.freq_hz = lgw_config->fb;
	rfconf.type = radio_type;
	rfconf.tx_enable = false;
	rfconf.single_input_mode = lgw_config->single_input_mode;
	rfconf.rssi_offset = lgw_config->rssi_offset;
	rfconf.rssi_tcomp.coeff_a = lgw_config->rssi_tcomp_coeff_a;
	rfconf.rssi_tcomp.coeff_b = lgw_config->rssi_tcomp_coeff_b;
	rfconf.rssi_tcomp.coeff_c = lgw_config->rssi_tcomp_coeff_c;
	rfconf.rssi_tcomp.coeff_d = lgw_config->rssi_tcomp_coeff_d;
	rfconf.rssi_tcomp.coeff_e = lgw_config->rssi_tcomp_coeff_e;

	if (lgw_rxrf_setconf(1, &rfconf) != LGW_HAL_SUCCESS) {
		printf("failed to configure rxrf 1\n");
		return EXIT_FAILURE;
	}

	/* set configuration for LoRa multi-SF channels (bandwidth cannot be set) */

	struct lgw_conf_rxif_s ifconf;

	memset(&ifconf, 0, sizeof(ifconf));

	printf("RX frequency plan :\r\n");
	int i;
	for (i = 0; i < LGW_IF_CHAIN_NB - 2; i++) {
		ifconf.enable = true;
		ifconf.rf_chain = lgw_config->channel_rfchain[i];
		ifconf.freq_hz = lgw_config->channel_if[i];
		ifconf.datarate = DR_LORA_SF7;
		if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
			printf("failed to configure rxif %d\n", i);
			return EXIT_FAILURE;
		}
		printf("CH%d \tRadio %s \t%.3fMHz \r\n", i, ifconf.rf_chain ? "B" : "A",
				ifconf.rf_chain ?
						(double) (lgw_config->fb + ifconf.freq_hz) / 1000000.0 :
						(double) (lgw_config->fa + ifconf.freq_hz) / 1000000.0);

	}

	// Frequency plan for bench cmd
	for (i = 0; i < LGW_IF_CHAIN_NB - 2; i++) {
		_freq_hz_plan[i] =
				lgw_config->channel_rfchain[i] ?
						lgw_config->fb + lgw_config->channel_if[i] :
						lgw_config->fa + lgw_config->channel_if[i];
	}

	/* set configuration for LoRa Service channel */
	i = LGW_IF_CHAIN_NB - 2;
	if (!lgw_config->lorastd_enable) {
		printf("INFO: Lora standard channel %i disabled\n", i);
	} else {
		memset(&ifconf, 0, sizeof ifconf); /* initialize configuration structure */
		ifconf.enable = true;
		ifconf.rf_chain = lgw_config->channel_rfchain[i];
		ifconf.freq_hz = lgw_config->channel_if[i];
		ifconf.datarate = lgw_config->lorastd_datarate;
		ifconf.bandwidth = lgw_bandwidthToBwLoRaCode(lgw_config->lorastd_bw);

		// TODO lorastd_implicit_hdr == true
		if (lgw_config->lorastd_implicit_hdr) {
			printf(
					"ERROR: Lora standard channel with implicit header is not implemented\n");
			return EXIT_FAILURE;
		}

		if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
			printf("ERROR: failed to configure LoRa Standard channel\n");
			return EXIT_FAILURE;
		}
		printf("CH%d \tRadio %s \t%.3f MHz \t%lu Hz (LoRa Standard band)\r\n",
				i, ifconf.rf_chain ? "B" : "A",
				ifconf.rf_chain ?
						(double) (lgw_config->fb + ifconf.freq_hz) / 1000000.0 :
						(double) (lgw_config->fa + ifconf.freq_hz) / 1000000.0,
				lgw_config->lorastd_bw);
	}

	/* set configuration for FSK channel */
	i = LGW_IF_CHAIN_NB - 1;
	if (!lgw_config->fsk_enable) {
		printf("INFO: FSK channel disabled\n");
	} else {
		uint32_t bw, fdev;

		// TODO  add  FSK channel config into "chan_FSK":       {"enable": true, "radio": 1, "if":  300000, "bandwidth": 125000, "datarate": 50000}
		memset(&ifconf, 0, sizeof ifconf); /* initialize configuration structure */
		ifconf.enable = true;

		ifconf.rf_chain = lgw_config->channel_rfchain[i]; // "chan_FSK.radio"
		ifconf.freq_hz = lgw_config->channel_if[i]; // "chan_FSK.if"
		ifconf.datarate = lgw_config->fsk_datarate; //  "chan_FSK.datarate"

		bw = lgw_config->fsk_bw; // "chan_FSK.bandwidth"
		fdev = lgw_config->fsk_fdev; // "chan_FSK.freq_deviation"

		/* if chan_FSK.bandwidth is set, it has priority over chan_FSK.freq_deviation */
		if ((bw == 0) && (fdev != 0)) {
			bw = 2 * fdev + ifconf.datarate;
		}
		if (bw == 0)
			ifconf.bandwidth = BW_UNDEFINED;
#if 0
		else if (bw <= 7800)   ifconf.bandwidth = BW_7K8HZ;
		else if (bw <= 15600)  ifconf.bandwidth = BW_15K6HZ;
		else if (bw <= 31200)  ifconf.bandwidth = BW_31K2HZ;
		else if (bw <= 62500)  ifconf.bandwidth = BW_62K5HZ;
#endif
		else if (bw <= 125000)
			ifconf.bandwidth = BW_125KHZ;
		else if (bw <= 250000)
			ifconf.bandwidth = BW_250KHZ;
		else if (bw <= 500000)
			ifconf.bandwidth = BW_500KHZ;
		else
			ifconf.bandwidth = BW_UNDEFINED;

		if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
			printf("ERROR: invalid configuration for FSK channel\n");
			return EXIT_FAILURE;
		}

		printf(
				"CH%d \tRadio %s \t%.3f MHz \t%lu Hz \%lu bps datarate (FSK channel)\r\n",
				i, ifconf.rf_chain ? "B" : "A",
				ifconf.rf_chain ?
						(double) (lgw_config->fb + ifconf.freq_hz) / 1000000.0 :
						(double) (lgw_config->fa + ifconf.freq_hz) / 1000000.0,
				bw, ifconf.datarate);
	}
	return EXIT_SUCCESS;
}

/**
 * Start command
 * @brief Start the gateway
 */
static int lgw_start_cmd(int argc, char **argv) {

	// TODO add param for enable/disable TX
	(void) argv;

	if (argc != 1) {
		puts("usage: lgw_start");
		return -1;
	}

	if (lgw_is_started()) {
		printf("ERROR: the gateway is already started\n");
		return EXIT_FAILURE;
	}

	printf("SX1302 Lib %s\n", lgw_version_info());

	if (sx1302_reset() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	if (_configure_board() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	// Configure radio for RX and TX
	if (_configure_radio(true) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	/* connect, configure and start the LoRa concentrator */
	int x = lgw_start();
	if (x != 0) {
		printf("ERROR: failed to start the gateway\n");

		lgw_com_close(); //close SPI and turn NSS pin low (to avoid current leaking)
		sx1302_poweroff(); //cut the alimentation if the start procedure didn't work

		return EXIT_FAILURE;
	}

	printf("INFO: Gateway is started\n");

	return EXIT_SUCCESS;
}

/**
 * Stop command
 * @brief Stop the gateway
 */
static int lgw_stop_cmd(int argc, char **argv) {

	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		lgw_com_close();
		return EXIT_SUCCESS;
	}

	_quit_rx = true;

	int x = lgw_stop();
	if (x != 0) {
		printf("ERROR: failed to stop the gateway\n");
		return EXIT_FAILURE;
	}

	if (sx1302_poweroff() == EXIT_FAILURE) {
		printf("ERROR: failed to power off the gateway\n");
		return EXIT_FAILURE;
	}

	printf("INFO: Gateway is stopped\n");

	return EXIT_SUCCESS;
}

/**
 * EUI command
 * @brief Get the EUI of the concentrator
 */
static int lgw_eui_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	uint64_t eui;
	int err = lgw_get_eui(&eui);
	if (err != 0) {
		printf("ERROR: failed to get EUI\n");
		return EXIT_FAILURE;
	}

	char eui_hex[20] = { 0 };
	fmt_u64_hex(eui_hex, eui);
	printf("Concentrator EUI: 0x%s\n", eui_hex);

	return EXIT_SUCCESS;
}

/**
 * RF params command
 * @brief Get the RF params of the concentrator
 */
static int lgw_rfparams_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	printf("SX1302_LUT_DIG_GAIN: %d\n", LGW_SX1302_LUT_DIG_GAIN);
#if ENABLE_SX125X == 1
	printf("SX1257_LUT_DAC_GAIN: %d\n", LGW_SX1257_LUT_DAC_GAIN);
	printf("SX1257_LUT_MIX_GAIN: %d\n", LGW_SX1257_LUT_MIX_GAIN);
#endif
	printf("Power      :");
	for (int rf_power = 12; rf_power < 27; rf_power++) {
		printf(" %2u", rf_power);
	}
	printf("\n");
	printf("Power index:");
	for (int rf_power = 12; rf_power < 27; rf_power++) {
		printf(" %2u", lgw_get_pwr_idx(rf_power));
	}
	printf("\n");
	printf("Power gain :");
	for (int rf_power = 12; rf_power < 27; rf_power++) {
		printf(" %2u", lgw_get_pa_gain(rf_power));
	}
	printf("\n");

	return EXIT_SUCCESS;
}

// For lgw_bench
static uint32_t _fCntUp = 0;

/**
 * Bench command
 */
static int lgw_bench_cmd(int argc, char **argv) {

	if (argc != 10) {
		puts(
				"usage: lgw_bench <nbpkt> <sf> <bw> <preamble> <rfpower> <crc> <invert_iq> <size> <pause>");
		puts("nbpkt: Number of packets to transmit before exiting.");
		puts("sf: Spreading factor (7 .. 12).");
		puts("bw: Bandwidth in kHz (125, 250, 500).");
		puts("preamble: Preamble (8 .. 12).");
		puts("rfpower: RF power in dBm (12 .. 27).");
		puts("crc: CRC (on|off: off for a gateway instead of like a device).");
		puts(
				"invert_iq: Invert I/Q (true: tx as a gateway instead of like a device).");
		puts("size: Random data size.");
		puts("pause: Pause in millisec before next transmit.");
		return -1;
	}

	const int nb_packets_to_transmit = strtoul(argv[1], NULL, 10);
	int nb_remaining_packets_to_transmit = nb_packets_to_transmit;
	const uint32_t spreading_factor = strtoul(argv[2], NULL, 10);
	uint32_t bandwidth = strtoul(argv[3], NULL, 10) * 1000U;
	const uint16_t preamble = strtoul(argv[4], NULL, 10);
	const uint32_t rfpower = strtoul(argv[5], NULL, 10);
	const bool crc_on = strcmp(argv[6], "on") == 0;
	const bool invert_pol = strcmp(argv[7], "true") == 0;
	const uint32_t size = strtoul(argv[8], NULL, 10);
	const uint32_t pause = strtoul(argv[9], NULL, 10);

	// TODO check value ranges

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_SUCCESS;
	}

	/* TX gain table */
	if (_configure_txlut(&_txlut, rfpower) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	struct lgw_pkt_tx_s pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.tx_mode = IMMEDIATE;
	pkt.rf_chain = 0; //only rf_chain 0 is able to tx
	pkt.rf_power = 0; //use the single entry of the txlut TODO Should be check

	pkt.modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt.coderate = CR_LORA_4_5; // LoRaWAN
	pkt.no_crc = (crc_on != true); // LoRaWAN : on for uplink and off for downlink
	pkt.no_header = false; 		// Beacons have not header
	pkt.invert_pol = invert_pol;
	pkt.freq_hz = _freq_hz_plan[0];
	pkt.datarate = spreading_factor;
	pkt.preamble = preamble;	//  8 for LoRaWAN
	pkt.size = size;

	switch (bandwidth) {
	case 125000:
		pkt.bandwidth = BW_125KHZ;
		break;
	case 250000:
		pkt.bandwidth = BW_250KHZ;
		break;
	case 500000:
		pkt.bandwidth = BW_500KHZ;
		break;
	default:
		pkt.bandwidth = BW_125KHZ;
		bandwidth = 125000;
		break;
	}

	const uint32_t time_on_air = lgw_time_on_air(&pkt);

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;

	// APRS (https://www.aprs.org/doc/APRS101.PDF) like text message.
	const uint8_t fpayload[] =
			"@sx1302@world :This is a simple test$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62";
	const uint8_t fpayload_size = sizeof(fpayload);
	const uint8_t fPort = 100; // For Thingsat decoder (Plaintext)
	// Note: fpayload can contains an APRS string

	while (nb_remaining_packets_to_transmit--) {

		uint8_t frame_size;
		lorawan_prepare_up_dataframe(
		false, devaddr,
				0x00, // FCTrl (FOptLen = 0)
				_fCntUp,
				fPort, // fPort
				fpayload, fpayload_size, lgw_sx130x_endpoint->nwkskey,
				lgw_sx130x_endpoint->appskey, pkt.payload, &frame_size);
		pkt.size = frame_size;

		pkt.freq_hz = _freq_hz_plan[nb_packets_to_transmit
				% ARRAY_SIZE(_freq_hz_plan)];

		printf(
				"INFO: Transmitting LoRa packet (%d/%d) on %.3fMHz [SF%ldBW%d, TXPOWER %lu, PWID %u, PA %s, ToA %lu msec] devaddr: 0x%8lx fCntUp: %ld\n",
				nb_packets_to_transmit - nb_remaining_packets_to_transmit,
				nb_packets_to_transmit, (double) (pkt.freq_hz / 1E6),
				pkt.datarate, (pkt.bandwidth - 3) * 125, rfpower,
				_txlut.lut[0].pwr_idx, _txlut.lut[0].pa_gain ? "ON" : "OFF",
				time_on_air, devaddr, _fCntUp);

		_fCntUp++;

		for (unsigned int j = 0; j < size; j++) {
			printf("%02X ", pkt.payload[j]);
		}
		printf("\n");

		int x = lgw_send(&pkt);
		if (x != 0) {
			printf("ERROR: failed to send packet\n");
			_lgw_stat.tx_abort++;
			return EXIT_FAILURE;
		}

		_lgw_stat.tx++;

		// TODO seconds_between_transmit should be greated than the Time On Air
		printf("Waiting %lu msec\n", time_on_air + pause);
		xtimer_msleep(time_on_air + pause);
	}

	return EXIT_SUCCESS;
}

/**
 * RF Powers for Chan Test
 * REM: 17 : PA disable, 18 : PA enable
 */
static const uint32_t _rfpower_to_test[] = { 12, 14, 16, 17, 18, 19, 21, 23, 25,
		27 };

/**
 * ChanTest command
 */
static int lgw_chantest_cmd(int argc, char **argv) {

	if (argc != 9) {
		puts(
				"usage: lgw_chantest <nbpkt> <sf> <bw> <preamble> <crc> <invert_iq> <size> <pause>");
		puts("nbpkt: Number of packets to transmit before exiting.");
		puts("sf: Spreading factor (7 .. 12).");
		puts("bw: Bandwidth in kHz (125, 250, 500).");
		puts("preamble: Preamble (8 .. 12).");
		puts("crc: CRC (on|off: off for a gateway instead of like a device).");
		puts(
				"invert_iq: Invert I/Q (true: tx as a gateway instead of like a device).");
		puts("size: Random data size.");
		puts("pause: Pause in millisec before next transmit.");
		return -1;
	}

	const int nb_packets_to_transmit = strtoul(argv[1], NULL, 10);
	int nb_remaining_packets_to_transmit = nb_packets_to_transmit;
	const uint32_t spreading_factor = strtoul(argv[2], NULL, 10);
	uint32_t bandwidth = strtoul(argv[3], NULL, 10) * 1000U;
	const uint16_t preamble = strtoul(argv[4], NULL, 10);
	const bool crc_on = strcmp(argv[5], "on") == 0;
	const bool invert_pol = strcmp(argv[6], "true") == 0;
	const uint32_t size = strtoul(argv[7], NULL, 10);
	const uint32_t pause = strtoul(argv[8], NULL, 10);

	const uint32_t _rfpower_to_test_len = ARRAY_SIZE(_rfpower_to_test);
	const uint32_t _freq_hz_plan_len = ARRAY_SIZE(_freq_hz_plan);

	uint32_t rfpower_idx = 0;
	uint32_t rfpower = _rfpower_to_test[rfpower_idx];

	uint32_t freq_hz_plan_idx = 0;

	// TODO check value ranges

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_SUCCESS;
	}

	struct lgw_pkt_tx_s pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.tx_mode = IMMEDIATE;
	pkt.rf_chain = 0; //only rf_chain 0 is able to tx
	pkt.rf_power = 0; //use the single entry of the txlut TODO Should be check

	pkt.modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt.coderate = CR_LORA_4_5; // LoRaWAN
	pkt.no_crc = (crc_on != true); // LoRaWAN : on for uplink and off for downlink
	pkt.no_header = false; 		// Beacons have not header
	pkt.invert_pol = invert_pol;
	pkt.freq_hz = _freq_hz_plan[0];
	pkt.datarate = spreading_factor;
	pkt.preamble = preamble;	//  8 for LoRaWAN
	pkt.size = size;

	switch (bandwidth) {
	case 125000:
		pkt.bandwidth = BW_125KHZ;
		break;
	case 250000:
		pkt.bandwidth = BW_250KHZ;
		break;
	case 500000:
		pkt.bandwidth = BW_500KHZ;
		break;
	default:
		pkt.bandwidth = BW_125KHZ;
		bandwidth = 125000;
		break;
	}

	const uint32_t time_on_air = lgw_time_on_air(&pkt);

	const uint32_t devaddr = lgw_sx130x_endpoint->devaddr;

	// Set the payload
	pkt.payload[0] = 0x40; /* Confirmed Data Up */
	pkt.payload[1] = (devaddr & 0xff);
	pkt.payload[2] = ((devaddr >> 8) & 0xff);
	pkt.payload[3] = ((devaddr >> 16) & 0xff);
	pkt.payload[4] = ((devaddr >> 24) & 0xff);
	pkt.payload[5] = 0x00; /* FCTrl (FOptLen = 0) */
	pkt.payload[6] = 0; /* FCnt */
	pkt.payload[7] = 0; /* FCnt */
	pkt.payload[8] = 0x02; /* FPort */
	for (int i = 9; i < 255; i++) {
		pkt.payload[i] = i;
	}

	while (nb_remaining_packets_to_transmit--) {

		// Increment FCnt (32 bit counter)
		pkt.payload[6] = (uint8_t) _fCntUp;
		pkt.payload[7] = (uint8_t) ((_fCntUp >> 8));

		freq_hz_plan_idx = (nb_packets_to_transmit
				- nb_remaining_packets_to_transmit) % _freq_hz_plan_len;
		pkt.freq_hz = _freq_hz_plan[freq_hz_plan_idx];

		if ((nb_packets_to_transmit - nb_remaining_packets_to_transmit)
				% _freq_hz_plan_len == 0) {
			rfpower_idx = (rfpower_idx + 1) % _rfpower_to_test_len;
			rfpower = _rfpower_to_test[rfpower_idx];
		}

		/* TX gain table */
		if (_configure_txlut(&_txlut, rfpower) == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}

		printf(
				"INFO: Transmitting LoRa packet (%d/%d) on %.3fMHz [SF%ldBW%d, TXPOWER %lu, PWID %u, PA %s, ToA %lu msec] devaddr: 0x%8lx fCntUp: %ld\n",
				nb_packets_to_transmit - nb_remaining_packets_to_transmit,
				nb_packets_to_transmit, (double) (pkt.freq_hz / 1E6),
				pkt.datarate, (pkt.bandwidth - 3) * 125, rfpower,
				_txlut.lut[0].pwr_idx, _txlut.lut[0].pa_gain ? "ON" : "OFF",
				time_on_air, devaddr, _fCntUp);

		_fCntUp++;

		for (unsigned int j = 0; j < size; j++) {
			printf("%02X ", pkt.payload[j]);
		}
		printf("\n");

		int x = lgw_send(&pkt);
		if (x != 0) {
			printf("ERROR: failed to send packet\n");
			_lgw_stat.tx_abort++;
			return EXIT_FAILURE;
		}

		_lgw_stat.tx++;

		// TODO seconds_between_transmit should be greated than the Time On Air
		printf("Waiting %lu msec\n", time_on_air + pause);
		xtimer_msleep(time_on_air + pause);
	}

	return EXIT_SUCCESS;
}

static const char* _get_status_str(uint8_t status) {
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

static uint8_t last_phypayload[256];
static uint16_t last_phypayload_size = 0;
// TODO last_phypayload_timestamp

/**
 * Transmit
 */
static int _lgw_tx_pkt_tx(const struct lgw_pkt_tx_s *pkt) {

	const uint32_t time_on_air = lgw_time_on_air(pkt);

	/* TX gain table */
	if (_configure_txlut(&_txlut, pkt->rf_power) == EXIT_FAILURE) {
		printf("ERROR: failed to configure txlut with rf_power =%udBm\n",
				pkt->rf_power);
		_lgw_stat.tx_abort++;
		return EXIT_FAILURE;
	}

	// TODO add dac_gain, mix_gain, offset_i, offset_q for SX1257

	// save last packet before sending
	memcpy(last_phypayload, pkt->payload, pkt->size);
	last_phypayload_size = pkt->size;

	uint32_t inst_cnt_us_start;
	uint32_t inst_cnt_us_end;
	uint32_t inst_cnt_us_delta;
	lgw_get_instcnt(&inst_cnt_us_start);

	// Send before print for avoiding miss the tx_count
	// TODO check instruction
	int x = lgw_send((struct lgw_pkt_tx_s*) pkt); // cast since loragw_sx1302 fixes some fields.

	printf(
			"INFO: Transmitting LoRa packet on %.3fMHz [%s, SF%ldBW%d, TXPOWER %u, PWID %u, PA %s, DIG %d, %d bytes, ToA %lu msec]\n",
			(double) (pkt->freq_hz / 1E6),
			pkt->tx_mode == IMMEDIATE ? "IMMEDIATE" : pkt->tx_mode == ON_GPS ? "ON_GPS" : "TIMESTAMPED",
			pkt->datarate,
			(pkt->bandwidth - 3) * 125, pkt->rf_power, _txlut.lut[0].pwr_idx,
			_txlut.lut[0].pa_gain ? "ON" : "OFF", _txlut.lut[0].dig_gain,
			pkt->size, time_on_air);
	for (unsigned int j = 0; j < pkt->size; j++) {
		printf("%02X ", pkt->payload[j]);
	}
	printf("\n");

	if (x != 0) {
		lgw_get_instcnt(&inst_cnt_us_end);
		inst_cnt_us_delta = lgw_get_delta_instcnt(inst_cnt_us_start,
				inst_cnt_us_end);
		printf("ERROR: failed to send packet - ");
		printf("count_us: start:%lu end:%lu delta:%lu\n", inst_cnt_us_start,
				inst_cnt_us_end, inst_cnt_us_delta);
		_lgw_stat.tx_abort++;

		return EXIT_FAILURE;
	}
	lgw_get_instcnt(&inst_cnt_us_end);
	inst_cnt_us_delta = lgw_get_delta_instcnt(inst_cnt_us_start, inst_cnt_us_end);
	printf("INFO: count_us: start:%lu end:%lu delta:%lu\n", inst_cnt_us_start,
			inst_cnt_us_end, inst_cnt_us_delta);

	_lgw_stat.tx++;

	// TODO seconds_between_transmit should be greated than the Time On Air
	printf("Waiting %lu msec\n", time_on_air);
	xtimer_msleep(time_on_air);

	return EXIT_SUCCESS;
}

/**
 * Transmit a LoRa message
 */
static int _lgw_tx(const uint32_t freq_hz, const uint32_t spreading_factor, const uint32_t bandwidth,
		const uint16_t preamble, const uint32_t rf_power, const bool crc_on,
		const bool invert_pol, const uint32_t pause_ms,
		const uint16_t phypayload_size, const uint8_t *phypayload) {

	if (phypayload_size == 0) {
		printf("ERROR: the payload size must be not zero\n");
		return EXIT_FAILURE;
	}

	struct lgw_pkt_tx_s pkt;

	memset(&pkt, 0, sizeof(pkt));

	pkt.tx_mode = IMMEDIATE;
	pkt.rf_chain = 0; //only rf_chain 0 is able to tx
	pkt.rf_power = rf_power; //use the single entry of the txlut TODO Should be check

	pkt.modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt.coderate = CR_LORA_4_5; // LoRaWAN
	pkt.no_header = false; 		// Beacons have not header
	pkt.no_crc = (crc_on != true); // LoRaWAN : on for uplink and off for downlink
	pkt.invert_pol = invert_pol;

	if(freq_hz == 0) {
		pkt.freq_hz = _freq_hz_plan[_lgw_stat.tx % ARRAY_SIZE(_freq_hz_plan)];
	} else {
		pkt.freq_hz = freq_hz;
	}

	pkt.datarate = spreading_factor;
	pkt.preamble = preamble;	//  8 for LoRaWAN

	memcpy(pkt.payload, phypayload, phypayload_size);
	pkt.size = phypayload_size;

	switch (bandwidth) {
	case 125000:
		pkt.bandwidth = BW_125KHZ;
		break;
	case 250000:
		pkt.bandwidth = BW_250KHZ;
		break;
	case 500000:
		pkt.bandwidth = BW_500KHZ;
		break;
	default:
		pkt.bandwidth = BW_125KHZ;
		break;
	}

	_lgw_tx_pkt_tx(&pkt);


	printf("Waiting %lu msec\n", pause_ms);
	xtimer_msleep(pause_ms);

	return EXIT_SUCCESS;
}

/**
 * instcnt command
 */
static int lgw_get_instcnt_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}
	uint32_t inst_cnt_us;
	lgw_get_instcnt(&inst_cnt_us);
	printf("count_us: %lu\n", inst_cnt_us);

	return EXIT_SUCCESS;
}

/**
 * TX command
 */
static int lgw_tx_cmd(int argc, char **argv) {

	if (argc != 9) {
		puts(
				"usage: lgw_tx <freq> <sf> <bw> <preamble> <rfpower> <crc> <invert_iq> <payload>");
		puts("freq: Frequency in kHz.");
		puts("sf: Spreading factor (7 .. 12).");
		puts("bw: Bandwidth in kHz (125, 250, 500).");
		puts("preamble: Preamble (8 .. 12).");
		puts("rfpower: RF power in dBm (12 .. 27).");
		puts("crc: CRC (on|off: off for a gateway instead of like a device).");
		puts(
				"invert_iq: Invert I/Q (true: tx as a gateway instead of like a device).");
		puts("payload: hexadecimal payload (length < 256).");
		return EXIT_FAILURE;
	}

	const uint32_t frequency = strtoul(argv[1], NULL, 10) * 1000U;
	const uint32_t spreading_factor = strtoul(argv[2], NULL, 10);
	uint32_t bandwidth = strtoul(argv[3], NULL, 10) * 1000U;
	const uint16_t preamble = strtoul(argv[4], NULL, 10);
	const uint32_t rfpower = strtoul(argv[5], NULL, 10);
	const bool crc_on = strcmp(argv[6], "on") == 0;
	const bool invert_pol = strcmp(argv[7], "true") == 0;
	const char *hexpayload = argv[8];

	// TODO check value ranges

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_SUCCESS;
	}

	if (strlen(hexpayload) > 255 * 2) {
		printf("ERROR: the payload length must be less than 255 bytes-long\n");
		return EXIT_FAILURE;
	}

	uint8_t phypayload[256];
	uint16_t phypayload_size = fmt_hex_bytes(phypayload, hexpayload);

	return _lgw_tx(frequency, spreading_factor, bandwidth, preamble, rfpower,
			crc_on, invert_pol, 0, phypayload_size, phypayload);
}

//#define MAX_RX_PKT 4
//static struct lgw_pkt_rx_s rxpkt[MAX_RX_PKT];

#if INVOKE_CALLBACKS == 1
static lgw_pkt_tx_s_t lgw_pkt_tx_to_send;
#endif
static uint8_t previous_size = 0;
static uint8_t previous_payload[256];

/**
 * RX command
 */
static int lgw_rx_cmd(int argc, char **argv) {

	// TODO add parameter for MAX_RX_PKT

	if (argc != 3) {
		puts("usage: lgw_rx <nbpkt> <max_rx_pkt>");
		puts("nbpkt: Number of RX before exiting.");
		puts("max_rx_pkt: Max mumber of pkt into the fetching buffer.");
		return EXIT_FAILURE;
	}

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	unsigned int rx_count = strtoul(argv[1], NULL, 10);
	unsigned int MAX_RX_PKT = strtoul(argv[2], NULL, 10);
	struct lgw_pkt_rx_s rxpkt[MAX_RX_PKT];

	printf("rxpkt buffer size is set to %u\n", MAX_RX_PKT);

	unsigned long nb_pkt_crc_ok = 0;	//, nb_loop = 0; //, cnt_loop = 0;
	int nb_pkt;

	/* Loop until we have enough packets with CRC OK */
	printf("Waiting for packets...\n");
	nb_pkt_crc_ok = 0;
	_quit_rx = false;

#if INVOKE_CALLBACKS == 1
	ztimer_now_t last_periodic_call = ztimer_now(ZTIMER_SEC);
#endif
	while (!_quit_rx) {
		/* fetch N packets */
		nb_pkt = lgw_receive(MAX_RX_PKT, rxpkt);

		if (nb_pkt != 0) {
			for (int i = 0; i < nb_pkt; i++) {
				_lgw_stat.rx++;


				lgw_printf_rxpkt(rxpkt + i);

				bool same_as_previous = false;
				if (rxpkt[i].size > 0) {
					if (rxpkt[i].size == previous_size
							&& memcmp((void*) previous_payload,
									(void*) rxpkt[i].payload,
									rxpkt[i].size & 0xFF) == 0) {
						same_as_previous = true;
					} else {
						previous_size = (uint8_t) rxpkt[i].size;
						memcpy((void* )previous_payload,
								(void* )rxpkt[i].payload, previous_size);
					}
				}

				// check if rxpkt[i].payload is a packet sent by the concentrator itself
				bool is_last_tx_packet = (rxpkt[i].size == last_phypayload_size
						&& memcmp(rxpkt[i].payload, last_phypayload,
								last_phypayload_size) == 0);

				// TODO check if frame_with_my_devaddr; --> occur at the first TX which is repeated
				// TODO check if frame_with_my_relayid (Chirpstack Mesh)

				if (same_as_previous) {
					printf("INFO: Same payload than the previous RX\n");
				}

				if (is_last_tx_packet) {
					printf("INFO: Same payload than the last TX\n");
				}

				// update local statistics
				if (!same_as_previous && !is_last_tx_packet) {

					switch (rxpkt[i].status) {
					case STAT_CRC_OK:
						_lgw_stat.rx_ok++;

						nb_pkt_crc_ok += 1;
						if (nb_pkt_crc_ok >= rx_count) {
							_quit_rx = true;
							break;
						}
						break;
					case STAT_CRC_BAD:
						_lgw_stat.rx_bad++;
						break;
					case STAT_NO_CRC:
						_lgw_stat.rx_nocrc++;
						break;
					default:
						break;
					}
				}

#if INVOKE_CALLBACKS == 1
				if(!same_as_previous && !is_last_tx_packet) {

					if(pkt_rx_cb != NULL
							&& !same_as_previous && !is_last_tx_packet) {
						(*pkt_rx_cb)(rxpkt + i, &lgw_pkt_tx_to_send);
						if(lgw_pkt_tx_to_send.size > 0) {
							int res = _lgw_tx_pkt_tx(&lgw_pkt_tx_to_send);
							if(res == EXIT_FAILURE) {
								printf("ERROR: failed to send packet\n");
								_lgw_stat.tx_abort++;
							}
						}
					}
				}
#endif
			}
			printf("Received %d packets (total:%lu)\n", nb_pkt, nb_pkt_crc_ok);
		} else {
			// printf("No packet");
		}

#if INVOKE_CALLBACKS == 1
		if(pkt_period_cb != NULL) {
			// call periodically the period callback
			ztimer_now_t now = ztimer_now(ZTIMER_SEC);
			if(now > last_periodic_call + PERIODIC_CALLBACK_IN_SEC){
				last_periodic_call = now;

				// clean lgw_pkt_tx_s
				memset(&lgw_pkt_tx_to_send, 0, sizeof(lgw_pkt_tx_to_send));

				(*pkt_period_cb)(&lgw_pkt_tx_to_send);

				if(lgw_pkt_tx_to_send.size > 0) {
					int res = _lgw_tx_pkt_tx(&lgw_pkt_tx_to_send);
					if(res == EXIT_FAILURE) {
						printf("ERROR: failed to send packet\n");
						_lgw_stat.tx_abort++;
					}
					printf("INFO: packet sent\n");

				} else {
					printf("INFO: no packet to send\n");
				}
			}
		}
#endif
	}

	printf("Exiting\n");

	// TODO print local stat
	printf("Nb valid packets received: %lu CRC OK\n", nb_pkt_crc_ok);

#if INVOKE_CALLBACKS == 1
	(*pkt_period_cb)(&lgw_pkt_tx_to_send);
#endif
	return EXIT_SUCCESS;
}

void* _listen_thread(void *arg) {
	(void) arg;
	char *argv[] = { "rx", "100000000", "4" };
	lgw_rx_cmd(3, argv);
	return NULL;
}

#ifndef LGW_LISTEN_STACKSIZE
#define LGW_LISTEN_STACKSIZE      (THREAD_STACKSIZE_DEFAULT)
#endif

#if ENABLE_LGW_LISTEN_MALLOC_STACK == 1
static char* _listen_thread_stack = NULL;
#else
static char _listen_thread_stack[LGW_LISTEN_STACKSIZE];
#endif
static kernel_pid_t _listen_thread_pid;

/**
 * Listen command
 */
static int lgw_listen_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}
#if ENABLE_LGW_LISTEN_MALLOC_STACK == 1
	// alloc the stack if not already allocated
	if(_listen_thread_stack == NULL) {
		_listen_thread_stack = malloc(LGW_LISTEN_STACKSIZE);
		if(_listen_thread_stack==NULL){
			printf("ERROR: can malloc the stacka of LGW_LISTEN thread\n");
			return EXIT_FAILURE;
		}
	}
#endif
	_listen_thread_pid = thread_create(_listen_thread_stack,
			sizeof(_listen_thread_stack), THREAD_PRIORITY_MAIN - 1,
			THREAD_CREATE_STACKTEST, _listen_thread, NULL, "LGW_LISTEN");

	if (_listen_thread_pid <= KERNEL_PID_UNDEF) {
		printf("ERROR: Creation of rx thread failed\n");
#if ENABLE_LGW_LISTEN_MALLOC_STACK == 1
		free(_listen_thread_stack);
		_listen_thread_stack=NULL;
#endif
		return EXIT_FAILURE;
	}

	printf("INFO: Gateway is listening ...\n");

	return EXIT_SUCCESS;
}

#if 0
// Tentative to implement the listening with ztimer

#include "ztimer.h"
#include "ztimer/periodic.h"

#ifndef LISTENING_PERIOD
#define LISTENING_PERIOD		100LU	// msec
#endif

static unsigned cpt = 0;

static const char* _param = "LISTEN";

static int _listen_cb(void *arg)
{
	(void)arg;
	cpt++;
    DEBUG("\n[%s] LISTEN %s %d\n", __FUNCTION__, (char*)arg, cpt);
    lgw_rx_cmd(3, (char*[]){"rx", "1", "1"});

    // TODO quit
    return false;
}

static ztimer_periodic_t _listen_ztimer;

/**
 * Listen command (with ztimer)
 */
static int lgw_listen_cmd_with_ztimer(int argc, char **argv) {
    (void)argc;
    (void)argv;

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

    ztimer_periodic_init(ZTIMER_MSEC, &_listen_ztimer, _listen_cb, (void*)_param, LISTENING_PERIOD);
    ztimer_periodic_start(&_listen_ztimer);

    if (!ztimer_is_set(ZTIMER_MSEC, &_listen_ztimer.timer)) {
    	printf("[%s] ERROR\n", __FUNCTION__);
        return -1;
    }

    printf("INFO: Gateway is listening ...\n");

    return EXIT_SUCCESS;
}
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

/**
 * Repeat callback
 */
static void _lgw_repeat_cb(const struct lgw_pkt_rx_s *pkt_rx,
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

#if MESHTASTIC_ENABLE == 1
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

	printf("INFO: Repeat the received frame\n");

	pkt_tx->tx_mode = IMMEDIATE;
	pkt_tx->rf_chain = 0; //only rf_chain 0 is able to tx

	pkt_tx->freq_hz = pkt_rx->freq_hz;
	pkt_tx->datarate = pkt_rx->datarate;
	pkt_tx->bandwidth = pkt_rx->bandwidth;

#if MESHTASTIC_ENABLE == 1
		if(pkt_rx->freq_hz == 869525000) {
			pkt_tx->rf_power = 27;
		} else {
			pkt_tx->rf_power = 14;
		}
		pkt_tx->modulation = MOD_LORA;	// ONLY LoRa (No FSK)
		pkt_tx->preamble = 16;
		pkt_tx->coderate = CR_LORA_4_8;
#else
	pkt_tx->rf_power = 14; //use the single entry of the txlut TODO Should be check
	pkt_tx->modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt_tx->preamble = 8;	//  8 for LoRaWAN
	pkt_tx->coderate = CR_LORA_4_5; // 4/5 for LoRaWAN
#endif

	pkt_tx->no_header = false; 		// Beacons have not header
	pkt_tx->no_crc = false; 	// LoRaWAN : on for uplink and off for downlink
	pkt_tx->invert_pol = false;

	memcpy(pkt_tx->payload, pkt_rx->payload, pkt_rx->size);
	pkt_tx->size = pkt_rx->size;
}

inline static uint32_t h2d(const char c) {
	return (c & 0xF) + 9 * (c >> 6);
}

static uint32_t hex2dec(const char *s) {
	uint32_t v = 0;
	const unsigned int l = strlen(s);
	for (unsigned int i = 0; i < l; i++) {
		v = v << 4;
		v += h2d(s[i]);
	}
	return v;
}

/**
 * Filter command
 */
static int lgw_filter_cmd(int argc, char **argv) {
	// argv does not contain command name
	if (argc == 1) {
		// show current filter
		printf("Filter devaddr in subnet: %8lx mask: %8lx\n", _devaddr_subnet,
				_devaddr_mask);

	} else if (argc == 3) {
		// set current filter

		_devaddr_subnet = hex2dec(argv[1]);
		_devaddr_mask = hex2dec(argv[2]);

	} else {
		puts("usage: lgw filter");
		puts("usage: lgw filter <subnet> <mask>");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * SNR threshold command
 */
static int lgw_snr_threshold_cmd(int argc, char **argv) {
	if (argc == 1) {
		// show current threshold
		printf("SNR threshold : %d\n", _snr_threshold);

	} else if (argc == 2) {
		// set current threshold
		_snr_threshold = atoi(argv[1]);

	} else {
		puts("usage: lgw snr_threshold");
		puts("usage: lgw snr_threshold <threshold>");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Repeat command
 */
static int lgw_repeat_cmd(int argc, char **argv) {

	// argv does not contain command name
	if (argc != 2) {
		puts("usage: lgw repeat on");
		puts("usage: lgw repeat off");
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "on") == 0) {
		pkt_rx_cb = _lgw_repeat_cb;
	} else if (strcmp(argv[1], "off") == 0) {
		pkt_rx_cb = NULL;
	} else {
		puts("ERROR: unknown parameter");
		return EXIT_FAILURE;
	};

	return EXIT_SUCCESS;
}

/**
 * Idle command
 */
static int lgw_idle_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	_quit_rx = true;
	return EXIT_SUCCESS;
}

/**
 * Temp command
 */
static int lgw_temp_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if (!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	float temperature;
	if (!lgw_get_temperature(&temperature)) {
		printf("Corecell temperature: %f C\n", temperature);
		return EXIT_SUCCESS;
	} else {
		printf("ERROR: Can not get corecell temperature\n");
		return EXIT_FAILURE;
	}
}

#if ENABLE_LBT == 1
/**
 * LBT command
 */
static int lgw_lbt_cmd(int argc, char **argv) {
	(void)argc;
	(void)argv;

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	printf("ERROR: LBT is not currently implemented\n");
	return EXIT_FAILURE;
}

/**
 * spectral_scan command
 */
static int lgw_spectral_scan_cmd(int argc, char **argv) {
	(void)argc;
	(void)argv;

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	printf("ERROR: spectral_scan is not currently implemented\n");
	return EXIT_FAILURE;
}

#endif

#if ENABLE_REGTEST == 1

#include <math.h>

#include "loragw_reg.h"
#include "loragw_spi.h"
#include "loragw_sx1302.h"

#define START_CHRONO	uint32_t _benchmark_time = xtimer_now_usec()
#if BOARD==esp32-wroom-32
#define PRINT_CHRONO    printf("Time: %u us\n", (uint32_t)(xtimer_now_usec() - _benchmark_time))
#else
#define PRINT_CHRONO    printf("Time: %lu us\n", (uint32_t)(xtimer_now_usec() - _benchmark_time))
#endif

bool reg_ignored[LGW_TOTALREGS] = { 0 };
uint8_t rand_values[LGW_TOTALREGS] = { 0 };

static int lgw_reg_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	/* reset concentrator */
	if (sx1302_reset() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	if (_configure_board() == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	/* start concentrator */
	int err = lgw_start_for_regtest();
	if (err != 0) {
		printf("ERROR: failed to start concentrator\n");
		return EXIT_FAILURE;
	}

	//sx1302_spi_acquire_set(false);
	//sx1302_spi_acquire(&sx1302);

	START_CHRONO;

	static bool ran = false;

	/* all test fails if we set this one to 1 */
	reg_ignored[SX1302_REG_COMMON_CTRL0_CLK32_RIF_CTRL] = true;
	bool error_found = false;

	if (ran) {
		printf("TEST#1 can only be done once after reboot\n\n");
	} else {
		/* Test 1: read all registers and check default value for
		 * non-read-only registers */
		printf("## TEST#1: read all registers and check default value "
				"for non-read-only registers\n");
		for (int i = 0; i < LGW_TOTALREGS; i++) {
			if (loregs[i].rdon == 0) {
				int32_t val;
				int res = lgw_reg_r(i, &val);
				if (res == LGW_REG_ERROR)
					printf("ERROR : lgw_reg_r");
				if (val != loregs[i].dflt) {
					printf("ERROR : default value for "
							"register at index %d (0x%x, %d, %d, %d, %d) is "
#if BOARD==esp32-wroom-32
                    	"%d, should be %d\n",
#else
							"%ld, should be %ld\n",
#endif
							i, loregs[i].addr, loregs[i].offs, loregs[i].sign,
							loregs[i].leng, loregs[i].chck, val,
							loregs[i].dflt);
					error_found = true;
				} else {
					printf("SUCCESS: default value for "
							"register at index %d (0x%x, %d, %d, %d, %d) is "
#if BOARD==esp32-wroom-32
                        	"%d, should be %d\n",
#else
							"%ld, should be %ld\n",
#endif
							i, loregs[i].addr, loregs[i].offs, loregs[i].sign,
							loregs[i].leng, loregs[i].chck, val,
							loregs[i].dflt);

				}
			}
		}
		printf("------------------\n");
		printf(" TEST#1 %s\n", (error_found == false) ? "PASSED" : "FAILED");
		printf("------------------\n\n");

		ran = true;
	}

	/* Test 2: read/write test on all non-read-only, non-pulse, non-w0clr,
	 * non-w1clr registers */
	printf("## TEST#2: read/write test on all non-read-only, non-pulse, "
			"non-w0clr, non-w1clr registers\n");
	/* Write all registers with a random value */
	error_found = false;
	int32_t reg_val;
	for (int i = 0; i < LGW_TOTALREGS; i++) {
		if ((loregs[i].rdon == 0) && (reg_ignored[i] == false)) {
			/* Peek a random value different form the default reg
			 * value */
			int reg_max = pow(2, loregs[i].leng) - 1;
			if (loregs[i].leng == 1) {
				reg_val = !loregs[i].dflt;
			} else {
				/* ensure random value is not the default one */
				do {
					if (loregs[i].sign == 1) {
						reg_val = rand() % (reg_max / 2);
					} else {
						reg_val = rand() % reg_max;
					}
				} while (reg_val == loregs[i].dflt);
			}
			/* Write selected value */
			int res = lgw_reg_w(i, reg_val);
			if (res == LGW_REG_ERROR)
				printf("ERROR : lgw_reg_w");
			/* store value for later check */
			rand_values[i] = reg_val;
		}
	}
	/* Read all registers and check if we got proper random value back */
	for (int i = 0; i < LGW_TOTALREGS; i++) {
		if ((loregs[i].rdon == 0) && (loregs[i].chck == 1)
				&& (reg_ignored[i] == false)) {

			int32_t val;
			int res = lgw_reg_r(i, &val);
			if (res == LGW_REG_ERROR)
				printf("ERROR : lgw_reg_r");

			/* check value */
			if (val != rand_values[i]) {
				printf("ERROR: value read from register at "
						"index %d differs from the written "
#if BOARD==esp32-wroom-32
                        "value (w:%u r:%d)\n",
#else
						"value (w:%u r:%ld)\n",
#endif
						i, rand_values[i], val);
				error_found = true;
			} else {
				printf("INFO: MATCH reg %d (%u, %u)\n", i, rand_values[i],
						(uint8_t) val);
			}
		}
	}
	printf("------------------\n");
	printf(" TEST#2 %s\n", (error_found == false) ? "PASSED" : "FAILED");
	printf("------------------\n\n");

	//sx1302_spi_release(&sx1302);
	//sx1302_spi_acquire_set(true);

	PRINT_CHRONO;

	return EXIT_SUCCESS;
}
#endif

static void _lgw_cmd_usage(char **argv) {
	(void) argv;
	printf("%s reset         : Reset the SX1302/SX1303\n", argv[0]);
	printf("%s status        : Get the SX1302/SX1303 status\n", argv[0]);
	printf("%s freq_plan     : Get the frequencies plan\n", argv[0]);
	printf("%s start         : Start the gateway\n", argv[0]);
	printf("%s stop          : Stop the gateway\n", argv[0]);
	printf("%s stat          : Get stats of the gateway\n", argv[0]);
	printf("%s eui           : Get the concentrator EUI\n", argv[0]);
	printf("%s instcnt       : Get the instruction counter\n", argv[0]);
	printf("%s rx            : Receive radio packet(s)\n", argv[0]);
	printf(
			"%s listen        : Receive radio packet(s) in background Stop to receive radio packet(s) in background \n",
			argv[0]);
	printf("%s repeat        : Enable or disable the packet repeating\n",
			argv[0]);
	printf(
			"%s snr_threshold : Set the SNR threshold for filtering the packet repeating\n",
			argv[0]);
	printf(
			"%s filter        : Set the devaddr subnet for filtering the packet repeating\n",
			argv[0]);
	printf(
			"%s aprs          : Enable or disable the period APRS frame transmit\n",
			argv[0]);
	printf(
			"%s idle          : Stop to receive radio packet(s) in background (remark: the sx1302 continue to buffer received messages)\n",
			argv[0]);
	printf("%s tx            : Transmit one radio packet\n", argv[0]);
	printf("%s bench         : Transmit a sequence of radio packets\n",
			argv[0]);
	printf("%s rfparams      : Print the RF params of concentrator\n", argv[0]);
	printf(
			"%s chantest      : Transmit a sequence of radio packets at various tx power\n",
			argv[0]);
	printf("%s temp          : Get on-board temperature\n", argv[0]);
#if ENABLE_LBT == 1
    printf("%s lbt           : Start LBT\n", argv[0]);
#endif
//    printf("%s spectral_scan : Start spectral scan\n", argv[0]);
//    printf("%s beacon   : Transmit a beacon packet\n", argv[0]);
#if ENABLE_REGTEST == 1
	printf("%s reg_test   : Test writing and reading from registers\n",
			argv[0]);
#endif

}

int lgw_cmd(int argc, char **argv) {

	if (argc < 2) {
		_lgw_cmd_usage(argv);
		return EXIT_FAILURE;
	} else if (strcmp(argv[1], "reset") == 0) {
		return lgw_reset_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "status") == 0) {
		return lgw_status_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "freq_plan") == 0) {
		return lgw_frequencies_plan_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "start") == 0) {
		return lgw_start_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "stop") == 0) {
		return lgw_stop_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "stat") == 0) {
		return lgw_stat_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "eui") == 0) {
		return lgw_eui_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "instcnt") == 0) {
		return lgw_get_instcnt_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "rx") == 0) {
		return lgw_rx_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "listen") == 0) {
		return lgw_listen_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "repeat") == 0) {
		return lgw_repeat_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "filter") == 0) {
		return lgw_filter_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "snr_threshold") == 0) {
		return lgw_snr_threshold_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "idle") == 0) {
		return lgw_idle_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "tx") == 0) {
		return lgw_tx_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "bench") == 0) {
		return lgw_bench_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "rfparams") == 0) {
		return lgw_rfparams_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "chantest") == 0) {
		return lgw_chantest_cmd(argc - 1, argv + 1);
	} else if (strcmp(argv[1], "temp") == 0) {
		return lgw_temp_cmd(argc - 1, argv + 1);
#if ENABLE_LBT == 1
	} else if (strcmp(argv[1], "lbt") == 0) {
		return lgw_lbt_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "spectral_scan") == 0) {
		return lgw_spectral_scan_cmd(argc-1, argv+1);
#endif
#if ENABLE_REGTEST == 1
	} else if (strcmp(argv[1], "reg_test") == 0) {
		return lgw_reg_cmd(argc - 1, argv + 1);
#endif
	} else {
		_lgw_cmd_usage(argv);
		return EXIT_FAILURE;
	}
}

