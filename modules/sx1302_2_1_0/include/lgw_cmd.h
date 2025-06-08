/*
 SX1302 LGW commands
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LGW_CMD_H
#define _LGW_CMD_H

/**
 * The `lgw` shell command
 */
int lgw_cmd(int argc, char **argv);

/**
 * Get number of channels in the frequency plan
 */
uint32_t lgw_frequency_plan_size(void);

/**
 * Get the central frequency of the channel
 */
uint32_t lgw_get_freq_hz(const uint8_t channel);


#if INVOKE_CALLBACKS == 1

#include "loragw_hal.h"

extern void (*pkt_rx_cb)(const struct lgw_pkt_rx_s*, struct lgw_pkt_tx_s*);
extern void (*pkt_period_cb)(struct lgw_pkt_tx_s*);
#endif


#endif //_LGW_CMD_H
