/*
 * ewss.c
 *
 *  Created on: Jun 9, 2025
 *      Author: donsez
 */

#if 0

#include <stdio.h>
#include <string.h>
#include <time.h>

void generate_aprs_ews(char *buffer, size_t bufsize,
                       const char *callsign,         // ex: "F0EWS-13"
                       const char *object_name,      // ex: "EWSALRT "
                       float lat_deg, float lon_deg, // en degrés décimaux
                       const char *alert_msg,        // ex: "Tsunami Alert!"
                       uint16_t radius_km,           // rayon affecté
                       uint16_t duration_min)        // durée d'effet
{
    // Conversion latitude
    char lat_aprs[10];
    char lon_aprs[11];

    char ns = (lat_deg >= 0) ? 'N' : 'S';
    char ew = (lon_deg >= 0) ? 'E' : 'W';

    lat_deg = fabs(lat_deg);
    lon_deg = fabs(lon_deg);

    int lat_deg_int = (int)lat_deg;
    float lat_min = (lat_deg - lat_deg_int) * 60.0;
    sprintf(lat_aprs, "%02d%05.2f%c", lat_deg_int, lat_min, ns);

    int lon_deg_int = (int)lon_deg;
    float lon_min = (lon_deg - lon_deg_int) * 60.0;
    sprintf(lon_aprs, "%03d%05.2f%c", lon_deg_int, lon_min, ew);

    // Heure UTC
    time_t now = time(NULL);
    struct tm *utc = gmtime(&now);
    char time_str[8];
    strftime(time_str, sizeof(time_str), "%H%M%Sz", utc);

    // Création du message APRS object
    snprintf(buffer, bufsize,
             "%s>APRS,TCPIP*:;%s*%s%s/%s-%s Radius:%dkm Dur:%dm",
             callsign,
             object_name,
             time_str,
             lat_aprs,
             lon_aprs,
             alert_msg,
             radius_km,
             duration_min);
}

char aprs_msg[200];

int main() {
    generate_aprs_ews(aprs_msg, sizeof(aprs_msg),
                      "F0EWS-13",      // Callsign
                      "EWSALRT ",      // Object name (9 chars max)
                      37.7749, -122.4194, // San Francisco
                      "Tsunami Alert!",   // Alerte
                      100,               // Rayon 100 km
                      120);              // Durée 120 min

    printf("Message APRS:\n%s\n", aprs_msg);
    return 0;
}

F0EWS-13>APRS,TCPIP*:;EWSALRT *142215z3746.49N/12225.16W-Tsunami Alert! Radius:100km Dur:120m


#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// AX.25 constants
#define AX25_FLAG     0x7E
#define AX25_CTRL_UI  0x03
#define AX25_PID_APRS 0xF0

// CRC-CCITT
uint16_t ax25_crc(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= ((uint16_t)data[i]) << 8;
        for (int j = 0; j < 8; j++)
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return ~crc;
}

// Encode AX.25 address field (callsign-SSID, shifted & padded)
void ax25_encode_addr(uint8_t *out, const char *callsign, uint8_t ssid, int last) {
    memset(out, ' ' << 1, 6);
    for (int i = 0; i < 6 && callsign[i]; i++)
        out[i] = callsign[i] << 1;
    out[6] = ((ssid & 0x0F) << 1) | 0x60;
    if (last) out[6] |= 0x01;  // Set end-of-address bit
}

// Generate APRS info field (text)
void aprs_info_field(char *out, float lat, float lon, const char *msg) {
    char lat_str[10], lon_str[11];
    char ns = (lat >= 0) ? 'N' : 'S';
    char ew = (lon >= 0) ? 'E' : 'W';

    lat = fabs(lat);
    lon = fabs(lon);

    int lat_deg = (int)lat;
    float lat_min = (lat - lat_deg) * 60;
    int lon_deg = (int)lon;
    float lon_min = (lon - lon_deg) * 60;

    sprintf(lat_str, "%02d%05.2f%c", lat_deg, lat_min, ns);
    sprintf(lon_str, "%03d%05.2f%c", lon_deg, lon_min, ew);

    sprintf(out, "!%s/%s-%s", lat_str, lon_str, msg);
}

// Generate AX.25 UI frame
size_t generate_ax25_frame(uint8_t *frame, const char *dest_call, uint8_t dest_ssid,
                           const char *src_call, uint8_t src_ssid,
                           float lat, float lon,
                           const char *ews_msg) {
    uint8_t addr[14];
    uint8_t info[128];

    ax25_encode_addr(&addr[0], dest_call, dest_ssid, 0);
    ax25_encode_addr(&addr[7], src_call, src_ssid, 1);

    aprs_info_field((char *)info, lat, lon, ews_msg);
    size_t info_len = strlen((char *)info);

    size_t i = 0;
    frame[i++] = AX25_FLAG;
    memcpy(&frame[i], addr, 14); i += 14;
    frame[i++] = AX25_CTRL_UI;
    frame[i++] = AX25_PID_APRS;
    memcpy(&frame[i], info, info_len); i += info_len;

    uint16_t crc = ax25_crc(&frame[1], i - 1);  // exclude initial flag
    frame[i++] = crc & 0xFF;
    frame[i++] = (crc >> 8) & 0xFF;
    frame[i++] = AX25_FLAG;

    return i;  // Total frame length
}


#include <stdio.h>

int main() {
    uint8_t frame[256];
    size_t len = generate_ax25_frame(frame,
        "APRS", 0,
        "F0EWS", 13,
        37.7749, -122.4194,
        "Tsunami Alert! Radius:100km Dur:2h");

    printf("AX.25 frame (%zu bytes):\n", len);
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", frame[i]);
    }
    printf("\n");
    return 0;
}

#endif
