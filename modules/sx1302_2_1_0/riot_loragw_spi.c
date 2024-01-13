/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    Host specific functions to address the LoRa concentrator registers through
    a SPI interface.
    Single-byte read/write and burst read/write.
    Could be used with multiple SPI ports in parallel (explicit file descriptor)

License: Revised BSD License, see LICENSE.TXT file include in the project
*/

/*
 * Port of SX1302 driver for RIOT
 *
 * Author:
 *   Didier DONSEZ, Université Grenoble Alpes, 2021-2023.
 */

#define ENABLE_DEBUG	1
#include "debug.h"


#include "loragw_spi.h"
#include "periph/spi.h"

#include "board.h"
#include "include/riot_sx1302_pinmapping.h"

#include "spiconf.h"
// TODO should replaced malloc/free by a static array
#include <malloc.h>
#include <string.h>

#define CHUNK_SIZE_MAX 64

#ifndef LGW_BURST_CHUNK
#define LGW_BURST_CHUNK     (CHUNK_SIZE_MAX/2)
#endif

#define READ_ACCESS 0x00
#define WRITE_ACCESS 0x80

#ifndef SPI_PAUSE_DELAY
#define SPI_PAUSE_DELAY 1000
#endif

#if SPI_PAUSE_DELAY == 0
#define SPI_PAUSE
#else
#include "xtimer.h"
#define SPI_PAUSE      xtimer_usleep(SPI_PAUSE_DELAY)
#endif


/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

//#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#if DEBUG_COM == 1
static void printfhex(const uint8_t *data, const uint16_t size)
{
        for (int i=0; i < size; ++i) {
                printf("%02x", data[i]);
        }
        printf("\n");
}


    #define CHECK_NULL(a)               if(a==NULL){printf("%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__);return LGW_REG_ERROR;}
    #define DEBUG_MSG(str)              printf(str)
    #define DEBUG_PRINTF(fmt, args...)  printf(fmt, args)
    #define DEBUG_HEX(data, size)    	  printfhex(data, size)
#else
    #define CHECK_NULL(a)                if(a==NULL){return LGW_SPI_ERROR;}
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define DEBUG_HEX(data, size)
#endif

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#define READ_ACCESS     0x00
#define WRITE_ACCESS    0x80


/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

