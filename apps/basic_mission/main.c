/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

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


#if ENABLE_WDT_ZTIMER == 1
#include "wdt_ztimer.h"
#endif

#include "shell.h"

#include "lgw_cmd.h"
#include "repeat.h"
#include "endpoints.h"
#include "mission.h"
#include "ztimer.h"

#if GPS_UART_ENABLE == 1
#include "gps_uart.h"
#endif

#include "git_utils.h"
#include "board.h"
#include "i2c_scan.h"

//#define NO_SHELL 1

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
static const shell_command_t shell_commands[] = { { "git", "Show git info",
		git_cmd }, { "lgw", "LoRa gateway commands", lgw_cmd },

#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
    { "temp", "Get the temperatures (Celsius)", sensors_get_all_temp },
#endif
		{ NULL, NULL, NULL } };
#endif


static void print_i2c(void) {
	// I2C scan
	for (int idx = 0; idx < (int) I2C_NUMOF; idx++) {
		(void) mission_i2c_scan(idx);
		(void) mission_i2c_scan_and_check(idx);
	}
}

int main(void) {


	puts("=========================================");
	puts("Thingsat :: Basic Mission");
	puts("Copyright (c) 2021-2025 GINP UGA CSUG LIG");
	puts("=========================================");

	puts("\nBOARD:        " RIOT_BOARD "\n");

#if ENABLE_WDT_ZTIMER == 1
	(void)start_wdt_ztimer();
#else
	puts("WARNING: No watchdog timer\n");
#endif


#if GPS_UART_ENABLE == 1
	puts("INFO: Starting the GNSS parsing ...");

	printf("GPS_UART_DEV: %d\n", GPS_UART_DEV);
	printf("GPS_BAUDRATE: %d\n", GPS_BAUDRATE);
	gps_start(GPS_UART_DEV,GPS_UART_BAUDRATE);
#else
	puts("WARN: No GNSS module");
#endif

#if MESHTASTIC_ENABLE == 1
	puts("INFO: Meshtastic EU868 gateway configuration");
#else
	puts("INFO: LoRaWAN EU868 gateway configuration");
#endif

#if MESHTASTIC_ENABLE == 1
	puts("INFO: Chirpstack mesh is enabled");
#endif

	print_git();

	print_i2c();

#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
	sensors_init_all();
#endif


#if NO_SHELL == 1
	puts("INFO: Starting the gateway ...");
#if defined(MODULE_STTS751) || defined(STTS751_CORECELL_I2C_ADDR)
	lgw_cmd(1, (char*[]){"temp"});
#endif
	lgw_cmd(2, (char*[] ) { "lgw", "start" });
	lgw_cmd(2, (char*[] ) { "lgw", "eui" });
	lgw_cmd(2, (char*[] ) { "lgw", "freq_plan" });

#ifndef ENDPOINT_DEVADDR
	puts("INFO: Set endpoint");
	set_endpoint();
#endif
	puts("INFO: Set callback function for mission");
	pkt_period_cb = mission_periodic_cb;

	puts("INFO: Repeating is on");
	basic_mission_repeat_enable(true);

	puts("INFO: Set filter on RX frames");
	basic_mission_filter(REPEAT_FILTER_DEVADDR_SUBNET,
			REPEAT_FILTER_DEVADDR_MASK);

	puts("INFO: Set filter on SNR threshold");
	basic_mission_snr_threshold(REPEAT_FILTER_SNR_THRESHOLD);

	puts("INFO: Starting listening ...");
	lgw_cmd(2, (char*[] ) { "lgw", "listen" });


#else
	/* start the shell */
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif
	return 0;
}
