/*
 Thingsat Mission
 Copyright (c) 2021-2026 Université Grenoble Alpes CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes CSUG LIG
 */

#ifndef TLE_UTILS_H
#define TLE_UTILS_H

#include <stdint.h>
#include <time.h>

/**
 * @brief Init TLE
 */
void init_tle(const char* tle_name, const char* tle_line1, const char* tle_line2);


/**
 * @brief Predict the GPS at a timestamp (from TLE)
 * @param secondsSinceEpoch the timestamp
 * @param p_dlat the result for latitude
 * @param p_dlon the result for longitude
 */
void predict_gps_from_tle_at_epoch(time_t secondsSinceEpoch, double* p_dlat, double* p_dlon);

#endif

