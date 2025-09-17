/*!
 * \file      sx1280_hal.c
 *
 * \brief     Implements the sx1280 HAL functions
 *
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>     // C99 types
#include <stdbool.h>    // bool type
#include <stdio.h>

#include "xtimer.h"
#include "irq.h"
#include "periph/spi.h"

#include "sx1280_hal.h"

#ifndef ENABLE_ONE_SPI_TRANSFER_BYTES
#define ENABLE_ONE_SPI_TRANSFER_BYTES 8
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

// This variable will hold the current operating mode of the radio
static volatile sx1280_hal_operating_mode_t radio_opmode;

static struct {
    spi_t dev;
    spi_mode_t mode;
    spi_clk_t clk;
    spi_cs_t cs;
} spiconf;

static bool spi_acquired;
static bool hal_initialized;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS             -------------------------------------------
 */

/**
 * @brief Wait until radio busy pin is reset to 0
 */
static void sx1280_hal_wait_on_busy( void )
{
    while (gpio_read(SX1280_PARAM_DIO0)) {}
}

/**
    @brief     Blocking wait for a given amount of time.
    @param[in] period Time to wait in ms.
 */
static void _delay_ms(uint32_t period)
{
    xtimer_usleep(period * 1000);
}


static inline bool _prepare_spi(void) {
	if (hal_initialized && !spi_acquired) {
		spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk);
		spi_acquired = true;
	}

	return spi_acquired;
}

static inline void _release_spi(void) {
	if (spi_acquired) {
		spi_acquired = false;
		spi_release(spiconf.dev);
	}
}

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS            ---------------------------------------------
 */

sx1280_hal_status_t sx1280_hal_write( const void *context, const uint8_t *command,
                                      const uint16_t command_length,
                                      const uint8_t *data, const uint16_t data_length )
{
    sx1280_hal_wakeup( context );
	if (!_prepare_spi()) {
		return SX1280_HAL_STATUS_ERROR;
	}

    //printf("sx1280_hal_write: command_length=%d data_length=%d\n", command_length, data_length);
    if (command_length) {
        spi_transfer_bytes(spiconf.dev, spiconf.cs, data_length, command, NULL, command_length);
    }
    if (data_length) {
        spi_transfer_bytes(spiconf.dev, spiconf.cs, false, data, NULL, data_length);
    }

	_release_spi();

    // 0x84 - SX1280_SET_SLEEP opcode. In sleep mode the radio dio is struck to 1 => do not test it
    if (command[0] != 0x84) {
        sx1280_hal_wait_on_busy( );
    }

    return SX1280_HAL_STATUS_OK;
}

sx1280_hal_status_t sx1280_hal_read( const void *context, const uint8_t *command,
                                     const uint16_t command_length,
                                     uint8_t *data, const uint16_t data_length )
{
	sx1280_hal_wakeup( context );
	if (!_prepare_spi()) {
		return SX1280_HAL_STATUS_ERROR;
	}

#if ENABLE_ONE_SPI_TRANSFER_BYTES == 1

	if(command_length == data_length) {
	    printf("sx1280_hal_read: command_length=%d data_length=%d\n", command_length, data_length);
		spi_transfer_bytes(spiconf.dev, spiconf.cs, false, command, data, command_length);
	} else {
		if (command_length) {
			spi_transfer_bytes(spiconf.dev, spiconf.cs, data_length, command, NULL, command_length);
		}
		if (data_length) {
			spi_transfer_bytes(spiconf.dev, spiconf.cs, false, NULL, data, data_length);
		}
	}
#else

	if (command_length) {
		spi_transfer_bytes(spiconf.dev, spiconf.cs, data_length, command, NULL, command_length);
	}
	if (data_length) {
		spi_transfer_bytes(spiconf.dev, spiconf.cs, false, NULL, data, data_length);
	}

#endif

	_release_spi();
    return SX1280_HAL_STATUS_OK;
}

void sx1280_hal_reset( const void *context )
{
	(void)context;

    gpio_clear(SX1280_PARAM_RESET);
    _delay_ms(5);
    gpio_set(SX1280_PARAM_RESET);

	printf("BUSY check ON\n");
	while (!gpio_read(SX1280_PARAM_DIO0)) {}; // wait ON
	printf("BUSY check OFF\n");
	sx1280_hal_wait_on_busy(); // wait OFF
	printf("BUSY check OK\n");

    _delay_ms(1);
}

