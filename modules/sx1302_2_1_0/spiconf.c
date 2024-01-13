/*
 * Port of SX1302 driver for RIOT
 *
 * Author:
 *   Didier DONSEZ, Université Grenoble Alpes, 2021-2023.
 */

#include "spiconf.h"

spiconf_t spiconf;

bool spi_initialized = false;


void _lgw_spi_acquire(void) {
    spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk);
}

void _lgw_spi_release(void) {
    spi_release(spiconf.dev);
}
