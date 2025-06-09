/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */


#ifndef APPS_BASIC_MISSION_MISSION_H_
#define APPS_BASIC_MISSION_MISSION_H_

// TODO change for enum
#define RANGE1_PAYLOAD			0
#define TELEMETRY_PAYLOAD		1
#define APRS_LORAWAN_PAYLOAD	2
#define APRS_AX25_PAYLOAD		3
#define EWSS_PAYLOAD			4

#include "loragw_hal.h"

void mission_periodic_cb(struct lgw_pkt_tx_s *lgw_pkt_tx_s);

void mission_gnss_print(void);

void mission_inst_cnt_print(void);


#endif /* APPS_BASIC_MISSION_MISSION_H_ */
