#ifndef _SPICONF_H
#define _SPICONF_H

#include "periph/spi.h"

typedef struct {
    spi_t dev;
    spi_mode_t mode;
    spi_clk_t clk;
    spi_cs_t cs;
    // TODO add reset pin
    // TODO add power pin
} spiconf_t;

extern spiconf_t spiconf;

extern bool spi_initialized;


void _lgw_spi_acquire(void);

void _lgw_spi_release(void);



#endif //_SPICONF_H
