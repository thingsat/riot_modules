#include "AioP13.h"

#ifndef SCREEN
// Wio Terminal Resolution	320 x 240 Display Size	2.4 inch
// https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/
#define SCREEN_WioTerminal		1
#endif

#ifndef SATELITTE
#define SATELITTE_STORK_1		1
#endif

#ifdef SCREEN_WioTerminal
#define MAP_MAXX   320
#define MAP_MAXY   240
#endif

#ifndef OBSERVER
#define OBSERVER_CSUG		1
#endif

#ifdef SATELITTE_STORK_1
// https://celestrak.com/NORAD/elements/gp.php?CATNR=51087
#define TLE_NAME   		"STORK-1"
#define TLE_LINE1		"1 51087U 22002DH  22109.45450769  .00008886  00000+0  50878-3 0  9991"
#define TLE_LINE2		"2 51087  97.4971 177.4889 0011957 274.8199 207.1507 15.12799091 14480"
// Nominal downlink frequency
#define RXFREQ			867.500
// Nominal uplink frequency
#define TXFREQ			867.500
#endif

#ifdef SATELITTE_ISS_ZARYA
#define TLE_NAME   		"ISS (ZARYA)"
#define TLE_LINE1		"1 25544U 98067A   22137.94890917  .00009100  00000+0  17024-3 0  9998"
#define TLE_LINE2		"2 25544  51.6421 126.3391 0004953 129.4958 273.8802 15.49485493340497"
// Nominal downlink frequency
#define RXFREQ			867.500
// Nominal uplink frequency
#define TXFREQ			867.500
#endif

#ifdef SATELITTE_ESHAIL_2
// For testing purpose (geostationary, no motion)
#define TLE_NAME   		"ES'HAIL 2"
#define TLE_LINE1		"1 43700U 18090A   21320.51254296  .00000150  00000+0  00000+0 0  9998"
#define TLE_LINE2		"2 43700   0.0138 278.3980 0002418 337.0092  10.7288  1.00272495 10898"
// Nominal downlink frequency
#define RXFREQ			867.500
// Nominal uplink frequency
#define TXFREQ			867.500
#endif


#ifdef OBSERVER_CSUG
#define OBSERVER_NAME   	"CSUG"
#define OBSERVER_LAT		45.19272111567223
#define OBSERVER_LON		5.759947347645972
#define OBSERVER_ALT		220.0
#endif

const char *tleName = TLE_NAME;
const char *tlel1 = TLE_LINE1;
const char *tlel2 = TLE_LINE2;

char observer_name[256] = OBSERVER_NAME;    // Observer name
double observer_lat = OBSERVER_LAT;  		// Latitude
double observer_lon = OBSERVER_LON;  		// Longitude
double observer_alt = OBSERVER_ALT;  		// Altitude ASL (m)

double dfreqRX = RXFREQ;     // Nominal downlink frequency
double dfreqTX = TXFREQ;     // Nominal uplink frequency


// Expecting the ISS to be at 289,61° elevation and 20,12° azimuth (Gpredict)
// Result for ESP32 will be 289,74° elevation and 20,44° azimuth.
// Result for UNO will be 289,70° elevation and 20,75° azimuth.
// Expecting the sun to be at -60.79° elevation and 0.86° azimuth (https://www.sunearthtools.com/dp/tools/pos_sun.php)
// Result for ESP32 will be -60.79° elevation and 0.89° azimuth.
// Result for UNO will be -60.79° elevation and 0.94° azimuth.

double dSatLAT = 0;           // Satellite latitude
double dSatLON = 0;           // Satellite longitude
double dSatAZ = 0;           // Satellite azimuth
double dSatEL = 0;           // Satellite elevation

double dSunLAT = 0;           // Sun latitude
double dSunLON = 0;           // Sun longitude
double dSunAZ = 0;           // Sun azimuth
double dSunEL = 0;           // Sun elevation

int ixQTH = 0;           // Map pixel coordinate x of QTH
int iyQTH = 0;           // Map pixel coordinate y of QTH
int ixSAT = 0;           // Map pixel coordinate x of satellite
int iySAT = 0;           // Map pixel coordinate y of satellite
int ixSUN = 0;           // Map pixel coordinate x of sun
int iySUN = 0;           // Map pixel coordinate y of sun