/* SPI initialization and configuration */
int lgw_spi_open(const char * com_path, void **com_target_ptr) {
    //DEBUG_PRINTF("[%s]: %s\n", __FUNCTION__, spidev_path);
    DEBUG_PRINTF("[%s:%d]\n", __FUNCTION__, __LINE__);

	(void) com_path;
	(void) com_target_ptr;

	spiconf.dev = SX1302_PARAM_SPI;
	spiconf.mode = SPI_MODE_0;
	//spiconf.clk = SPI_CLK_5MHZ; // SPI_CLK_1MHZ
	spiconf.clk = SX1302_PARAM_SPI_CLK_SPEED; // SPI_CLK_1MHZ
	spiconf.cs = (spi_cs_t)SX1302_PARAM_SPI_NSS;

    DEBUG_PRINTF("[%s:%d] SPI_%i clock is %ld\n", __FUNCTION__, __LINE__, spiconf.dev, spiconf.clk);

    //.reset_pin    = SX1302_PARAM_RESET,
    //.power_en_pin = SX1302_PARAM_POWER_EN


    /* Setup SPI for SX1302 */

    // TODO should only be called once for the whole system
    spi_init(spiconf.dev);

    int res;

    res = spi_init_cs(spiconf.dev, spiconf.cs);

#ifdef MODULE_PERIPH_SPI_GPIO_MODE
	DEBUG_PRINTF(
        "[%s:%d] spi_init_with_gpio_mode SPI_%i device (code %i)\n", __FUNCTION__, __LINE__, spiconf.dev);

    spi_gpio_mode_t gpio_modes = {
        .mosi = (GPIO_OUT | SX1302_DIO_PULL_MODE),
        .miso = (SX1302_DIO_PULL_MODE),
        .sclk = (GPIO_OUT | SX1302_DIO_PULL_MODE),
    };
    res += spi_init_with_gpio_mode(spiconf.dev, gpio_modes);
#endif

    if (res != SPI_OK) {
    	DEBUG_PRINTF(
            "[%s:%d] error: failed to initialize SPI_%i device (code %i)\n", __FUNCTION__, __LINE__, spiconf.dev, res);
        return LGW_SPI_ERROR;
    }


    *com_target_ptr = (void *)&spiconf;


    DEBUG_PRINTF("[%s:%d] SPI_%i initialized with success\n", __FUNCTION__, __LINE__, spiconf.dev);
	return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* SPI release */
int lgw_spi_close(const void *com_target) {
    DEBUG_PRINTF("[%s]\n", __FUNCTION__);
	(void) com_target;
    
    _lgw_spi_release();

    //turning chip select into an input (to avoid current leaking in corecell)
    gpio_init(SX1302_PARAM_SPI_NSS,GPIO_IN);

	return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple write */
int lgw_spi_w(const void * com_target, uint8_t spi_mux_target, uint16_t address, uint8_t data) {
    DEBUG_PRINTF("[%s:%d] 0x%4x 0x%2x\n", __FUNCTION__, __LINE__, address, data);
	(void) com_target;


	uint8_t out_buf[4];
	//uint8_t in_buf[4];
	uint8_t command_size = sizeof(out_buf);

	out_buf[0] = spi_mux_target;
	out_buf[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
	out_buf[2] = ((address >> 0) & 0xFF);
	out_buf[3] = data;

	_lgw_spi_acquire();
    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, NULL, command_size);
	_lgw_spi_release();

    SPI_PAUSE;

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple read */
int lgw_spi_r(const void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data) {
    DEBUG_PRINTF("[%s:%d] 0x%4x ", __FUNCTION__, __LINE__, address);
	(void) com_target;

//	return lgw_spi_rb(spi_target, spi_mux_target, address, data, 1);

    uint8_t out_buf[5];
    uint8_t in_buf[5];
    uint8_t command_size = sizeof(out_buf);

    out_buf[0] = spi_mux_target;
    out_buf[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    out_buf[2] = ((address >> 0) & 0xFF);
    out_buf[3] = 0x00;
    out_buf[4] = 0x00;

	_lgw_spi_acquire();

	spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, in_buf, command_size);

	//spi_transfer_bytes(spiconf.dev, spiconf.cs, true, out_buf, in_buf, command_size);
    //SPI_PAUSE;
    //spi_transfer_bytes(spiconf.dev, spiconf.cs, false, NULL, in_buf, command_size);

	_lgw_spi_release();

    *data = in_buf[command_size - 1];

    SPI_PAUSE;

    DEBUG_PRINTF(" 0x%x ", *data); // TODO add data
    DEBUG_MSG("\n");

    return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Single Byte Read-Modify-Write */
int lgw_spi_rmw(const void *com_target, uint8_t spi_mux_target, uint16_t address, uint8_t offs, uint8_t leng, uint8_t data) {
    DEBUG_PRINTF("[%s:%d] %4x %d %d ", __FUNCTION__, __LINE__, address, offs, leng);

    int spi_stat = LGW_SPI_SUCCESS;
    uint8_t buf[4] = "\x00\x00\x00\x00";

    /* Read */
    spi_stat += lgw_spi_r(com_target, spi_mux_target, address, &buf[0]);

    /* Modify */
    buf[1] = ((1 << leng) - 1) << offs; /* bit mask */
    buf[2] = ((uint8_t)data) << offs; /* new data offsetted */
    buf[3] = (~buf[1] & buf[0]) | (buf[1] & buf[2]); /* mixing old & new data */

    /* Write */
    spi_stat += lgw_spi_w(com_target, spi_mux_target, address, buf[3]);

    return spi_stat;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) write */

// TODO should replaced malloc/free by a static array

int lgw_spi_wb(const void *com_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size) {
	(void) com_target;

    DEBUG_PRINTF("[%s:%d] %4x %d ", __FUNCTION__, __LINE__, address, size);
    DEBUG_HEX(data,size);
    DEBUG_MSG("\n");

    uint32_t transfer_size = 3 + size;

    DEBUG_PRINTF("[%s:%d] malloc %ld bytes\n", __FUNCTION__, __LINE__, transfer_size);
    uint8_t *out_buf = malloc(transfer_size);
    if (out_buf == NULL)
    {
    	printf("[%s:%d] Failed to allocate memory for burst write\n", __FUNCTION__, __LINE__);
        return LGW_SPI_ERROR;
    }

    out_buf[0] = spi_mux_target;
    out_buf[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    out_buf[2] = ((address >> 0) & 0xFF);

    memcpy(out_buf + 3, data, size);


    // DEBUG : CHUNK_SIZE_MAX should be LGW_BURST_CHUNK
    _lgw_spi_acquire();
	if (transfer_size > CHUNK_SIZE_MAX) {
		uint32_t chunck_cpt = 0;
	    uint32_t transfer_done = 0;
		while (transfer_done < transfer_size) {
			uint32_t this_transfer_size = transfer_size - transfer_done;
			if (this_transfer_size > CHUNK_SIZE_MAX)
			{
				this_transfer_size = CHUNK_SIZE_MAX;
			}

			spi_transfer_bytes(spiconf.dev, spiconf.cs, this_transfer_size > CHUNK_SIZE_MAX,
					out_buf + transfer_done, NULL, this_transfer_size);

			transfer_done += this_transfer_size;
			chunck_cpt++;
		}
		DEBUG_PRINTF("[%s:%d] transfer %d bytes in %ld chunks\n", __FUNCTION__, __LINE__, size, chunck_cpt);
	} else {
		spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, NULL, transfer_size);
		DEBUG_PRINTF("[%s:%d] transfer %d in 1 chunk\n", __FUNCTION__, __LINE__, size);
	}

	_lgw_spi_release();

    DEBUG_PRINTF("[%s:%d] free %ld bytes\n", __FUNCTION__, __LINE__, transfer_size);
    free(out_buf);

    SPI_PAUSE;

	return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Test avec le code de void sx1302_reg_read_batch(const sx1302_t *dev, uint16_t register_id, uint8_t *data, uint16_t size) {

int lgw_spi_rb(const void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
	DEBUG_PRINTF("[%s:%d] %4x %d ", __FUNCTION__, __LINE__, address, size);

	(void) spi_target;

    uint8_t cmd[4];
    cmd[0] = spi_mux_target;
    cmd[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] = address & 0xFF;
    cmd[3] = 0;

    _lgw_spi_acquire();
    spi_transfer_bytes(spiconf.dev, spiconf.cs, true, cmd, NULL,sizeof(cmd));
    SPI_PAUSE;

    int chunk_size;
    int offset = 0;
    int remain = size;
    /* write memory by chunks */
    while (remain > 0) {
        /* full or partial chunk ? */
        chunk_size = (remain > CHUNK_SIZE_MAX) ? CHUNK_SIZE_MAX : remain;

        /* do the burst write */
        spi_transfer_bytes(spiconf.dev, spiconf.cs,
                           size > CHUNK_SIZE_MAX, NULL, &data[offset],
                           chunk_size);
        SPI_PAUSE;
        /* prepare for next write */
        remain -= chunk_size;
        offset += CHUNK_SIZE_MAX;
    }

    _lgw_spi_release();

    DEBUG_HEX(data, size);
    DEBUG_MSG("\n");

	return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

uint16_t lgw_spi_chunk_size(void) {
    return (uint16_t)LGW_BURST_CHUNK;
}

/* --- EOF ------------------------------------------------------------------ */
