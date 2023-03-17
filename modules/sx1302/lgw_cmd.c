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

#include "lgw_config.h"
#include "lgw_cmd.h"
#include "lorawan_mac.h"

#include "loragw_sx1302.h"

// Frequency plan should be computed from concentrator channel
static uint32_t _freq_hz_plan[] = { 868100000, 868300000, 868500000, 867100000,
		867300000, 867500000, 867700000, 867900000 };

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
};

static struct lgw_stat_t _lgw_stat = {0,0,0,0,0};


// Imported from sys/shell/commands/sc_rtc.c
static int _print_time(struct tm *time)
{
    printf("%04i-%02i-%02i %02i:%02i:%02i",
            time->tm_year + 1900, time->tm_mon + 1, time->tm_mday,
            time->tm_hour, time->tm_min, time->tm_sec
          );
    return 0;
}

// Imported from sys/shell/commands/sc_rtc.c
static int _print_rtc(void)
{
    struct tm t;
    if (rtc_get_time(&t) == 0) {
        _print_time(&t);
        return 0;
    }
    else {
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
	printf("\n");
}


/**
 * Status command
 * @brief Get the gateway stats
 */
static int lgw_status_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;
	if(lgw_is_started()) {
		puts("lgw is started");
	} else {
		puts("lgw is not_started");
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
	} else if (argc == 2 && strcmp(argv[1],"reset")==0) {
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
	strncpy(boardconf.spidev_path, "SPI_0\0", sizeof(boardconf.spidev_path));
	boardconf.spidev_path[sizeof(boardconf.spidev_path - 1)] = '\0';
	if (lgw_board_setconf(&boardconf) != LGW_HAL_SUCCESS) {
		printf("ERROR: failed to configure board\r\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static int _configure_txlut(struct lgw_tx_gain_lut_s *txlut, uint32_t pwr_idx, uint32_t power_amplifier) {
	txlut->size = 0;
	memset(txlut->lut, 0, sizeof txlut->lut);
	txlut->size = 1;
	txlut->lut[0].pwr_idx = pwr_idx;
	txlut->lut[0].pa_gain = power_amplifier;
	txlut->lut[0].mix_gain = 5; /* TODO: rework this, should not be needed for sx1250 */
	txlut->lut[0].rf_power = 0;
	if (lgw_txgain_setconf(0, txlut) != LGW_HAL_SUCCESS) {
		printf("ERROR: failed to configure txgain lut\n");
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


	lgw_config_t* lgw_config =  lgw_get_lgw_config();

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
	for (i = 0; i < 8; i++) {
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

	/* set configuration for LoRa Service channel */
	memset(&ifconf, 0, sizeof(ifconf));
	ifconf.enable = true;
	ifconf.rf_chain = lgw_config->channel_rfchain[i];
	ifconf.freq_hz = lgw_config->channel_if[i];
	ifconf.datarate = DR_LORA_SF7;
	ifconf.bandwidth = BW_250KHZ;
	if (lgw_rxif_setconf(i, &ifconf) != LGW_HAL_SUCCESS) {
		printf("failed to configure LoRa service channel\n");
		return EXIT_FAILURE;
	}
	printf("CH%d \tRadio %s \t%.3fMHz \t250kHz (LoRa service band)\r\n", i,
			ifconf.rf_chain ? "B" : "A",
			ifconf.rf_chain ?
					(double) (lgw_config->fb + ifconf.freq_hz) / 1000000.0 :
					(double) (lgw_config->fa + ifconf.freq_hz) / 1000000.0
			);

	// TODO  "chan_FSK":       {"enable": true, "radio": 1, "if":  300000, "bandwidth": 125000, "datarate": 50000}

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

	if(lgw_is_started()) {
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
		return EXIT_FAILURE;
	}

	printf("Gateway started\n");

	return EXIT_SUCCESS;
}

/**
 * Stop command
 * @brief Stop the gateway
 */
static int lgw_stop_cmd(int argc, char **argv) {

	(void) argc;
	(void) argv;

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
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

	return EXIT_SUCCESS;
}

/**
 * EUI command
 * @brief Get the EUI of the concentrator
 */
static int lgw_eui_cmd(int argc, char **argv) {
	(void) argc;
	(void) argv;

	if(!lgw_is_started()) {
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

// For lgw_bench
static 	uint32_t _fCntUp = 0;

/**
 * Bench command
 */
static int lgw_bench_cmd(int argc, char **argv) {

	if (argc != 10) {
		puts("usage: lgw_bench <nbpkt> <sf> <bw> <preamble> <rfpower> <crc> <invert_iq> <size> <pause>");
		puts("nbpkt: Number of packets to transmit before exiting.");
		puts("sf: Spreading factor (7 .. 12).");
		puts("bw: Bandwidth in kHz (125, 250, 500).");
		puts("preamble: Preamble (8 .. 12).");
		puts("rfpower: RF power in dBm (12 .. 27).");
		puts("crc: CRC (on|off: off for a gateway instead of like a device).");
		puts("invert_iq: Invert I/Q (true: tx as a gateway instead of like a device).");
		puts("size: Random data size.");
		puts("pause: Pause in millisec before next transmit.");
		return -1;
	}

	int nb_packets_to_transmit = strtoul(argv[1], NULL, 10);
	const uint32_t spreading_factor = strtoul(argv[2], NULL, 10);
	uint32_t bandwidth = strtoul(argv[3], NULL, 10)  * 1000U;
	const uint16_t preamble = strtoul(argv[4], NULL, 10);
	const uint32_t rfpower = strtoul(argv[5], NULL, 10);
	const bool crc_on = strcmp(argv[6],"on") == 0;
	const bool invert_pol = strcmp(argv[7],"true") == 0;
	const uint32_t size = strtoul(argv[8], NULL, 10);
	const uint32_t pause = strtoul(argv[9], NULL, 10);

	// TODO check value ranges

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_SUCCESS;
	}

	/* TX gain table */
	if (_configure_txlut(&_txlut, lgw_get_pwr_idx(rfpower), lgw_get_pa_gain(rfpower)) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	struct lgw_pkt_tx_s pkt;
	memset(&pkt, 0, sizeof(pkt));

	pkt.tx_mode = IMMEDIATE;
	pkt.rf_chain = 0; //only rf_chain 0 is able to tx
	pkt.rf_power = 0; //use the single entry of the txlut TODO Should be check

	pkt.modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt.coderate = CR_LORA_4_5; // LoRaWAN
	pkt.no_crc = (crc_on != true); 		// LoRaWAN : on for uplink and off for downlink
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

	// Set the payload
	pkt.payload[0] = 0x40; 	/* Confirmed Data Up */
	pkt.payload[1] = 0x34;	/* DevAddr 0xFC001234 */
	pkt.payload[2] = 0x12;
	pkt.payload[3] = 0x00;
	pkt.payload[4] = 0xFC;
	pkt.payload[5] = 0x00; 	/* FCTrl (FOptLen = 0) */
	pkt.payload[6] = 0; 	/* FCnt */
	pkt.payload[7] = 0; 	/* FCnt */
	pkt.payload[8] = 0x02; 	/* FPort */
	for (int i = 9; i < 255; i++) {
		pkt.payload[i] = i;
	}

	while (nb_packets_to_transmit--) {

		// Increment FCnt (32 bit counter)
		pkt.payload[6] = (uint8_t) _fCntUp;
		pkt.payload[7] = (uint8_t) ((_fCntUp >> 8));
		_fCntUp++;

		pkt.freq_hz = _freq_hz_plan[nb_packets_to_transmit
				% ARRAY_SIZE(_freq_hz_plan)];

		printf(
				"Transmitting LoRa packet on %.3fMHz [SF%ldBW%d, TXPOWER %lu, PWID %u, PA %s, ToA %lu msec]\n",
				(double) (pkt.freq_hz / 1E6), pkt.datarate,
				(pkt.bandwidth - 3) * 125,
				rfpower,
				_txlut.lut[0].pwr_idx,
				_txlut.lut[0].pa_gain ? "ON" : "OFF",
				time_on_air);
		for (unsigned int j = 0; j < size; j++) {
			printf("%02X ", pkt.payload[j]);
		}
		printf("\n");

		int x = lgw_send(&pkt);
		if (x != 0) {
			printf("ERROR: failed to send packet\n");
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

/**
 * TX command
 */
static int lgw_tx_cmd(int argc, char **argv) {

	if (argc != 9) {
		puts("usage: lgw_tx <freq> <sf> <bw> <preamble> <rfpower> <crc> <invert_iq> <payload>");
		puts("freq: Frequency in kHz.");
		puts("sf: Spreading factor (7 .. 12).");
		puts("bw: Bandwidth in kHz (125, 250, 500).");
		puts("preamble: Preamble (8 .. 12).");
		puts("rfpower: RF power in dBm (12 .. 27).");
		puts("crc: CRC (on|off: off for a gateway instead of like a device).");
		puts("invert_iq: Invert I/Q (true: tx as a gateway instead of like a device).");
		puts("payload: hexadecimal payload (length < 256).");
		return EXIT_FAILURE;
	}

	const uint32_t frequency = strtoul(argv[1], NULL, 10) * 1000U;
	const uint32_t spreading_factor = strtoul(argv[2], NULL, 10);
	uint32_t bandwidth = strtoul(argv[3], NULL, 10) * 1000U;
	const uint16_t preamble = strtoul(argv[4], NULL, 10);
	const uint32_t rfpower = strtoul(argv[5], NULL, 10);
	const bool crc_on = strcmp(argv[6],"on") == 0;
	const bool invert_pol = strcmp(argv[7],"true") == 0;
	const char* hexpayload = argv[8];

	// TODO check value ranges

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_SUCCESS;
	}

	if(strlen(hexpayload) > 255*2) {
		printf("ERROR: the payload length must be less than 255 bytes-long\n");
		return EXIT_FAILURE;
	}

	struct lgw_pkt_tx_s pkt;
	memset(&pkt, 0, sizeof(pkt));
	size_t size = fmt_hex_bytes(pkt.payload, hexpayload);

	if(size == 0) {
		printf("ERROR: the payload must be a hexadecimal string\n");
		return EXIT_FAILURE;
	}

	/* TX gain table */
	if (_configure_txlut(&_txlut, lgw_get_pwr_idx(rfpower), lgw_get_pa_gain(rfpower)) == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	pkt.tx_mode = IMMEDIATE;
	pkt.rf_chain = 0; //only rf_chain 0 is able to tx
	pkt.rf_power = 0; //use the single entry of the txlut TODO Should be check

	pkt.modulation = MOD_LORA;	// ONLY LoRa (No FSK)
	pkt.coderate = CR_LORA_4_5; // LoRaWAN
	pkt.no_header = false; 		// Beacons have not header
	pkt.no_crc = (crc_on != true); 		// LoRaWAN : on for uplink and off for downlink
	pkt.invert_pol = invert_pol;
	pkt.freq_hz = frequency;
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

	printf(
			"Transmitting LoRa packet on %.3fMHz [SF%ldBW%d, TXPOWER %lu, PWID %u, PA %s, ToA %lu msec]\n",
			(double) (pkt.freq_hz / 1E6), pkt.datarate,
			(pkt.bandwidth - 3) * 125,
			rfpower,
			_txlut.lut[0].pwr_idx,
			_txlut.lut[0].pa_gain ? "ON" : "OFF",
			time_on_air);
	for (unsigned int j = 0; j < size; j++) {
		printf("%02X ", pkt.payload[j]);
	}
	printf("\n");

	int x = lgw_send(&pkt);
	if (x != 0) {
		printf("ERROR: failed to send packet\n");
		return EXIT_FAILURE;
	}

	_lgw_stat.tx++;

	// TODO seconds_between_transmit should be greated than the Time On Air
	printf("Waiting %lu msec\n", time_on_air);
	xtimer_msleep(time_on_air);

	return EXIT_SUCCESS;
}


static uint32_t    _last_count_us = 0;

static void _printf_rxpkt(struct lgw_pkt_rx_s *rxpkt) {

	// TODO print the delta in us between rxpkt->count_us;

	uint32_t count_us = rxpkt->count_us;
	uint32_t delta_count_us;
	if(_last_count_us == 0) {
		delta_count_us = 0;
	} else if(count_us > _last_count_us) {
		delta_count_us = count_us - _last_count_us;
	} else {
		delta_count_us = count_us + UINT32_MAX - _last_count_us;
	}
	_last_count_us = count_us;

	const uint32_t time_on_air = lgw_time_on_air_params(
			rxpkt->modulation,
			rxpkt->bandwidth, // 0x04 for 125000 KHz
			rxpkt->coderate, // 1 for CR4/5
			rxpkt->datarate, // SF
			8, // default in LoRaWAN
			true, /* header is always enabled, except for beacons */
			(rxpkt->status != STAT_NO_CRC),
			rxpkt->size
	);

	printf("\n----- %s packet - TimeOnAir: %lu msec (",
			(rxpkt->modulation == MOD_LORA) ? "LoRa" : "FSK", time_on_air);
	_print_rtc();
	printf(") -----\n");


	printf("  count_us: %lu (delta: %lu)\n", rxpkt->count_us, delta_count_us);
	printf("  size:     %u\n", rxpkt->size);
	printf("  chan:     %u\n", rxpkt->if_chain);
	printf("  status:   0x%02X %s\n", rxpkt->status, _get_status_str(rxpkt->status));
	printf("  datr:     %lu\n", rxpkt->datarate);
	printf("  codr:     %u\n", rxpkt->coderate);
	printf("  rf_chain  %u\n", rxpkt->rf_chain);
	printf("  freq_hz   %lu\n", rxpkt->freq_hz);

	printf("  snr_avg:  %.1f\n", rxpkt->snr);
	printf("  rssi_chan:%.1f\n", rxpkt->rssic);
	printf("  rssi_sig :%.1f\n", rxpkt->rssis);

	printf("  temp     :%.1f\n", rxpkt->temperature);
	printf("  rssi_off :%.1f\n", rxpkt->rssi_temperature_offset);

	printf("  crc:      0x%04X\n", rxpkt->crc);
	for (int j = 0; j < rxpkt->size; j++) {
		printf("%02X ", rxpkt->payload[j]);
	}
	printf("\n");
	lorawan_printf_payload(rxpkt->payload,rxpkt->size);
}

//#define MAX_RX_PKT 4
//static struct lgw_pkt_rx_s rxpkt[MAX_RX_PKT];


static uint8_t previous_size=0;
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

	if(!lgw_is_started()) {
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

	while (!_quit_rx) {
		/* fetch N packets */
		nb_pkt = lgw_receive(MAX_RX_PKT, rxpkt);

		if (nb_pkt != 0) {
			for (int i = 0; i < nb_pkt; i++) {
				_lgw_stat.rx++;

				bool same_as_previous = false;
				if(rxpkt[i].size > 0) {
					if(rxpkt[i].size == previous_size && memcmp((void*)previous_payload, (void*)rxpkt[i].payload, rxpkt[i].size & 0xFF) == 0) {
						same_as_previous = true;
					} else {
						previous_size = (uint8_t) rxpkt[i].size;
						memcpy((void*)previous_payload, (void*)rxpkt[i].payload, previous_size);
					}
				}

				switch (rxpkt[i].status) {
				case STAT_CRC_OK:
					_lgw_stat.rx_ok++;

					nb_pkt_crc_ok += 1;
					if (nb_pkt_crc_ok >= rx_count) {
						_quit_rx = true;
						break;
					}

					// Add repeater function here
					// repeat(&rxpkt[i]);

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

				_printf_rxpkt(rxpkt + i);
				if(same_as_previous) {
					printf("INFO: Same payload than the previous RX\n");
				}

			}
			printf("Received %d packets (total:%lu)\n", nb_pkt, nb_pkt_crc_ok);
		} else {
			// printf("No packet");
		}
	}

	printf("Exiting\n");

	printf("Nb valid packets received: %lu CRC OK\n", nb_pkt_crc_ok);

	return EXIT_SUCCESS;
}


void* _listen_thread(void *arg) {
    (void)arg;
    char* argv[] = {"rx", "100000000", "4"};
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
    (void)argc;
    (void)argv;

	if(!lgw_is_started()) {
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
	_listen_thread_pid = thread_create(_listen_thread_stack, sizeof(_listen_thread_stack),
				THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
				_listen_thread, NULL, "LGW_LISTEN");

	if (_listen_thread_pid <= KERNEL_PID_UNDEF) {
		puts("Creation of rx thread failed");
#if ENABLE_LGW_LISTEN_MALLOC_STACK == 1
		free(_listen_thread_stack);
		_listen_thread_stack=NULL;
#endif
		return EXIT_FAILURE;
	}

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
    } else {
    	printf("[%s] Gateway is listening ...\n", __FUNCTION__);
    }

    return EXIT_SUCCESS;
}
#endif

/**
 * Idle command
 */
static int lgw_idle_cmd(int argc, char **argv) {
    (void)argc;
    (void)argv;

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	_quit_rx = true;
	return EXIT_SUCCESS;
}


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
				printf("INFO: MATCH reg %d (%u, %u)\n", i, rand_values[i], (uint8_t)val);
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
	(void)argv;
    printf("%s reset    : Reset the SX1302\n", argv[0]);
    printf("%s status   : Get the SX1302 status\n", argv[0]);
    printf("%s start    : Start the gateway\n", argv[0]);
    printf("%s stop     : Stop the gateway\n", argv[0]);
    printf("%s stat     : Get stats of the gateway\n", argv[0]);
    printf("%s eui      : Get the concentrator EUI\n", argv[0]);
    printf("%s rx       : Receive radio packet(s)\n", argv[0]);
    printf("%s listen   : Receive radio packet(s) in background Stop to receive radio packet(s) in background (remark: the sx1302 continue to buffer received messages)\n", argv[0]);
    printf("%s idle     : Stop to receive radio packet(s) in background\n", argv[0]);
    printf("%s tx       : Transmit one radio packet\n", argv[0]);
    printf("%s bench    : Transmit a sequence of radio packets\n", argv[0]);
//    printf("%s beacon   : Transmit a beacon packet\n", argv[0]);
//    printf("%s repeater : Repeat received packet(s)\n", argv[0]);
#if ENABLE_REGTEST == 1
    printf("%s reg_test : Test writing and reading from registers\n", argv[0]);
#endif

}

int lgw_cmd(int argc, char **argv) {

	if (argc < 2) {
		_lgw_cmd_usage(argv);
		return EXIT_FAILURE;
	} else if (strcmp(argv[1], "reset") == 0) {
		return lgw_reset_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "status") == 0) {
		return lgw_status_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "start") == 0) {
		return lgw_start_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "stop") == 0) {
		return lgw_stop_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "stat") == 0) {
		return lgw_stat_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "eui") == 0) {
		return lgw_eui_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "rx") == 0) {
		return lgw_rx_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "listen") == 0) {
		return lgw_listen_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "idle") == 0) {
		return lgw_idle_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "tx") == 0) {
		return lgw_tx_cmd(argc-1, argv+1);
	} else if (strcmp(argv[1], "bench") == 0) {
		return lgw_bench_cmd(argc-1, argv+1);
#if ENABLE_REGTEST == 1
	} else if (strcmp(argv[1], "reg_test") == 0) {
		return lgw_reg_cmd(argc-1, argv+1);
#endif
	} else {
		_lgw_cmd_usage(argv);
	    return EXIT_FAILURE;
	}
}



