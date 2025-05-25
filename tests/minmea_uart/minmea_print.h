/*
 * minmea_print.h
 *
 *  Created on: May 25, 2025
 *      Author: donsez
 */

#ifndef TESTS_MINMEA_UART_MINMEA_PRINT_H_
#define TESTS_MINMEA_UART_MINMEA_PRINT_H_

#include <stdint.h>

/**
 * Parse the NMEA sentence
 * Return false for invalid checksum, buffer overflow, unknown sentence ...
 */
bool parse_nmea(uint8_t c);

#endif /* TESTS_MINMEA_UART_MINMEA_PRINT_H_ */