char acBuffer[P13DateTime::ascii_str_len + 1]; // Buffer for ASCII time

int aiSatFP[32][2]; // Array for storing the satellite footprint map coordinates
int aiSunFP[32][2];  // Array for storing the sunlight footprint map coordinates

void predict_tle(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond) {

	uint16_t i;

	P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
	P13Observer MyQTH(observer_name, observer_lat, observer_lon, observer_alt); // Set observer coordinates

	P13Satellite MySAT(tleName, tlel1, tlel2);       // Create ISS data from TLE

	latlon2xy(ixQTH, iyQTH, observer_lat, observer_lon, MAP_MAXX, MAP_MAXY); // Get x/y for the pixel map

	printf(
			"\r\nPrediction for %s at %s (MAP %dx%d: x = %d,y = %d):\r\n\r\n",
			MySAT.c_ccSatName, MyQTH.c_ccObsName, MAP_MAXX, MAP_MAXY, ixQTH, iyQTH);

	MyTime.ascii(acBuffer);           // Get time for prediction as ASCII string
	MySAT.predict(MyTime);              // Predict ISS for specific time
	MySAT.latlon(dSatLAT, dSatLON);     // Get the rectangular coordinates
	MySAT.elaz(MyQTH, dSatEL, dSatAZ);  // Get azimut and elevation for MyQTH

	latlon2xy(ixSAT, iySAT, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY); // Get x/y for the pixel map

	printf(
			"%s -> Lat: %.6f Lon: %.6f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n\r\n",
			acBuffer, dSatLAT, dSatLON, MAP_MAXX, MAP_MAXY, ixSAT, iySAT,
			dSatAZ, dSatEL);
	printf("RX: %.6f MHz, TX: %.6f MHz\r\n\r\n",
			MySAT.doppler(dfreqRX, P13_FRX), MySAT.doppler(dfreqTX, P13_FTX));

	// Calcualte footprint
	printf("Satellite footprint map coordinates:\n\r");

	MySAT.footprint(aiSatFP, (sizeof(aiSatFP) / sizeof(int) / 2), MAP_MAXX,
			MAP_MAXY, dSatLAT, dSatLON);

	for (i = 0; i < (sizeof(aiSatFP) / sizeof(int) / 2); i++) {
		printf("%2d: x = %d, y = %d\r\n", i, aiSatFP[i][0], aiSatFP[i][1]);
	}
}

void predict_sun(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond) {

	uint16_t i;

	P13Sun Sun;                                     // Create object for the sun
	P13DateTime MyTime(iYear, iMonth, iDay, iHour, iMinute, iSecond); // Set start time for the prediction
	P13Observer MyQTH(observer_name, observer_lat, observer_lon, observer_alt); // Set observer coordinates

	latlon2xy(ixQTH, iyQTH, observer_lat, observer_lon, MAP_MAXX, MAP_MAXY); // Get x/y for the pixel map

	// Predict sun
	Sun.predict(MyTime);                // Predict ISS for specific time
	Sun.latlon(dSunLAT, dSunLON);       // Get the rectangular coordinates
	Sun.elaz(MyQTH, dSunEL, dSunAZ);    // Get azimut and elevation for MyQTH

	latlon2xy(ixSUN, iySUN, dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY);

	printf(
			"\r\nSun -> Lat: %.6f Lon: %.6f (MAP %dx%d: x = %d,y = %d) Az: %.2f El: %.2f\r\n\r\n",
			dSunLAT, dSunLON, MAP_MAXX, MAP_MAXY, ixSUN, iySUN, dSunAZ, dSunEL);

	// Calcualte sunlight footprint
	printf("Sunlight footprint map coordinates:\n\r");

	Sun.footprint(aiSunFP, (sizeof(aiSunFP) / sizeof(int) / 2), MAP_MAXX,
			MAP_MAXY, dSunLAT, dSunLON);

	for (i = 0; i < (sizeof(aiSunFP) / sizeof(int) / 2); i++) {
		printf("%2d: x = %d, y = %d\r\n", i, aiSunFP[i][0], aiSunFP[i][1]);
	}

}
