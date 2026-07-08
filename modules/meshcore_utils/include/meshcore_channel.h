#ifndef _MESHCORE_UTILS_CHANNEL_H_
#define _MESHCORE_UTILS_CHANNEL_H_

#include "meshcore_constants.h"

typedef struct GroupChannel {
public:
  uint8_t hash[PATH_HASH_SIZE];
  uint8_t secret[PUB_KEY_SIZE];
} GroupChannel_t;

struct ChannelDetails {
  GroupChannel channel;
  char name[32];
};

#endif