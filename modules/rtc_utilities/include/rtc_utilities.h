/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

#ifndef _INCLUDE_RTC_UTILITIES_H
#define _INCLUDE_RTC_UTILITIES_H

#include <stdint.h>
#include <time.h>
/**
 * @brief Print the time
 *
 * @param label the label prefixing the time
 * @param time the time
 */
void print_time(const char *label, const struct tm *time);

/**
 * @brief Print the RTC time
 */
void print_rtc(void);

/**
 * @brief  Get the RTC time in seconds since 1/1/1970 (Linux time)
 */
unsigned int get_time_since_epoch(void);

/**
 * @brief  Get the RTC time in seconds since GPS epoch 6/1/1980 (Linux time)
 */
unsigned int get_time_since_gps_epoch(void);

/**
 * @brief Set the RTC time
 *
 * @param timeToSet the time in seconds since 1/1/1970 (Linux start time)
 */
void set_rtc(unsigned int timeToSet);


/**
 * @brief Set the RTC time
 *
 * @param seconds_since_epoch the time in seconds since 1/1/1970 (Linux start time)
 */
void set_time_since_epoch(time_t seconds_since_epoch);

/**
 * @brief Set the RTC time
 *
 * @param seconds_since_gps_epoch the time in seconds since 6/1/1980 (GOS start time)
 */
void set_time_since_gps_epoch(time_t seconds_since_gps_epoch);



double difftimespec(struct timespec end, struct timespec beginning);


#endif
