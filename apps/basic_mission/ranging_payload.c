/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>
#include <string.h>
#include <stdio.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_RANGING
#include "debug.h"

#include "loragw_hal.h"
#include "lgw_utils.h"
#include "lorawan_printf.h"
#include "parse_nmea.h"
#include "pack_coord.h"
#include "common_payload.h"

#include "ranging.h"
#include "stat.h"


void ranging_payload_range1_printf(const Ranging01Payload_t* p){
	common_payload_tx_printf(&p->tx);
	common_payload_packed_location_printf(&p->location);
}

void ranging_payload_range2_printf(const Ranging2Payload_t* p){
	common_payload_tx_printf(&p->tx);
	printf("  devaddr1       : %08lx\n", p->devaddr1);
	printf("  fcnt1          : %u\n", p->fcnt1);
	printf("  rx_uscount     : %lu\n", p->rx_uscount);
	if(p->rx_ftime != RANGING_NO_FTIME) {
		printf("  rx_ftime       : %lu\n", p->rx_ftime);
	}
	printf("  snr1           : %d\n", p->snr1);
	printf("  rssi1          : %d\n", p->rssi1 * -1);
	printf("  distance12     : %lu\n", p->distance12);
	common_payload_packed_location_printf(&p->location);
}

void ranging_payload_range3_printf(const Ranging3Payload_t* p){
	common_payload_tx_printf(&p->tx);
	printf("  devaddr2       : %08lx\n", p->devaddr2);
	printf("  fcnt2          : %u\n", p->fcnt2);
	printf("  rx_uscount     : %lu\n", p->rx_uscount);
	if(p->rx_ftime != RANGING_NO_FTIME) {
		printf("  rx_ftime       : %lu\n", p->rx_ftime);
	}
	printf("  snr2           : %d\n", p->snr2);
	printf("  rssi2          : %d\n", p->rssi2 * -1);
	printf("  distance12     : %lu\n", p->distance12);
	common_payload_packed_location_printf(&p->location);
}
