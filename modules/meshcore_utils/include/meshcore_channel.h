/*
 * Copyright (C) 2026 Université Grenoble Alpes - CSUG - LIG
 */

/*
 * Utils for MeshCore mesh network
 */

 #ifndef _MESHCORE_UTILS_CHANNEL_H_
#define _MESHCORE_UTILS_CHANNEL_H_

#include "meshcore_constants.h"

typedef struct GroupChannel {
public:
  uint8_t hash[PATH_HASH_SIZE];
  uint8_t secret[PUB_KEY_SIZE]; // AES128 --> Size is 16 (not 32) PRIV_SHARED_KEY
} GroupChannel_t;

typedef struct ChannelDetails {
  GroupChannel_t channel;
  char name[32];
} ChannelDetails_t;

#endif