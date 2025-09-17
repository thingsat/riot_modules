/*
 SX1302/SX1303 LGW commands
 Copyright (c) 2021-2024 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
//#include "riotbuild.h"

#include "shell.h"

#include "lgw_cmd.h"
#include "loragw_hal.h"
#include "git_utils.h"
#include "board.h"
#include "i2c_scan.h"

#if GPS_UART_ENABLE == 1
#include "gps_uart.h"
#endif

#if SX1280 == 1
#include "sx1280_cmd.h"
#endif

#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)

#include "stts751.h"
#include "stts751_params.h"

#include "board.h"

// STTS751 temperature sensors
static const stts751_params_t stts751_params_up4[] =
{
	    {
	        .i2c_dev = STTS751_PARAM_I2C_DEV,
	        .i2c_addr = STTS751_CORECELL_I2C_ADDR,
	    },
#ifdef BOARD_THINGSAT_UP4
	    {
	        .i2c_dev = STTS751_PARAM_I2C_DEV,
	        .i2c_addr = STTS751_TOP_LEFT_I2C_ADDR,
	    },
	    {
	        .i2c_dev = STTS751_PARAM_I2C_DEV,
	        .i2c_addr = STTS751_BOTTOM_LEFT_I2C_ADDR,
	    },
	    {
	        .i2c_dev = STTS751_PARAM_I2C_DEV,
	        .i2c_addr = STTS751_BOTTOM_RIGHT_I2C_ADDR,
	    },
#endif
};

#ifdef BOARD_THINGSAT_UP4
static stts751_t stts751_top_left;
static stts751_t stts751_bottom_left;
static stts751_t stts751_botton_right;
#endif
static stts751_t stts751_corecell;

stts751_t* p_stts751_corecell = &stts751_corecell;


int sensors_init_all(void) {

    if (stts751_init(&stts751_corecell, &stts751_params_up4[0]) != 0) {
        puts("stts751_corecell: Initialization failed!");
        return 1;
    }
#ifdef BOARD_THINGSAT_UP4
    if (stts751_init(&stts751_top_left, &stts751_params_up4[1]) != 0) {
        puts("stts751_top_left: Initialization failed!");
        return 1;
    }
    if (stts751_init(&stts751_bottom_left, &stts751_params_up4[2]) != 0) {
        puts("stts751_bottom_left: Initialization failed!");
        return 1;
    }
    if (stts751_init(&stts751_botton_right, &stts751_params_up4[3]) != 0) {
        puts("stts751_botton_right: Initialization failed!");
        return 1;
    }
#endif
    puts("STTS751 initializations done");

    return 0;
}

int sensors_get_all_temp(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    double temp;
    temp = stts751_get_temperature(&stts751_corecell);
    printf("stts751_corecell: Temperature: %d.%d C\n", (uint8_t)temp, (uint8_t)(temp * 10) % 10);
#ifdef BOARD_THINGSAT_UP4
    temp = stts751_get_temperature(&stts751_top_left);
    printf("stts751_top_left: Temperature: %d.%d C\n", (uint8_t)temp, (uint8_t)(temp * 10) % 10);
    temp = stts751_get_temperature(&stts751_bottom_left);
    printf("stts751_bottom_left: Temperature: %d.%d C\n", (uint8_t)temp, (uint8_t)(temp * 10) % 10);
    temp = stts751_get_temperature(&stts751_botton_right);
    printf("stts751_botton_right: Temperature: %d.%d C\n", (uint8_t)temp, (uint8_t)(temp * 10) % 10);
#endif

    return 0;
}
#endif

#ifndef NO_SHELL
static const shell_command_t shell_commands[] = {
		{ "git", "Show git info", git_cmd },
		{ "lgw", "LoRa gateway commands", lgw_cmd },
#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
    { "temp", "Get the temperatures (Celsius)", sensors_get_all_temp },
#endif
#if ENABLE_GPS == 1
    { "gps", "GPS commands", gps_cmd },
#endif
#if SX1280 == 1
    { "sx1280", "LoRa sx1280 basic commands", sx1280_cmd },
#endif
	{ NULL, NULL, NULL }
};
#endif



#ifndef ENDPOINT_DEVADDR

#include "endpoints.h"

static int _set_endpoint(void) {

	if(!lgw_is_started()) {
		printf("ERROR: the gateway is not started\n");
		return EXIT_FAILURE;
	}

	uint64_t eui;
	int err = lgw_get_eui(&eui);
	if (err != 0) {
		printf("ERROR: failed to get SX130x EUI\n");
		return EXIT_FAILURE;
	}
	lgw_sx130x_endpoint = NULL;

	for(unsigned int i=0; i<ARRAY_SIZE(lgw_sx130x_endpoints); i++) {
		if(eui == lgw_sx130x_endpoints[i].deveui) {
			lgw_sx130x_endpoint = lgw_sx130x_endpoints + i;
		}
	}

	if(lgw_sx130x_endpoint == NULL) {
		printf("ERROR: no endpoint defined for SX130x EUI\n");
		return EXIT_FAILURE;
	} else {
		printf("INFO: endpoint with devaddr=%08lx defined for SX130x EUI\n", lgw_sx130x_endpoint->devaddr);
		return EXIT_SUCCESS;
	}
}

#endif


int main(void) {

	puts("=========================================");
	puts("SX1302/SX1303 Driver Test Application");
	puts("Copyright (c) 2021-2025 UGA CSUG LIG");
	puts("=========================================");


	puts("\nBOARD: " RIOT_BOARD "\n");
#if MESHTASTIC_ENABLE == 1
	puts("\nMESHTASTIC EU868 gateway configuration\n");
#else
	puts("\nLoRaWAN EU868 gateway configuration\n");
#endif

#ifdef NO_SHELL
	print_git();
#endif

	// I2C scan

	for (int idx = 0; idx < (int) I2C_NUMOF; idx++) {
		(void) mission_i2c_scan(idx);
		(void) mission_i2c_scan_and_check(idx);
	}

#if SX1280 == 1
	sx1280_init_and_reboot_on_failure();
#endif

#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
	sensors_init_all();
#endif

#ifdef NO_SHELL
	puts("Starting the gateway ...");
#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
	lgw_cmd(1, (char*[]){"temp"});
#endif
	lgw_cmd(2, (char*[]){"lgw","start"});
	lgw_cmd(2, (char*[]){"lgw","eui"});
	lgw_cmd(2, (char*[]){"lgw","freq_plan"});

#ifndef ENDPOINT_DEVADDR
	_set_endpoint();
#endif

	puts("Repeating is on");
	lgw_cmd(3, (char*[]){"lgw","repeat","on"});
#if MESHTASTIC_ENABLE == 1
	lgw_cmd(2, (char*[]){"lgw","filter"});
#else
	// Filter CampusIoT only
	puts("Set filter on RX frames");
	lgw_cmd(4, (char*[]){"lgw","filter","fc00ac00","fffffc00"});
	lgw_cmd(2, (char*[]){"lgw","filter"});
#endif
	puts("Set filter on SNR threshold");
	lgw_cmd(3, (char*[]){"lgw","snr_threshold", "5"});
	lgw_cmd(2, (char*[]){"lgw","snr_threshold"});

	puts("Starting listening ...");
	lgw_cmd(2, (char*[]){"lgw","listen"});

	if(lgw_sx130x_endpoint == NULL) {
		puts("ERROR: lgw_sx130x_endpoint is null : Can not start TX bench");
	} else {
		puts("Starting TX bench ...");
		// every 30 seconds
		lgw_cmd(11, (char*[]){"lgw","bench","100000","7","125","8","12","on","false","32","30000"});
	}
#else
	/* start the shell */
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif
	return 0;
}
