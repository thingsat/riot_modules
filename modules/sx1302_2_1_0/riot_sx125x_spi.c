/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1255/SX1257 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

/*
 * Port of SX1302 driver for RIOT
 *
 * Author:
 *   Didier DONSEZ, Université Grenoble Alpes, 2021-2023.
 */

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdbool.h>    /* bool type */
#include <stdio.h>      /* printf fprintf */
#include <string.h>     /* memset */

#ifndef RIOT_APPLICATION
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#else
#include "spiconf.h"
#endif

#include "sx125x_spi.h"
#include "loragw_spi.h"
#include "loragw_reg.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#if DEBUG_RAD == 1
    #define DEBUG_MSG(str)              printf(str)
    #define DEBUG_PRINTF(fmt, args...)  printf(fmt, args)
    #define CHECK_NULL(a)               if(a==NULL){printf("%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_REG_ERROR;}
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define CHECK_NULL(a)               if(a==NULL){return LGW_SPI_ERROR;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE TYPES -------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define READ_ACCESS     0x00
#define WRITE_ACCESS    0x80

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

/* Simple read */
int sx125x_spi_r(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t *data) {
    uint8_t out_buf[3];
    uint8_t command_size;
    uint8_t in_buf[ARRAY_SIZE(out_buf)];

    /* check input variables */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = READ_ACCESS | (address & 0x7F);
    out_buf[2] = 0x00;
    command_size = 3;

    /* I/O transaction */
#ifndef RIOT_APPLICATION
    int com_device = *(int *)com_target;
    struct spi_ioc_transfer k;
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.rx_buf = (unsigned long) in_buf;
    k.len = command_size;
    k.cs_change = 0;
    int a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI READ FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        //DEBUG_MSG("Note: SPI read success\n");
        *data = in_buf[command_size - 1];
        return LGW_SPI_SUCCESS;
    }
#else
    _lgw_spi_acquire();
    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, in_buf, command_size);
    _lgw_spi_release();
    return LGW_SPI_SUCCESS;
#endif
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx125x_spi_w(void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t data) {
    uint8_t out_buf[3];
    uint8_t command_size;

    /* check input variables */
    CHECK_NULL(com_target);

    /* prepare frame to be sent */
    out_buf[0] = spi_mux_target;
    out_buf[1] = WRITE_ACCESS | (address & 0x7F);
    out_buf[2] = data;
    command_size = 3;

    /* I/O transaction */
#ifndef RIOT_APPLICATION
    int com_device = *(int *)com_target; /* must check that spi_target is not null beforehand */
    struct spi_ioc_transfer k;
    memset(&k, 0, sizeof(k)); /* clear k */
    k.tx_buf = (unsigned long) out_buf;
    k.len = command_size;
    k.speed_hz = SPI_SPEED;
    k.cs_change = 0;
    k.bits_per_word = 8;
    int a = ioctl(com_device, SPI_IOC_MESSAGE(1), &k);

    /* determine return code */
    if (a != (int)k.len) {
        DEBUG_MSG("ERROR: SPI WRITE FAILURE\n");
        return LGW_SPI_ERROR;
    } else {
        //DEBUG_MSG("Note: SPI write success\n");
        return LGW_SPI_SUCCESS;
    }
#else
    _lgw_spi_acquire();
    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, NULL, command_size);
    _lgw_spi_release();
    return LGW_SPI_SUCCESS;
#endif
}

/* --- EOF ------------------------------------------------------------------ */