sx1280_hal_status_t sx1280_hal_wakeup( const void *context )
{
    (void)context;
	if (!hal_initialized) {
		return SX1280_HAL_STATUS_ERROR;
	}

    if (radio_opmode == SX1280_HAL_OP_MODE_SLEEP) {
        // Busy is HIGH in sleep mode, wake-up the device
        gpio_clear(SX1280_PARAM_SPI_NSS);
        sx1280_hal_wait_on_busy( );
        gpio_set(SX1280_PARAM_SPI_NSS);

        // Radio is awake in STDBY_RC mode
        radio_opmode = SX1280_HAL_OP_MODE_STDBY_RC;

    }
    else {
        // if the radio is awake, just wait until busy pin get low
        sx1280_hal_wait_on_busy( );
    }

    return SX1280_HAL_STATUS_OK;
}

sx1280_hal_operating_mode_t sx1280_hal_get_operating_mode( const void *context )
{
	(void)context;
    return radio_opmode;
}

void sx1280_hal_set_operating_mode( const void *context, const sx1280_hal_operating_mode_t op_mode )
{
	(void)context;
    radio_opmode = op_mode;
}

/*
 * -----------------------------------------------------------------------------
 * --- ADDONS                      ---------------------------------------------
 */
#include "ral_sx1280.h"

static const radio_callbacks_t *radio_cb;

#define CB_DISPATCH(CB) do { if (radio_cb->CB) { radio_cb->CB(); } } while (0)
void sx1280_hal_dispatch( const ral_irq_t irq_status )
{
    if (irq_status & RAL_IRQ_RX_DONE) {
        if (irq_status & RAL_IRQ_RX_CRC_ERROR) {
            CB_DISPATCH(rxError);
        }
        else {
            CB_DISPATCH(rxDone);
        }
    }
    else if (irq_status & RAL_IRQ_TX_DONE) {
        CB_DISPATCH(txDone);

    }
    else if (irq_status & RAL_IRQ_RX_HDR_OK) {
        CB_DISPATCH(rxHeaderDone);

    }
    else if (irq_status & RAL_IRQ_RX_HDR_ERROR) {
        CB_DISPATCH(rxError);

    }
    else if (irq_status & RAL_IRQ_RX_TIMEOUT) {
        CB_DISPATCH(rxTimeout);
    }
}


void sx1280_hal_setup_io( const radio_callbacks_t *cb  )
{
    radio_cb = cb;

    spiconf.dev = SX1280_PARAM_SPI;
    spiconf.mode = SPI_MODE_0;
    spiconf.clk = SPI_CLK_5MHZ;
    spiconf.cs = (spi_cs_t)SX1280_PARAM_SPI_NSS;

    /* test setup */
    int tmp = spi_init_cs(spiconf.dev, spiconf.cs);

    if (tmp != SPI_OK) {
        puts("error: unable to initialize the given chip select line");
        return;
    }

	spi_acquire(spiconf.dev, spiconf.cs, spiconf.mode, spiconf.clk); // should assert if parameters are wrong
	spi_release(spiconf.dev);


    if (gpio_init(SX1280_PARAM_RESET, GPIO_OUT) < 0) {
        printf("Error to initialize SX1280_PARAM_RESET\n");
        return;
    }

    if (gpio_init(SX1280_PARAM_DIO0, GPIO_IN) < 0) {
        printf("Error to initialize SX1280_PARAM_DIO0\n");
        return;
    }

    _delay_ms(2);

	printf("SX1280 HAL initialized\n");
	hal_initialized = true;
}


void sx1280_hal_setup_irq(gpio_cb_t evt_handler )
{
    if (gpio_init_int(SX1280_PARAM_DIO1, GPIO_IN, GPIO_RISING, evt_handler, NULL) < 0) {
        printf("Error to initialize SX1280_PARAM_DIO1\n");
        return;
    }
}

/* --- EOF ------------------------------------------------------------------ */
