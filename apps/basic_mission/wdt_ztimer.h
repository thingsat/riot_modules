/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */


#ifndef _WDT_TIMER_H
#define _WDT_TIMER_H

int start_wdt_ztimer(void);
#if WDT_HAS_STOP
int wdt_stop_cmd(int argc, char *argv[]);
#endif
int abort_cmd(int argc, char *argv[]);

#endif
