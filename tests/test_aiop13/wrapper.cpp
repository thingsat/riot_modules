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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "ztimer.h"
#include "xtimer.h"
#include "aiop13_wrapper.h"


// https://celestrak.com/NORAD/elements/gp.php?CATNR=51087
#define TLE_NAME   		"STORK-1"
#define TLE_LINE1		"1 51087U 22002DH  22109.45450769  .00008886  00000+0  50878-3 0  9991"
#define TLE_LINE2		"2 51087  97.4971 177.4889 0011957 274.8199 207.1507 15.12799091 14480"


/* test_wrapper */
int test_wrapper_aoip13(void)
{

    printf("\n************ test wrapper for AoiP13 ***********\n");
    printf("\n");


    P13Predict_t instance = P13Predict_new(TLE_NAME, TLE_LINE1, TLE_LINE2);
    if(instance != NULL) {
		P13Predict_predict(instance, 2022, 6, 4, 17, 0, 0);

		double p_dlat;
		double p_dlon;
		P13Predict_latlon(instance, &p_dlat, &p_dlon);

		printf("GPS prediction : latitude=%lf°, longitude=%lf°\n", p_dlat, p_dlon);
    }
    return 0;
}
const static uint32_t BENCH_SIZE = 10000;

/* bench_wrapper */
int bench_wrapper_aoip13(void)
{
    printf("\n************ bench wrapper for AoiP13 ***********\n");
    printf("\n");

	P13Predict_t instance = P13Predict_new(TLE_NAME, TLE_LINE1, TLE_LINE2);
	if(instance != NULL) {

	    uint32_t start, stop;
		double p_dlat;
		double p_dlon;

		start = xtimer_now_usec();
		for (uint32_t i = 0; i < BENCH_SIZE; ++i) {
			P13Predict_predict(instance, 2022, 6, 4, 17, 0, 0);
			P13Predict_latlon(instance, &p_dlat, &p_dlon);
		}
	    stop = xtimer_now_usec();

		printf("%lu GPS predictions from TLE in %lu usec (%f usec per call)\n", BENCH_SIZE, stop-start, (stop-start)*1.0/BENCH_SIZE);

		return 0;

	} else {
	    return -1;
	}
}
