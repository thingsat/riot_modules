/*
 * Copyright (C) 2022 Université Grenoble Alpes
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test the AiOP13 library
 *
 * @author      Didier DONSEZ
 *
 * @}
 */

using namespace std;

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <shell.h>
#include "periph/rtc.h"

extern  void predict_tle(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond);
extern  void predict_sun(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond);

#define TM_YEAR_OFFSET      (1900)

static struct tm _time;

static void print_time(const char *label, const struct tm *time)
{

    printf("%s  %04d-%02d-%02d %02d:%02d:%02d\n", label,
            time->tm_year + TM_YEAR_OFFSET,
            time->tm_mon + 1,
            time->tm_mday,
            time->tm_hour,
            time->tm_min,
            time->tm_sec);
}


int tle_cmd(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	rtc_get_time(&_time);
	print_time("Current time: ",&_time);

    predict_tle(
    		_time.tm_year + TM_YEAR_OFFSET,
			_time.tm_mon + 1,
			_time.tm_mday,
			_time.tm_hour,
			_time.tm_min,
			_time.tm_sec
		);

	return 0;
}

int sun_cmd(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	rtc_get_time(&_time);
	print_time("Current time: ",&_time);

    predict_sun(
    		_time.tm_year + TM_YEAR_OFFSET,
			_time.tm_mon + 1,
			_time.tm_mday,
			_time.tm_hour,
			_time.tm_min,
			_time.tm_sec
		);

	return 0;
}


//extern char *observer_name;    	// Observer name
extern double observer_lat;  		// Latitude
extern double observer_lon;  		// Longitude
extern double observer_alt;  		// Altitude ASL (m)


int observer_cmd(int argc, char *argv[]) {
	(void) argc;
	(void) argv;

	printf("Observer");
	//printf(" %s", observer_name);

	printf(": lat: %.6f", observer_lat);
	printf(" lon: %.6f", observer_lon);
	printf(" alt: %f\n", observer_alt);

	return 0;
}


static const shell_command_t shell_commands[] = {
	    { "tle", "TLE command", tle_cmd },
	    { "sun", "Sun command", sun_cmd },
	    { "observer", "Observer command", observer_cmd },
	    { NULL, NULL, NULL }
};


extern int test_wrapper_aoip13(void);
extern int bench_wrapper_aoip13(void);

/* main */
int main()
{

	(void)test_wrapper_aoip13();
	(void)bench_wrapper_aoip13();

    printf("\n************ RIOT and TLE demo program ***********\n");
    printf("\n");

    /* run the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
