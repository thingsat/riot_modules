/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ENABLE_DEBUG	0
#include "debug.h"

#include "ztimer.h"
#include "ztimer/periodic.h"

#include "fmt.h"
#include "shell.h"

#ifndef WDT_UTILS_KICK_PERIOD
#define WDT_UTILS_KICK_PERIOD		5000LU	// msec
#endif

#ifndef WDT_UTILS_TIMEOUT
#define WDT_UTILS_TIMEOUT			30000 	// msec
#endif

#include "periph/wdt.h"

static unsigned cpt = 0;

static const char* _param = "WDT";

static bool _wdt_kick_cb(void *arg)
{
	(void)arg;
	cpt++;
    DEBUG("\n[%s] KICK %s %d\n", __FUNCTION__, (char*)arg, cpt);
    wdt_kick();

    return false;
}

static ztimer_periodic_t _wdt_ztimer;

int start_wdt_ztimer(void) {

    ztimer_periodic_init(ZTIMER_MSEC, &_wdt_ztimer, _wdt_kick_cb, (void*)_param, WDT_UTILS_KICK_PERIOD);
    ztimer_periodic_start(&_wdt_ztimer);
	wdt_setup_reboot(0, WDT_UTILS_TIMEOUT);
	wdt_start();

    if (!ztimer_is_set(ZTIMER_MSEC, &_wdt_ztimer.timer)) {
    	printf("[%s] ERROR\n", __FUNCTION__);
        return -1;
    } else {
    	printf("[%s] WDT started\n", __FUNCTION__);
    }

	return 0;
}

// TODO int stop_wdt_timer(void);

#if WDT_HAS_STOP
int wdt_stop_cmd(int argc, char *argv[]) {
	(void) argc;
	(void) argv;
	puts("WDT stopped");
	wdt_stop();
	return 0;
}
#endif

int abort_cmd(int argc, char *argv[]) {
	(void) argc;
	(void) argv;
	puts("Abort now !");
	abort();
	return 0;
}

#if 0
static const shell_command_t shell_commands[] = {
	    { "abort", "Abort the program", abort_cmd },
#if WDT_HAS_STOP
	    { "wdt_stop", "Stop WDT", wdt_stop_cmd },
#endif
	    { NULL, NULL, NULL }
};
#endif
