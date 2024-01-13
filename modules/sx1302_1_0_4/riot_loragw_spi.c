/*
  (C)2019 Strataggem

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

Port to RIOT by Didier DONSEZ & Olivier ALPHAND, Université Grenoble-Alpes

*/
#include <string.h>

#include "config.h"

#define ENABLE_DEBUG	1
#include "debug.h"

#include "loragw_aux.h"
#include "loragw_spi.h"
#include "loragw_sx1302.h"
#include "malloc.h"

#include "spiconf.h"

#define CHUNK_SIZE_MAX 64

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

#if DEBUG_SPI == 1
static void printfhex(const uint8_t *data, const uint16_t size)
{
        for (int i=0; i < size; ++i) {
                printf("%02x", data[i]);
        }
        printf("\n");
}

    #define DEBUG_MSG(str)                printf(str)
	#define DEBUG_PRINTF(fmt, args...)    printf("%s:%d: "fmt, __FUNCTION__, __LINE__, args)
    #define DEBUG_RW(fmt, args...)    	  printf(fmt, args)
    #define DEBUG_HEX(data, size)    	  printfhex(data, size)
#else
    #define DEBUG_MSG(str)
    #define DEBUG_PRINTF(fmt, args...)
    #define DEBUG_RW(fmt, args...)
    #define DEBUG_HEX(data, size)
#endif

// static spi_device_handle_t _sxSpiHandle;
// static spi_host_device_t _sxSpiDevice;

