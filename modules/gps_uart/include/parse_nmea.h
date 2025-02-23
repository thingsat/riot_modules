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

#ifndef _PARSE_NMEA_H
#define _PARSE_NMEA_H

#include <time.h>

void parse_nmea(uint8_t data);

/**
 * Get the number of errors during the NMEA decoding.
 */
uint16_t parse_nmea_get_global_error(void);

/**
 * Get the number of unknown sentences during the NMEA decoding.
 */
uint16_t parse_nmea_get_global_unknown(void);

/**
 * Get the GPS fix.
 */
bool gps_get_fix(void);

/**
 * Get the seconds since last fix.
 */
uint16_t gps_get_seconds_since_last_fix(void);

/**
 * Get GPS UTC date/time representation to a UNIX timestamp.
 */
bool gps_get_time(struct timespec *ts);

/**
 * Set RTC with GPS UTC date/time.
 */
bool gps_set_rtc_time(void);

/**
 * Get GPS position.
 */
bool gps_get_position(float *latitude, float *longitude, float *altitude);

/**
 * Get GPS speed and track.
 */
bool gps_get_speed_direction(float *speed_kph, float *true_track_degrees);

/**
 * Get Dilution of Precision
 * @see https://en.wikipedia.org/wiki/Dilution_of_precision_(navigation)
 */
bool gps_get_dop(float *pdop, float *hdop, float *vdop);

/**
 * Get GPS quality.
 */
bool gps_get_quality(int* fix_quality, int *satellites_tracked);

/**
 * Get GPS data.
 */
void gps_print(void);

#endif
