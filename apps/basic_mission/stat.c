/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_STAT
#include "debug.h"

#include "stat.h"


stat_lgw_t stat_lgw = {
		.rx = 0,
		.rx_bad_crc = 0,
		.rx_friends = 0,
		.rx_bad_mic = 0,
		.tx = 0,
		.tx_repeat = 0
};

stat_ranging_t stat_ranging = {
		.range1_tx = 0,
		.range1_rx = 0,
		.range2_rx = 0,
		.range3_rx = 0,
		.range2_rx_replies = 0,
};


