#ifndef _MESHCORE_UTILS_CONTACT_H_
#define _MESHCORE_UTILS_CONTACT_H_

#include "meshcore_constants.h"

// See Identity https://github.com/meshcore-dev/MeshCore/blob/main/src/Identity.h

typedef struct Identity {
  uint8_t pub_key[PUB_KEY_SIZE];
} Identity_t;

// See ContactInfo https://github.com/meshcore-dev/MeshCore/blob/main/src/helpers/ContactInfo.h

typedef struct ContactInfo {
  Identity_t id;
  char name[32];
  uint8_t type;   // on of ADV_TYPE_*
  uint8_t flags; 
  uint8_t out_path_len;
  mutable bool shared_secret_valid; // flag to indicate if shared_secret has been calculated
  uint8_t out_path[MAX_PATH_SIZE];
  uint32_t last_advert_timestamp;   // by THEIR clock
  uint32_t lastmod;  // by OUR clock
  int32_t gps_lat, gps_lon;    // 6 dec places
  uint32_t sync_since;

  uint8_t last_sf; //  SF of last received packet

  uint8_t shared_secret[PUB_KEY_SIZE]; // what is the role of this field ?
} ContactInfo_t;

#endif