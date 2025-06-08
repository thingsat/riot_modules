/*
 Thingsat project

 GPS over UART
 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 *
 * From "Manual test application for UART peripheral drivers", Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "board.h"
#include "shell.h"
#include "thread.h"
#include "msg.h"
#include "tsrb.h"
#include "periph/uart.h"
#include "xtimer.h"
#include "ztimer.h"

#include "parse_nmea.h"

#ifdef MODULE_STDIO_UART
#include "stdio_uart.h"
#endif

#ifndef SHELL_BUFSIZE
#define SHELL_BUFSIZE       (128U)
#endif
#ifndef UART_BUFSIZE
#define UART_BUFSIZE        (128U)
#endif

#define PRINTER_PRIO        (THREAD_PRIORITY_MAIN - 1)
#define PRINTER_TYPE        (0xabcd)

#define POWEROFF_DELAY      (250U * US_PER_MS)      /* quarter of a second */

/* if stdio is not done via UART, allow to use the stdio UART for the test */
#ifndef MODULE_STDIO_UART
#undef STDIO_UART_DEV
#endif

#ifndef STDIO_UART_DEV
#define STDIO_UART_DEV      (UART_UNDEF)
#endif

#ifndef STX
#define STX 0x2
#endif

typedef struct {
	uint8_t rx_mem[UART_BUFSIZE];
    //ringbuffer_t rx_buf;
    tsrb_t rx_buf;
} uart_ctx_t;

static uart_ctx_t ctx[UART_NUMOF];

static kernel_pid_t printer_pid;
static char printer_stack[THREAD_STACKSIZE_MAIN / 2];

static bool test_mode;

static int parse_dev(char *arg)
{
    unsigned dev = atoi(arg);
    if (dev >= UART_NUMOF) {
        printf("ERROR: Invalid UART_DEV device specified (%u).\n", dev);
        return -1;
    }
    else if (UART_DEV(dev) == STDIO_UART_DEV) {
        printf("ERROR: The selected UART_DEV(%u) is used for the shell!\n", dev);
        return -2;
    }
    return dev;
}

#ifdef MODULE_PERIPH_UART_RXSTART_IRQ
static void rxs_cb(void *arg)
{
    ringbuffer_add_one(arg, STX);
}
#endif

bool stop_gps_uart = false;

static void rx_cb(void *arg, uint8_t c)
{
    uart_t dev = (uart_t)(uintptr_t)arg;

    if(tsrb_full(&ctx[dev].rx_buf)) {
    	// drop char
    	// printf("DROP\n");
    } else {
        tsrb_add_one(&ctx[dev].rx_buf, c);
    }

    if (!test_mode) {
        msg_t msg;
        msg.content.value = (uint32_t)dev;
        msg_send(&msg, printer_pid);
    }
}

static void *printer(void *arg)
{
    (void)arg;
    msg_t msg;
    msg_t msg_queue[8];
    msg_init_queue(msg_queue, 8);

    while (1) {
        msg_receive(&msg);
        uart_t dev = (uart_t)msg.content.value;
        while(!tsrb_empty(&(ctx[dev].rx_buf))) {
        	int c = tsrb_get_one(&(ctx[dev].rx_buf));
            if(c > 0) {
            	parse_nmea((uint8_t)c);
            }
        }
    }

    /* this should never be reached */
    return NULL;
}

static unsigned gps_dev = 0;

// ugly to reset the GPS UART (can not support several GNSS module)
void gps_power_off_on(void)
{
    printf("INFO: UARD_DEV(%i): uart_poweron() and uart_poweroff()  ->  ", gps_dev);
    uart_poweroff(UART_DEV(gps_dev));
    ztimer_sleep(ZTIMER_USEC, POWEROFF_DELAY);
    uart_poweron(UART_DEV(gps_dev));
}


static unsigned gps_dev;
static uint32_t gps_baudrate;

#ifdef GPS_POWER_PIN
static void _gnss_power_init(void){
	gpio_init(GPS_POWER_PIN, GPIO_OUT);
	gpio_set(GPS_POWER_PIN);
}

static void _gnss_power_on(void){
	gpio_set(GPS_POWER_PIN);
	ztimer_sleep(ZTIMER_USEC,1000);
}

static void _gnss_power_off(void){
	gpio_clear(GPS_POWER_PIN);
	ztimer_sleep(ZTIMER_USEC,1000);
}
#endif


#ifdef GPS_STANDBY_PIN
static void _gnss_standby_init(void){
	gpio_init(GPS_STANDBY_PIN, GPIO_OUT);
	gpio_clear(GPS_STANDBY_PIN);
}

static void _gnss_standby(void){
	gpio_set(GPS_STANDBY_PIN);
	ztimer_sleep(ZTIMER_USEC,1000);
}

static void _gnss_wakeup(void){
	gpio_clear(GPS_STANDBY_PIN);
	ztimer_sleep(ZTIMER_USEC,1000);
}
#endif


#ifdef GPS_RESET_PIN
static void _gnss_reset_init(void){
	gpio_init(GPS_RESET_PIN, GPIO_OUT);
	gpio_set(GPS_RESET_PIN);
}

