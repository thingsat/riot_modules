#include <stdio.h>

#include "geofence.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

// Remark: lat2 should be greater than lat1
// Remark: lon2 should be greater than lon1

// REMARK: lon is before lat in GeoJSON, CSV and C file

struct Area {
	float lon1;
	float lat1;
	float lon2;
	float lat2;
};

typedef struct Area Area_t;

#include "geofence_rectangles.h"
 
/**
 * @brief Check if a location is into one of the areas
 *
 * @param lat the latitude to the point to check
 * @param lng the longitude to the point to check
 * @param areas the pointer to the area array
 * @param areas_len the length to the area array
 * @return the index of the area or -1 if the point is outside of the areas
 */
static int8_t check_location(float lat, float lon, const Area_t* areas, const uint8_t areas_len) {

	for(uint8_t i=0; i<areas_len; i++){
		const Area_t* a = areas + i;
		float lat1, lat2, lon1, lon2;
		lat1 = a->lat1;
		lat2 = a->lat2;		
		lon1 = a->lon1;
		lon2 = a->lon2;
		if(lat1>lat2) { lat1 = lat2; lat2 = a->lat1;}		
		if(lon1>lon2) { lon1 = lon2; lon2 = a->lon1;}		
		if((lat>=lat1 && lat<=lat2) && (lon>=lon1 && lon<=lon2)) {
			return i;
		}
	}
	return -1;
}

/**
 * @brief Check if a location is into one of the areas
 *
 * @param lat the latitude to the point to check
 * @param lng the longitude to the point to check
 * @return the index of the area or -1 if the point is outside of the areas
 */
int8_t check_geofence(float lat, float lon) {
	return check_location(lat, lon, geofence_retangles, NELEMS(geofence_retangles));
}

/**
 * @brief Print if a location is into one of the areas or not
 *
 * @param lat the latitude to the point to check
 * @param lng the longitude to the point to check
 * @return the index of the area or -1 if the point is outside of the areas
 */
int8_t print_geofence(float lat, float lng) {
	int8_t area_idx = check_geofence(lat,lng);
	if(area_idx == -1) {
	 	printf("INFO: %f,%f is not into a geofence area\n", lat, lng);
	} else {
	 	printf("WARNING: %f,%f is into the geofence area : %d\n", lat, lng, area_idx);
	}
	return area_idx;
}
