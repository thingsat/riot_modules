/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ENABLE_DEBUG		ENABLE_DEBUG_APRS
#include "debug.h"

#include "parse_nmea.h"
#include "fmt_utils.h"
#include "aprs.h"


// APRS (https://www.aprs.org/doc/APRS101.PDF) like text message.

// Helper pour convertir la latitude au format APRS ddmm.mmN
static void formatLatitude(float lat, uint8_t *out) {
    char hemi = (lat >= 0) ? 'N' : 'S';
    lat = fabs(lat);
    int deg = (int)lat;
    float min = (lat - deg) * 60.0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf((char*)out, 10, "%02d%05.2f%c", deg, min, hemi);
#pragma GCC diagnostic pop
}

// Helper pour convertir la longitude au format APRS dddmm.mmE
static void formatLongitude(float lon, uint8_t *out) {
    char hemi = (lon >= 0) ? 'E' : 'W';
    lon = fabs(lon);
    int deg = (int)lon;
    float min = (lon - deg) * 60.0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
   snprintf((char*)out, 11, "%03d%05.2f%c", deg, min, hemi);
#pragma GCC diagnostic pop
}

static const char format[] = "%s-11>APRS,TCPIP*:=%s/%sO%.3d/%.3d/A=%06d %s";

// Fonction pour construire une trame APRS
static void buildAPRSFrame(
    const char *callsign,       // Ex: "F1ABC"
    float latitude,
    float longitude,
    int course,                 // Cap en degrés (0-360)
    int speed,                  // Vitesse en noeuds
    float altitudeMeters,  	// Altitude en mètres
    const char *comment,        // Commentaire APRS à afficher
    uint8_t *out             // Résultat final
) {
	uint8_t latStr[10];
	uint8_t lonStr[11];

    formatLatitude(latitude, latStr);
    formatLongitude(longitude, lonStr);

    // Conversion altitude mètres → pieds (1 m = 3.28084 ft)
    int altitudeFeet = (int)(altitudeMeters * 3.28084);

    // Symbole ballon : '/O'
    // Course et vitesse sont formatés sur 3 chiffres

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf((char*)out, 256,
        format,  // Trame position APRS standard
        callsign,
        latStr,
        lonStr,
        course % 360, 	// in deg
        speed, // in kph
        altitudeFeet,
        comment
    );
#pragma GCC diagnostic pop
}

bool aprs_get_fpayload(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size

) {
	float latitude, longitude, altitude;
	gps_get_position(&latitude, &longitude, &altitude);

	float speed_kph;
	float true_track_degrees;

	gps_get_speed_direction(&speed_kph, &true_track_degrees);


	int fix_quality;
	int satellites_tracked;
	if(!gps_get_quality(&fix_quality, &satellites_tracked) || fix_quality == 0) {
		latitude = 0.0;
		longitude = 0.0;
		altitude = 0.0;
		speed_kph = 0.0;
		true_track_degrees = 0.0;
	}

    buildAPRSFrame(MISSION_APRS_CALLSIGN,
    		latitude, longitude, floor(true_track_degrees), floor(speed_kph), floor(altitude), MISSION_APRS_COMMENT, fpayload);

    DEBUG("INFO: %s : fpayload=%s\n", __FUNCTION__, fpayload);

	*fpayload_size = strlen((char*)fpayload);

	return true;
}


// CRC-16-CCITT (polynôme 0x8408) utilisé pour AX.25
static uint16_t ax25_crc(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0x8408;
            else crc >>= 1;
        }
    }
    return ~crc;
}

// Encode un callsign AX.25 (shifty + SSID)
static void encode_callsign(uint8_t *dest, const char *callsign, uint8_t ssid, int last) {
    int i;
    for (i = 0; i < 6; i++) {
        if (callsign[i])
            dest[i] = callsign[i] << 1;
        else
            dest[i] = ' ' << 1;
    }
    dest[6] = ((ssid & 0x0F) << 1) | 0x60;  // C bits = 1 1 0 0 0 0
    if (last)
        dest[6] |= 0x01; // Set bit 0 if it's the last address
}

// Construit la trame APRS binaire AX.25 avec commentaire et altitude
static size_t build_ax25_frame(
    const char *srcCall, uint8_t srcSSID,
    const char *dstCall, uint8_t dstSSID,
    float lat, float lon,
    int course, int speed,
    float altitudeMeters,
    const char *comment,
    uint8_t *frameOut
) {
    uint8_t *p = frameOut;

    // Encode adresse destination (APRS)
    encode_callsign(p, dstCall, dstSSID, 0); p += 7;

    // Encode adresse source
    encode_callsign(p, srcCall, srcSSID, 1); p += 7;

    // Control + PID
    *p++ = 0x03;  // UI frame
    *p++ = 0xF0;  // No layer 3

    // Construction de la trame APRS (ex: =lat/lonOcourse/speed/A=XXXXX comment)
    char latStr[10], lonStr[11], aprs[256];
    char hemiLat = (lat >= 0) ? 'N' : 'S';
    char hemiLon = (lon >= 0) ? 'E' : 'W';
    lat = fabs(lat);
    lon = fabs(lon);
    int degLat = (int)lat;
    float minLat = (lat - degLat) * 60.0;
    int degLon = (int)lon;
    float minLon = (lon - degLon) * 60.0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(latStr, sizeof(latStr), "%02d%05.2f%c", degLat, minLat, hemiLat);
    snprintf(lonStr, sizeof(lonStr), "%03d%05.2f%c", degLon, minLon, hemiLon);
#pragma GCC diagnostic pop
    int altitudeFeet = (int)(altitudeMeters * 3.28084);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(aprs, sizeof(aprs),
        "=%s/%sO%03d/%03d/A=%06d %s",
        latStr, lonStr,
        course % 360,
        speed,
        altitudeFeet,
        comment
    );
#pragma GCC diagnostic pop


    DEBUG("INFO: %s : aprs=%s\n", __FUNCTION__, aprs);

    size_t aprsLen = strlen(aprs);
    memcpy(p, aprs, aprsLen);
    p += aprsLen;

    // Ajouter FCS (CRC)
    uint16_t crc = ax25_crc(frameOut, p - frameOut);
    *p++ = crc & 0xFF;
    *p++ = (crc >> 8) & 0xFF;

#if ENABLE_DEBUG == 1
    DEBUG("INFO: %s : ax25=", __FUNCTION__);
    fmt_printf_ba(frameOut, p - frameOut, " ");
    DEBUG("\n");
#endif

    return p - frameOut; // Taille totale de la trame
}

bool aprs_get_ax25(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size
) {
    const char *srcCall = MISSION_APRS_CALLSIGN;
    const char *dstCall = "APRS";
    uint8_t srcSSID = 11;   // -11 pour ballon
    uint8_t dstSSID = 0;    // standard

	float latitude, longitude, altitude;
	gps_get_position(&latitude, &longitude, &altitude);

	float speed_kph;
	float true_track_degrees;

	gps_get_speed_direction(&speed_kph, &true_track_degrees);

	int fix_quality;
	int satellites_tracked;
	if(!gps_get_quality(&fix_quality, &satellites_tracked) || fix_quality == 0) {
		latitude = 0.0;
		longitude = 0.0;
		altitude = 0.0;
		speed_kph = 0.0;
		true_track_degrees = 0.0;
	}

    size_t len = build_ax25_frame(srcCall, srcSSID, dstCall, dstSSID,
    		latitude, longitude, floor(true_track_degrees), floor(speed_kph),
                                   altitude, MISSION_APRS_COMMENT, fpayload);
    *fpayload_size = len;

    return true;
}