static void _gnss_reset(void){
	gpio_clear(GPS_RESET_PIN);
	ztimer_sleep(ZTIMER_MSEC,1000);
	gpio_set(GPS_RESET_PIN);
}

#endif

#ifdef GPS_PPS_INT_PIN
static uint32_t _gnss_pss_cnt = 1;

static void _gnss_pps_int_handle(void *arg){
	(void)arg;
	printf("\nINFO: GPS PPS Signal #%ld\n", _gnss_pss_cnt++);
}

static void _gnss_pps_int_init(void){
#if GPS_UART_ENABLE_TRACE == 1
    printf("INFO: Initialized PPS GPIO\n");
#endif
	gpio_init_int(GPS_PPS_INT_PIN, GPIO_IN_PU, GPIO_FALLING, _gnss_pps_int_handle, NULL);
	// GPIO_RISING
}
#endif


int gps_start(const unsigned dev, const uint32_t baudrate)
{
#ifdef GPS_POWER_PIN
    printf("INFO: GPS Init power\n");
	_gnss_power_init();
#endif
#ifdef GPS_STANDBY_PIN
    printf("INFO: GPS Init standby\n");
	_gnss_standby_init();
#endif
#ifdef GPS_RESET_PIN
    printf("INFO: GPS Init reset\n");
	_gnss_reset_init();
	_gnss_reset();
#endif
#ifdef GPS_PPS_INT_PIN
    printf("INFO: GPS Init PPS\n");
	_gnss_pps_int_init();
#endif


    /* initialize thread-safe ring buffers */
    for (unsigned i = 0; i < UART_NUMOF; i++) {
        tsrb_init(&(ctx[i].rx_buf), ctx[i].rx_mem, UART_BUFSIZE);
    }

    /* start the printer thread */
    printer_pid = thread_create(printer_stack, sizeof(printer_stack),
                                PRINTER_PRIO, 0, printer, NULL, "parse_nmea");

    /* initialize UART */
    int res = uart_init(UART_DEV(dev), baudrate, rx_cb, (void *)dev);
    if (res == UART_NOBAUD) {
        printf("ERROR: Given baudrate (%u) not possible\n", (unsigned int)baudrate);
        return 1;
    }
    else if (res != UART_OK) {
        puts("ERROR: Unable to initialize UART device for GPS");
        return 1;
    }
#if GPS_UART_ENABLE_TRACE == 1
    printf("INFO: Initialized UART_DEV(%i) at BAUD %"PRIu32"\n", dev, baudrate);
#endif

	gps_dev = dev;
	gps_baudrate = baudrate;

    return 0;
}

int gps_restart(void)
{
    printf("INFO: Restart UART_DEV(%i) at BAUD %"PRIu32"\n", gps_dev, gps_baudrate);

    tsrb_clear(&(ctx[gps_dev].rx_buf));

    /* initialize UART */
    gps_power_off_on();

#ifdef GPS_POWER_PIN
    printf("INFO: GPS Power off\n");
	_gnss_power_off();
    printf("INFO: GPS Power on\n");
	_gnss_power_on();
#endif


#ifdef GPS_RESET_PIN
    printf("INFO: GPS Reset\n");
	_gnss_reset();
#endif

#ifdef GPS_STANDBY_PIN
    printf("INFO: GPS Standby\n");
	_gnss_standby();
    printf("INFO: GPS Wakeup\n");
	_gnss_wakeup();
#endif

   return 0;
}

static void _gps_cmd_usage(int argc, char **argv) {
	(void)argc;
    printf("usage: %s init <dev> <baudrate>\n", argv[0]);
    printf("usage: %s print\n", argv[0]);
#ifdef GPS_RESET_PIN
    printf("usage: %s reset\n", argv[0]);
#endif
#ifdef GPS_POWER_PIN
    printf("usage: %s poweron\n", argv[0]);
    printf("usage: %s poweroff\n", argv[0]);
#endif
#ifdef GPS_STANDBY_PIN
    printf("usage: %s standby\n", argv[0]);
    printf("usage: %s wakeup\n", argv[0]);
#endif

}

int gps_cmd(int argc, char **argv) {
    if(argc == 2 && (strcmp(argv[1],"print")==0)) {
    	gps_print();
    	return 0;
#ifdef GPS_RESET_PIN
    } else if(argc == 2 && (strcmp(argv[1],"reset")==0)) {
    	_gnss_reset();
    	return 0;
#endif
#ifdef GPS_POWER_PIN
    } else if(argc == 2 && (strcmp(argv[1],"poweron")==0)) {
    	_gnss_power_on();
        return 0;
    } else if(argc == 2 && (strcmp(argv[1],"poweroff")==0)) {
    	_gnss_power_off();
        return 0;
#endif
    } else if(argc == 4 && (strcmp(argv[1],"init")==0)) {
        const int dev = parse_dev(argv[2]);
        if (dev < 0) {
            return 1;
        }
        const uint32_t baud = strtol(argv[3], NULL, 0);

    	return gps_start(dev, baud);
    } else {
    	_gps_cmd_usage(argc, argv);
        return 1;
    }

}
