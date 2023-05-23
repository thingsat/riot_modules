/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Functions used to handle LoRa concentrator SX1250 radios.

License: Revised BSD License, see LICENSE.TXT file include in the project
*/


/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h>     /* C99 types */
#include <stdio.h>      /* printf fprintf */

#include "sx1250_com.h"
#include "sx1250_spi.h"

#ifndef RIOTOS_PORT
#include "sx1250_usb.h"
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
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
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int sx1250_com_w(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx1250_spi_w(com_target, spi_mux_target, op_code, data, size);
            break;
#ifndef RIOTOS_PORT
        case LGW_COM_USB:
            com_stat = sx1250_usb_w(com_target, spi_mux_target, op_code, data, size);
            break;
#endif
        default:
            printf("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            com_stat = LGW_COM_ERROR;
            break;
    }

    return com_stat;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int sx1250_com_r(lgw_com_type_t com_type, void *com_target, uint8_t spi_mux_target, sx1250_op_code_t op_code, uint8_t *data, uint16_t size) {
    int com_stat;

    /* Check input parameters */
    CHECK_NULL(com_target);
    CHECK_NULL(data);

    switch (com_type) {
        case LGW_COM_SPI:
            com_stat = sx1250_spi_r(com_target, spi_mux_target, op_code, data, size);
            break;
#ifndef RIOTOS_PORT
        case LGW_COM_USB:
            com_stat = sx1250_usb_r(com_target, spi_mux_target, op_code, data, size);
            break;
#endif
        default:
            printf("ERROR: wrong communication type (SHOULD NOT HAPPEN)\n");
            com_stat = LGW_COM_ERROR;
            break;
    }

    return com_stat;
}

/* --- EOF ------------------------------------------------------------------ */
