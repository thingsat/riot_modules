/*
 * Copyright (C) 2020-2026 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */

#if ZOI_ENABLE == 1

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "rtc_utilities.h"
#include "tle_utils.h"

#include "aiop13_wrapper.h"

#define TLE_NAME "STORK-1"
#define TLE_LINE1 "1 51087U 22002DH  22109.45450769  .00008886  00000+0  50878-3 0  9991"
#define TLE_LINE2 "2 51087  97.4971 177.4889 0011957 274.8199 207.1507 15.12799091 14480"

#include "geofence.h"

#include "zoi.h"

static double haversine(double lat1, double lon1, double lat2, double lon2);


static bool _is_init_tle = false;

static bool previous = false;
static double previous_dlat;
static double previous_dlon;

static const char *p_ccnm=TLE_NAME;
static const char *p_ccl1=TLE_LINE1;
static const char *p_ccl2=TLE_LINE2;


/**
 * Zone of Interets (ZoI)
 */

void zoi_init(void) {

	printf("INFO: TLE_NAME=%s\n", TLE_NAME);
	printf("INFO: TLE_LINE1=%s\n", TLE_LINE1);
	printf("INFO: TLE_LINE2=%s\n", TLE_LINE2);

	// Init the AIOP13 lib with the tle
	if (!_is_init_tle) {
		init_tle(p_ccnm, p_ccl1, p_ccl2);
#ifdef TLE_EPOCH_INIT
		printf("INFO: Set time to epoch %d\n", TLE_EPOCH_INIT);
		set_time_since_epoch(TLE_EPOCH_INIT);
#endif
		_is_init_tle = true;
	}

	zoi_exec();

}

void zoi_exec(void) {

	double dlat;
	double dlon;
	predict_gps_from_tle_at_epoch(get_time_since_epoch(), &dlat, &dlon);

	printf("INFO: ");
	print_rtc();
	printf("INFO: Position predicted from TLE (%s): lat=%0.9f, lon=%0.9f\n",
			TLE_NAME, dlat, dlon);

	(void) print_geofence((float)dlat, (float)dlon);


	if (previous)
		printf("INFO: Distance from last predict : %0.1f kms\n",
				haversine(dlat, dlon, previous_dlat, previous_dlon));
	previous = true;
	previous_dlat = dlat;
	previous_dlon = dlon;
}





#define M_PI 3.14159265358979323846
// Rayon moyen de la Terre en kilomètres
#define EARTH_RADIUS_KM 6371.0

// Fonction pour convertir des degrés en radians
static inline double degrees_to_radians(double degrees) {
	return degrees * M_PI / 180.0;
}

// Fonction de Haversine pour calculer la distance entre deux points
static double haversine(double lat1, double lon1, double lat2, double lon2) {
	// Convertir les latitudes et longitudes de degrés en radians
	double lat1_rad = degrees_to_radians(lat1);
	double lon1_rad = degrees_to_radians(lon1);
	double lat2_rad = degrees_to_radians(lat2);
	double lon2_rad = degrees_to_radians(lon2);

	// Calcul de la différence entre les latitudes et les longitudes
	double dlat = lat2_rad - lat1_rad;
	double dlon = lon2_rad - lon1_rad;

	// Formule de Haversine
	double a = sin(dlat / 2) * sin(dlat / 2)
			+ cos(lat1_rad) * cos(lat2_rad) * sin(dlon / 2) * sin(dlon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	// Distance en kilomètres
	return EARTH_RADIUS_KM * c;
}


#endif

