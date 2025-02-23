/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

#define ENABLE_DEBUG 		ENABLE_DEBUG_RTC
#include "debug.h"

#include <stdlib.h>

#include "xtimer.h"
#include <string.h>

#include "periph_conf.h"
#include "periph/rtc.h"

#include "rtc_utilities.h"


// 1972 and 1976 have 366 days (DELTA_EPOCH_GPS is 315964800 seconds)
// GPS Epoch consists of a count of weeks and seconds of the week since 0 hours (midnight) Sunday 6 January 1980
#define DELTA_EPOCH_GPS ((365*8 + 366*2 + 5)*(24*60*60))

#define TM_YEAR_OFFSET      (1900)

static time_t lastTimeCorrection = 0; // 01/01/1970

/**
 * @brief Print the time
 *
 * @param label the label prefixing the time
 * @param time the time
 */
void print_time(const char *label, const struct tm *time) {
	printf("%s  %04d-%02d-%02d %02d:%02d:%02d\n", label,
			time->tm_year + TM_YEAR_OFFSET, time->tm_mon + 1, time->tm_mday,
			time->tm_hour, time->tm_min, time->tm_sec);
}

/**
 * @brief Print the RTC time
 */
void print_rtc(void) {
	/* read RTC */
	struct tm current_time;
	rtc_get_time(&current_time);
	print_time("Current RTC time : ", &current_time);
	struct tm lastTimeCorrectionTime = *localtime(&lastTimeCorrection);
	if (lastTimeCorrection == 0) {
		DEBUG("Last correction  :   never\n");
	} else {
		print_time("Last correction  : ", &lastTimeCorrectionTime);
	}
}

/**
 * @brief Get the RTC time in seconds since 1/1/1970 (Linux time)
 */
unsigned int get_time_since_epoch(void) {
	struct tm current_time;
	// Read the RTC current time
	rtc_get_time(&current_time);
#if ENABLE_DEBUG == 1
	print_time("Current time: ", &current_time);
#endif
	time_t timeSinceEpoch = mktime(&current_time);
	return timeSinceEpoch;
}


/**
 * @brief  Get the RTC time in seconds since GPS epoch 6/1/1980 (GPS time)
 */
unsigned int get_time_since_gps_epoch(void) {
	return get_time_since_epoch()-DELTA_EPOCH_GPS;
}


/**
 * @brief Set the RTC time with time_t
 *
 * @param new_time the struct time
 */
void set_rtc_tm(struct tm *new_time) {

	struct tm current_time;
	// Read the RTC current time
	rtc_get_time(&current_time);
#if ENABLE_DEBUG == 1
	print_time("Current time    : ", &current_time);
#endif

#if ENABLE_DEBUG == 1
	int32_t rtc_drift = mktime(&current_time) - mktime(new_time);
	DEBUG("Drift in sec.   :  %ld\n", rtc_drift);
#endif

	rtc_set_time(new_time);
	lastTimeCorrection = mktime(new_time);
#if ENABLE_DEBUG == 1
	print_time("RTC time fixed  : ", new_time);
#endif
}

/**
 * @brief Set the RTC time
 *
 * @param seconds_since_epoch the time in seconds since 1/1/1970 (Linux start time)
 */
void set_time_since_epoch(time_t seconds_since_epoch) {
	struct tm epoch_time;
	memcpy(&epoch_time, gmtime(&seconds_since_epoch), sizeof (struct tm));
	rtc_set_time(&epoch_time);
}

/**
 * @brief Set the RTC time
 *
 * @param seconds_since_gps_epoch the time in seconds since 6/1/1980 (GOS start time)
 */
void set_time_since_gps_epoch(time_t seconds_since_gps_epoch) {
	seconds_since_gps_epoch += DELTA_EPOCH_GPS;
	struct tm epoch_time;
	memcpy(&epoch_time, gmtime(&seconds_since_gps_epoch), sizeof (struct tm));
	rtc_set_time(&epoch_time);
}


/**
 * @brief Set the RTC time with epoch
 *
 * @param timeToSet the time in seconds since 1/1/1970 (Linux start time)
 */
void set_rtc(unsigned int timeToSet) {
	time_t _timeToSet = timeToSet;
	struct tm new_time = *localtime(&_timeToSet);
	set_rtc_tm(&new_time);
}


/**
 * from lora_pkt_fwd.c line 1300
 */

double difftimespec(struct timespec end, struct timespec beginning) {
    double x;

    x = 1E-9 * (double)(end.tv_nsec - beginning.tv_nsec);
    x += (double)(end.tv_sec - beginning.tv_sec);

    return x;
}
