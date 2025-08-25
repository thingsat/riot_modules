/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef APPS_BASIC_MISSION_REPEAT_H_
#define APPS_BASIC_MISSION_REPEAT_H_


#include <stdint.h>
#include <stdbool.h>
#include "loragw_hal.h"
#include "endpoints.h"

/**
 * Process repeat
 */
bool basic_mission_repeat_process(const struct lgw_pkt_rx_s *pkt_rx,
		struct lgw_pkt_tx_s *pkt_tx, const lorawan_endpoint_t* rx_endpoint);

/**
 * Filter
 */
void basic_mission_filter(uint32_t devaddr_subnet, uint32_t devaddr_mask);

/**
 * SNR threshold
 */
void basic_mission_snr_threshold(int snr_threshold);

/**
 * Repeat enable/disable
 */
void basic_mission_repeat_enable(bool enable);


/**
 * Repeat check if enabled
 */
bool basic_mission_repeat_is_enabled(void);


#endif /* APPS_BASIC_MISSION_REPEAT_H_ */
