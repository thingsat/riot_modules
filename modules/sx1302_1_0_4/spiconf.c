
#include "spiconf.h"

spiconf_t spiconf;

bool spi_initialized = false;


void _lgw_spi_acquire(void) {
    spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk);
}

void _lgw_spi_release(void) {
    spi_release(spiconf.dev);
}
