/*
 SX1302 LGW Utilities
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */


#ifndef _LGW_UTILS_H_
#define _LGW_UTILS_H_

#include "loragw_hal.h"

uint8_t lgw_bandwidthToBwLoRaCode(uint32_t bandwidthInHz);

uint32_t lgw_bandwidthToBwLoRaHz(uint8_t bwCode);

const char* lgw_get_status_str(uint8_t status);

const char* lgw_get_modulation_str(uint8_t modulation);

const char* lgw_get_tx_mode_str(const uint8_t tx_mode);

uint32_t lgw_get_delta_instcnt(const uint32_t start, const uint32_t end);

void lgw_print_info(void);

int lgw_print_rtc(void);

void lgw_printf_rxpkt(const struct lgw_pkt_rx_s *rxpkt);


#endif /* _LGW_UTILS_H_ */