/* SPI initialization and configuration */
int lgw_spi_open(const char *spidev_path, void **spi_target_ptr)
{
    //DEBUG_RW("[%s]: %s\n", __FUNCTION__, spidev_path);
    DEBUG_RW("[%s]\n", __FUNCTION__);

	(void) spidev_path;
	(void) spi_target_ptr;

	spiconf.dev = SX1302_PARAM_SPI;
	spiconf.mode = SPI_MODE_0;
	spiconf.clk = SX1302_PARAM_SPI_CLK_SPEED;
	spiconf.cs = (spi_cs_t)SX1302_PARAM_SPI_NSS;

    DEBUG_PRINTF("[%s] SPI_%i clock is %ld\n", __FUNCTION__, spiconf.dev, spiconf.clk);

    //.reset_pin    = SX1302_PARAM_RESET,
    //.power_en_pin = SX1302_PARAM_POWER_EN


    /* Setup SPI for SX1302 */

    // TODO should only be called once for the whole system
    spi_init(spiconf.dev);

    int res;

    res = spi_init_cs(spiconf.dev, spiconf.cs);

#ifdef MODULE_PERIPH_SPI_GPIO_MODE
	DEBUG_RW(
        "[%s] spi_init_with_gpio_mode SPI_%i device (code %i)\n", __FUNCTION__, spiconf.dev);

    spi_gpio_mode_t gpio_modes = {
        .mosi = (GPIO_OUT | SX1302_DIO_PULL_MODE),
        .miso = (SX1302_DIO_PULL_MODE),
        .sclk = (GPIO_OUT | SX1302_DIO_PULL_MODE),
    };
    res += spi_init_with_gpio_mode(spiconf.dev, gpio_modes);
#endif

    if (res != SPI_OK) {
    	DEBUG_RW(
            "[%s] error: failed to initialize SPI_%i device (code %i)\n", __FUNCTION__, spiconf.dev, res);
        return LGW_SPI_ERROR;
    }


    *spi_target_ptr = (void *)&spiconf;


    DEBUG_RW("[%s] SPI_%i initialized with success\n", __FUNCTION__, spiconf.dev);
	return LGW_SPI_SUCCESS;

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* SPI release */
int lgw_spi_close(const void *spi_target)
{
    DEBUG_RW("[%s]\n", __FUNCTION__);
	(void) spi_target;

	return LGW_SPI_SUCCESS;
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Simple write */
int lgw_spi_w(const void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t data)
{
    DEBUG_RW("[%s] 0x%4x 0x%2x\n", __FUNCTION__, address, data);
	(void) spi_target;


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
int lgw_spi_r(const void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data)
{
    DEBUG_RW("[%s] 0x%4x ", __FUNCTION__, address);
	(void) spi_target;

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

    DEBUG_RW(" 0x%x ", *data); // TODO add data
    DEBUG_MSG("\n");

    return LGW_SPI_SUCCESS;

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Burst (multiple-byte) write */
int lgw_spi_wb(const void *spi_target, uint8_t spi_mux_target, uint16_t address, const uint8_t *data, uint16_t size)
{
	(void) spi_target;

    DEBUG_RW("[%s] %4x %d ", __FUNCTION__, address, size);
    DEBUG_HEX(data,size);
    DEBUG_MSG("\n");


    uint32_t transfer_size = 3 + size;
    uint8_t *out_buf = malloc(transfer_size);
    if (out_buf == NULL)
    {
        DEBUG("Failed to allocate memory for burst write");
        return LGW_SPI_ERROR;
    }

    out_buf[0] = spi_mux_target;
    out_buf[1] = WRITE_ACCESS | ((address >> 8) & 0x7F);
    out_buf[2] = ((address >> 0) & 0xFF);

    memcpy(out_buf + 3, data, size);

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
        DEBUG_RW("transfer %d bytes in %ld chunks\n",size,chunck_cpt);
	} else {
		spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, NULL, transfer_size);
        DEBUG_RW("transfer %d bytes in 1 chunk\n",size);
	}
	_lgw_spi_release();

    free(out_buf);

    SPI_PAUSE;

	return LGW_SPI_SUCCESS;

}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Test avec le code de void sx1302_reg_read_batch(const sx1302_t *dev, uint16_t register_id, uint8_t *data, uint16_t size) {

int lgw_spi_rb(const void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
    DEBUG_RW("[%s] %4x %d ", __FUNCTION__, address, size);

	(void) spi_target;

    uint8_t cmd[4];
    cmd[0] = spi_mux_target;
    cmd[1] = READ_ACCESS | ((address >> 8) & 0x7F);
    cmd[2] = address & 0xFF;
    cmd[3] = 0;

    _lgw_spi_acquire();
    spi_transfer_bytes(spiconf.dev, spiconf.cs, true, cmd, NULL,4);
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


#define ENABLE_ONE_SPI_TRANSFER_BYTES 1

/* Burst (multiple-byte) read */
int _lgw_spi_rb(const void *spi_target, uint8_t spi_mux_target, uint16_t address, uint8_t *data, uint16_t size)
{
	(void) spi_target;

    DEBUG_RW("[%s] %4x %d ", __FUNCTION__, address, size); // TODO add data

	uint32_t transfer_size = 4 + size;
	uint32_t transfer_done = 0;

	uint8_t *out_buf = malloc(transfer_size);
	uint8_t *in_buf = malloc(transfer_size);
	if (out_buf == NULL || in_buf == NULL)
	{
		DEBUG("Failed to allocate memory for burst read");
		return LGW_SPI_ERROR;
	}

	out_buf[0] = spi_mux_target;
	out_buf[1] = READ_ACCESS | ((address >> 8) & 0x7F);
	out_buf[2] = ((address >> 0) & 0xFF);
	out_buf[3] = 0x00;

    _lgw_spi_acquire();
    if (transfer_size > CHUNK_SIZE_MAX)
    {
    	while (transfer_done < transfer_size)
    	{
    		uint32_t this_transfer_size = transfer_size - transfer_done;
    		if (this_transfer_size > CHUNK_SIZE_MAX)
    		{
    			this_transfer_size = CHUNK_SIZE_MAX;
    		}

#if ENABLE_ONE_SPI_TRANSFER_BYTES == 1
		    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf + transfer_done, in_buf, transfer_size);
		    SPI_PAUSE;
#else
		    spi_transfer_bytes(spiconf.dev, spiconf.cs, true, out_buf, NULL, 4);
		    SPI_PAUSE;

		    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, NULL, in_buf + transfer_done, transfer_size);
		    SPI_PAUSE;
#endif
    		transfer_done += this_transfer_size;
    	}
    }
    else
    {
#if ENABLE_ONE_SPI_TRANSFER_BYTES == 1
	    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, out_buf, in_buf, transfer_size);
	    SPI_PAUSE;
#else
	    spi_transfer_bytes(spiconf.dev, spiconf.cs, true, out_buf, NULL, 4);
	    SPI_PAUSE;

	    spi_transfer_bytes(spiconf.dev, spiconf.cs, false, NULL, in_buf, transfer_size);
	    SPI_PAUSE;
#endif
    }
    _lgw_spi_release();
	memcpy(data, in_buf + 4, size);

    DEBUG_HEX(in_buf,size + 4);
    DEBUG_MSG("\n");

	free(out_buf);
	free(in_buf);

    SPI_PAUSE;


    DEBUG_HEX(data,size);
    DEBUG_MSG("\n");

	return LGW_SPI_SUCCESS;
}
