/*
 * Copyright (C) 2026 Université Grenoble Alpes - CSUG - LIG
 */

/*
 * Utils for MeshCore mesh network
 */

 #ifndef _MESHCORE_UTILS_PAYLOADS_H_
#define _MESHCORE_UTILS_PAYLOADS_H_

#include "meshcore_constants.h"

// https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md?plain=1

/**
* For Node advertisement
*/

#define ADVERT_APPDATA_MASK_IS_CHAT_NODE             (0x01U)
#define ADVERT_APPDATA_MASK_IS_REPEATER              (0x02U)
#define ADVERT_APPDATA_MASK_IS_ROOM_SERVER           (0x04U)
#define ADVERT_APPDATA_MASK_IS_SENSOR                (0x08U)
#define ADVERT_APPDATA_MASK_HAS_LOCATION             (0x10U)
#define ADVERT_APPDATA_MASK_HAS_FEATURE_1            (0x20U)
#define ADVERT_APPDATA_MASK_HAS_FEATURE_2            (0x40U)
#define ADVERT_APPDATA_MASK_HAS_NAME                 (0x80U)



#endif