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
#include <stdio.h>      /* printf fprintf */

#include "sx125x_com.h"
#include "sx125x_spi.h"
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
    #define CHECK_NULL(a)                if(a==NULL){return -1;}
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE TYPES -------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx125x_com_r(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t *data) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx125x_spi_r(com_target, spi_mux_target, address, data);
            break;
#ifndef RIOT_APPLICATION
        case LGW_COM_USB:
            printf("ERROR: USB COM type is not supported for sx125x\n");
            return -1;
#endif
        default:
            printf("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            return -1;
    }

    return com_stat;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx125x_com_w(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, uint8_t address, uint8_t data) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx125x_spi_w(com_target, spi_mux_target, address, data);
            break;
#ifndef RIOT_APPLICATION
        case LGW_COM_USB:
            printf("ERROR: USB COM type is not supported for sx125x\n");
            return -1;
#endif
        default:
            printf("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            return -1;
    }

    return com_stat;
}

/* --- EOF ------------------------------------------------------------------ */
