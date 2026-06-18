/*
 Thingsat Mission
 Copyright (c) 2021-2026 Université Grenoble Alpes CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes CSUG LIG
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tle_utils.h"

#ifdef MODULE_AIOP13

#include "aiop13_wrapper.h"

static P13Predict_t instance;

#define TM_YEAR_OFFSET      (1900)

#endif


/**
 * @brief Init TLE from a file buffer
 */
bool init_tle_from_buffer(const char* buffer, const size_t buffer_size) {
	(void)buffer;
	(void)buffer_size;
#ifdef MODULE_AIOP13

	// TODO check compatible size with TLE
	const char* tle_name = buffer;
	char* ptr;
	ptr = strchr(buffer, '\n');
	if (ptr == NULL)
	{
		printf("ERROR malformed TLE\n");
		return false;
	}
	*ptr = '\0';
	// check buffer size (index = buffer - str)
	const char* tle_line1 = ptr;

	ptr = strchr(tle_line1, '\n');
	if (ptr == NULL)
	{
		printf("ERROR malformed TLE\n");
		return false;
	}
	*ptr = '\0';
	// check buffer size (index = buffer - str)
	const char* tle_line2 = ptr;

	ptr = strchr(tle_line2, '\n');
	if (ptr != NULL)
	{
		// remove extra newline
		*ptr = '\0';
	}

	instance = P13Predict_new(tle_name, tle_line1, tle_line2);
#endif
	return true;
}

/**
 * @brief Init TLE
 */
void init_tle(const char* tle_name, const char* tle_line1, const char* tle_line2) {
#ifdef MODULE_AIOP13

    if(instance == NULL) {
    	instance = P13Predict_new(tle_name, tle_line1, tle_line2);
    }
#endif
}


/**
 * @brief Predict the GPS at a timestamp from TLE
 */
void predict_gps_from_tle_at_epoch(time_t secondsSinceEpoch, double* p_dlat, double* p_dlon) {

#ifdef MODULE_AIOP13

	struct tm epoch_time;
	memcpy(&epoch_time, gmtime(&secondsSinceEpoch), sizeof(struct tm));

	P13Predict_predict(instance,
			epoch_time.tm_year + TM_YEAR_OFFSET, epoch_time.tm_mon + 1, epoch_time.tm_mday,
			epoch_time.tm_hour, epoch_time.tm_min, epoch_time.tm_sec
	);

	P13Predict_latlon(instance, p_dlat, p_dlon);
#endif
}



