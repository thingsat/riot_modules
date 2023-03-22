/*
 * Copyright (C) 2020-2022 Université Grenoble Alpes
 */

#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <stdint.h>

/**
 * @brief Check if a location is into one of the geofenceed areas
 *
 * @param lat the latitude to the point to check
 * @param lng the longitude to the point to check
 * @return the index of the area or -1 if the point is outside of the areas
 */
int8_t check_geofence(float lat, float lng);

/**
 * @brief Print if a location is into one of the areas or not
 *
 * @param lat the latitude to the point to check
 * @param lng the longitude to the point to check
 * @return the index of the area or -1 if the point is outside of the areas
 */
int8_t print_geofence(float lat, float lng);

#endif
