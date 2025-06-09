/*
 Thingsat project

 GPS over UART
 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */

#ifndef _PACK_COORD_H
#define _PACK_COORD_H

#include <stdint.h>

// Convert GPS positions from double to binary values.
uint32_t gps_pack_latitude_f_to_i24(const float lat);

// Convert GPS positions from double to binary values.
uint32_t gps_pack_longitude_f_to_i24(const float lon);

float gps_unpack_latitude_i24_to_f(const uint32_t latitude_i24);

float gps_unpack_longitude_i24_to_f(const uint32_t longitude_i24);


#endif
