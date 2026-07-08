#ifndef _MESHCORE_UTILS_STATS_H_
#define _MESHCORE_UTILS_STATS_H_

#include "meshcore_constants.h"

// From https://github.com/meshcore-dev/MeshCore/blob/main/docs/stats_binary_frames.md

typedef struct __attribute__((packed)) StatsCore {
    uint8_t  response_code;  // 0x18
    uint8_t  stats_type;     // 0x00 (STATS_TYPE_CORE)
    uint16_t battery_mv;
    uint32_t uptime_secs;
    uint16_t errors;
    uint8_t  queue_len;
} StatsCore_t;


typedef struct __attribute__((packed)) StatsRadio {
    uint8_t  response_code;  // 0x18
    uint8_t  stats_type;     // 0x01 (STATS_TYPE_RADIO)
    int16_t  noise_floor;
    int8_t   last_rssi;
    int8_t   last_snr;       // Divide by 4.0 to get actual SNR in dB
    uint32_t tx_air_secs;
    uint32_t rx_air_secs;
} StatsRadio_t;

typedef struct  __attribute__((packed)) StatsPackets {
    uint8_t  response_code;  // 0x18
    uint8_t  stats_type;     // 0x02 (STATS_TYPE_PACKETS)
    uint32_t recv;
    uint32_t sent;
    uint32_t flood_tx;
    uint32_t direct_tx;
    uint32_t flood_rx;
    uint32_t direct_rx;
    uint32_t recv_errors;    // present when frame size is 30
} StatsPackets_t;


#endif