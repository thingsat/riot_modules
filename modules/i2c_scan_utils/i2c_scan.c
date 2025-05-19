/*
 Thingsat Mission
 Copyright (c) 2021-2024 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 * @brief       An I2C bus scanner
 *
 * @author      Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 * @author		Didier Donsez (update mission_i2c_scan)
 * @author		Leo Cordier (mission_i2c_scan_and_check)
 *
 * @}
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "periph/i2c.h"

static int get_dev(i2c_t *dev, const int idx) {
	if ((idx < 0) || (idx >= (int) I2C_NUMOF)) {
		printf("ERROR: I2C device with number \"%d\" not found\n", idx);
		return -1;
	} else {
		*dev = I2C_DEV(idx);
		return 0;
	}
}

static inline int is_addr_reserved(uint16_t addr) {
	if ((addr < 0x8) || (addr > 0x77))
		return 1;

	return 0;
}

int mission_i2c_scan(const int idx) {
	i2c_t dev;
	if (get_dev(&dev, idx)) {
		return -1;
	}

	printf("INFO: Scanning I2C device %d...\n", idx);
	i2c_acquire(dev);

	puts(
			"addr not ack'ed = \"-\", addr ack'ed = \"X\", addr reserved = \"R\", error = \"E\"\n"
					"     0 1 2 3 4 5 6 7 8 9 a b c d e f");

	for (char i = 0; i < 8; i++) {
		char row[] = { '0', 'x', '0' + i, '0', '\0' };
		fputs(row, stdout);
		uint16_t addr = i;
		addr <<= 4;
		for (unsigned j = 0; j < 16; j++) {
			char str[] = { ' ', '-', '\0' };
			if (is_addr_reserved(addr)) {
				str[1] = 'R';
			} else {
				char dummy[1];
				int retval;
				while (-EAGAIN == (retval = i2c_read_byte(dev, addr, dummy, 0))) {
					/* retry until bus arbitration succeeds */
				}

				switch (retval) {
				case 0:
					/* success: Device did respond */
					str[1] = 'X';
					break;
				case -ENXIO:
					/* No ACK --> no device */
					break;
				default:
					/* Some unexpected error */
					str[1] = 'E';
					break;
				}
			}

			fputs(str, stdout);
			addr++;
		}
		puts("");
	}

	i2c_release(dev);
	return 0;
}

typedef struct {
	// TODO add int dev
	uint8_t addr;
	char *description;
} i2c_dev_info_t;

#if defined(BOARD_THINGSAT_UP4_FM) || defined(BOARD_THINGSAT_UP4_V2) || defined(BOARD_THINGSAT_UP4)

static const i2c_dev_info_t known_i2c_dev[] = {
    {STTS751_TOP_LEFT_I2C_ADDR,	    "Top left temperature sensor (STTS751)"},
    {STTS751_BOTTOM_LEFT_I2C_ADDR,  "Bottom left temperature sensor (STTS751)"},
    {STTS751_BOTTOM_RIGHT_I2C_ADDR, "Bottom right temperature sensor (STTS751)"},
    {STTS751_CORECELL_I2C_ADDR,		"Corecell temperature sensor (STTS751)"},

	{A3G4250D_SA0_I2C_ADDR,			"Gyroscope (A3G4250D)"},

    {LSM303AGRTR_ACC_I2C_ADDR_READ,	"Accelerometer (LSM303AGRTR) Read"},
    //{LSM303AGRTR_ACC_I2C_ADDR_WRITE,"Accelerometer (LSM303AGRTR) Write"},
    {LSM303AGRTR_MAG_I2C_ADDR_READ,	"Magnetometer (LSM303AGRTR) Read"},
    //{LSM303AGRTR_MAG_I2C_ADDR_WRITE,"Magnetometer (LSM303AGRTR) Write"},

#if defined(BOARD_THINGSAT_UP4_FM)
    {M24M01E_I2C_ADDRESS,			"1Mbit EEPROM Memory (M24M01E)"},
    //{M24C01_I2C_ADDRESS,			"1Mbit EEPROM Memory (M24C01)"},
#endif
};

#elif defined(BOARD_NUCLEO_L432KC_INISAT)
static const i2c_dev_info_t known_i2c_dev[] = {
    {MCP9808_I2C_ADDRESS,	    "Temperature sensor (MCP9808)"},
};

#else

static const i2c_dev_info_t known_i2c_dev[] = {
		{0, "void"}
};

#endif

static void get_i2c_dev_name(uint8_t addr, char *string) {
	for (unsigned int i = 0; i < ARRAY_SIZE(known_i2c_dev); i++) {
		if (known_i2c_dev[i].addr == addr) {
			strcpy(string, known_i2c_dev[i].description);
			return;
		}
	}
	sprintf(string, "Unknown device");
}

int mission_i2c_scan_and_check(const int dev) {
	if (ARRAY_SIZE(known_i2c_dev) == 0) {
		printf("\nWARNING: known_i2c_dev is not defined for this board\n");
		return 0;
	}

	if (dev < 0 || (unsigned int) dev >= I2C_NUMOF) {
		printf("ERROR: Invalid I2C_DEV device specified (%u).\n", dev);
		return 1;
	}

	i2c_t i2c_dev = I2C_DEV(dev);

	i2c_init(i2c_dev);

	i2c_acquire(i2c_dev);

	printf("INFO: Scanning adresses from 0x00 to 0x7F ...\n");

	int nb_dev = 0;
	char description[64];

	for (unsigned int addr = 0; addr < 127; addr++) {

		// TODO show missing devices
		// TODO show unknown devices
		if (i2c_read_reg(i2c_dev, addr, 0, NULL, 0) >= 0) {
			get_i2c_dev_name(addr, description);

			printf("INFO:  - i2c device found at address: 0x%x [%s]\n", addr,
					description);
			nb_dev++;
		}
	}

	i2c_release(i2c_dev);

	printf("\nINFO: %d (/%d) devices found on I2C_DEV(%d)\n", nb_dev, ARRAY_SIZE(known_i2c_dev), dev);

	return 0;
}
