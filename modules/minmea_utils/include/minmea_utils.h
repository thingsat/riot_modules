/*
 * Copyright (C) 2025 Université Grenoble Alpes
 */

/*
 * Utils for MINMEA
 *      Author: Didier Donsez
 *
 */

#ifndef _MINMEA_MINMEA_UTILS_H_
#define _MINMEA_MINMEA_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Add a char for parsing the potential NMEA sentence when \n is found
 * Return false for invalid checksum, buffer overflow, unknown sentence ...
 */
bool minmea_parse_and_print(uint8_t c);

#endif /* _MINMEA_MINMEA_UTILS_H_ */
