/*
 Thingsat project

 GPS over UART
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "mutex.h"
#include "ztimer.h"

#include "fmt.h"
#include "minmea.h"
#include "gps_uart.h"

#include "rtc_utilities.h"

// Mutex that protect GPS data.
static mutex_t gps_mutex = MUTEX_INIT;

static uint32_t last_fix_time = 0;

static bool parse_sentence_gll = true;
static bool parse_sentence_zda = true;
static bool parse_sentence_vtg = true;
static bool parse_sentence_gsv = true;
static bool parse_sentence_gbs = true;
static bool parse_sentence_rmc = true;
static bool parse_sentence_gga = true;
static bool parse_sentence_gsa = true;
static bool parse_sentence_gst = true;

static bool has_sentence_gll = false;
static struct minmea_sentence_gll sentence_gll;

static bool has_sentence_zda = false;
static struct minmea_sentence_zda sentence_zda;

static bool has_sentence_vtg = false;
static struct minmea_sentence_vtg sentence_vtg;

static bool has_sentence_gsv = false;
static struct minmea_sentence_gsv sentence_gsv;

static bool has_sentence_gbs = false;
static struct minmea_sentence_gbs sentence_gbs;

static bool has_sentence_rmc = false;
static struct minmea_sentence_rmc sentence_rmc;

static bool has_sentence_gga = false;
static struct minmea_sentence_gga sentence_gga;

static bool has_sentence_gsa = false;
static struct minmea_sentence_gsa sentence_gsa;

static bool has_sentence_gst = false;
static struct minmea_sentence_gst sentence_gst;

static uint32_t parse_nmea_global_badchecksum = 0;
static uint32_t parse_nmea_global_valid = 0;
static uint32_t parse_nmea_global_error = 0;
static uint32_t parse_nmea_global_unknown = 0;

inline static void parse_nmea_incr_global_badchecksum(void) {
	parse_nmea_global_badchecksum++;
}

inline static void parse_nmea_incr_global_valid(void) {
	parse_nmea_global_valid++;
}

inline static void parse_nmea_incr_global_error(void) {
	parse_nmea_global_error++;
}

inline static void parse_nmea_incr_global_unknown(void) {
	parse_nmea_global_unknown++;
}

uint16_t parse_nmea_get_global_error(void) {
	return (uint16_t) (parse_nmea_global_error && 0xFFFF);
}

uint16_t parse_nmea_get_global_unknown(void) {
	return (uint16_t) (parse_nmea_global_unknown && 0xFFFF);
}

static void print_minmea_time(const struct minmea_time *time) {
	print_str("time=");
	print_s32_dec(time->hours);
	print_str(":");
	print_s32_dec(time->minutes);
	print_str(":");
	print_s32_dec(time->seconds);
	print_str(".");
	print_s32_dec(time->microseconds); // TODO add padding in prefix 000012 msec, 100000 msec, 000001 msec
}

/*
 static void print_minmea_date(const struct minmea_date* date){
 print_str("date=");
 print_s32_dec(date->year);
 print_str("-");
 print_s32_dec(date->month); // Padding
 print_str("-");
 print_s32_dec(date->day); // Padding
 }
 */

static void print_minmea_sentence_gll(
		const struct minmea_sentence_gll *sentence_gll) {
	print_str("lat=");
	print_float(minmea_tocoord(&sentence_gll->latitude), 6);

	print_str(" lon=");
	print_float(minmea_tocoord(&sentence_gll->longitude), 6);

	print_str(" ");
	print_minmea_time(&sentence_gll->time);
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gll.html
 */
static bool parse_nmea_gll(const char *sentence) {
	int res = minmea_parse_gll(&sentence_gll, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GLL sentence\n");
		return false;
	} else {
		has_sentence_gll = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GLL (Position data: position fix, time of position fix, and status) sentence parsed: ");
		print_minmea_sentence_gll(&sentence_gll);
		print_str("\nSUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_vtg.html
 */
static bool parse_nmea_vtg(const char *sentence) {
	int res = minmea_parse_vtg(&sentence_vtg, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS VTG sentence\n");
		return false;
	} else {
		has_sentence_vtg = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS VTG (Track made good and speed over ground) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see
 */
static bool parse_nmea_zda(const char *sentence) {
	int res = minmea_parse_zda(&sentence_zda, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS ZDA sentence\n");
		return false;
	} else {
		has_sentence_zda = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS ZDA (UTC and local date/time data) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gsv.html
 */
static bool parse_nmea_gsv(const char *sentence) {
	int res = minmea_parse_gsv(&sentence_gsv, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GSV sentence\n");
		return false;
	} else {
		has_sentence_gsv = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GSV (Satellite information) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gbs.html
 */
static bool parse_nmea_gbs(const char *sentence) {
	int res = minmea_parse_gbs(&sentence_gbs, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GBS sentence\n");
	} else {
		has_sentence_gbs = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GBS (GNSS satellite fault detection (RAIM support))sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_rmc.html
 */
static bool parse_nmea_rmc(const char *sentence) {
	int res = minmea_parse_rmc(&sentence_rmc, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS RMC sentence\n");
		return false;
	} else {
		has_sentence_rmc = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS RMC (Position, velocity, and time) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gga.html
 */
static bool parse_nmea_gga(const char *sentence) {
	int res = minmea_parse_gga(&sentence_gga, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GGA sentence\n");
		return false;
	} else {
		has_sentence_gga = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GGA (Time, position, and fix related data) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gsa.html
 */
static bool parse_nmea_gsa(const char *sentence) {
	int res = minmea_parse_gsa(&sentence_gsa, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GSA sentence\n");
		return false;
	} else {
		has_sentence_gsa = true;

		if (sentence_gsa.fix_type != MINMEA_GPGSA_FIX_NONE) {
			last_fix_time = ztimer_now(ZTIMER_SEC);
		}

#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GSA (GPS DOP and active satellites) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
		return true;
	}
}

/**
 * @see https://receiverhelp.trimble.com/r750-gnss/nmea-0183messages_gst.html
 */
static bool parse_nmea_gst(const char *sentence) {
	int res = minmea_parse_gst(&sentence_gst, sentence);
	if (!res) {
		print_str("FAILURE: error parsing NMEA GNSS GST sentence\n");
		return false;
	} else {
		has_sentence_gst = true;
#if GPS_UART_ENABLE_TRACE == 1
		print_str("GNSS GST (Position error statistics) sentence parsed:");
		print_str(" SUCCESS\n");
#endif
	}
	return true;
}

#define NMEA_BUFFER_SIZE 256
static uint8_t nmea_buffer[NMEA_BUFFER_SIZE];
static int nmea_buffer_idx = 0;

bool gps_get_fix(void) {

	mutex_lock(&gps_mutex);

	if (!has_sentence_gsa) {
		mutex_unlock(&gps_mutex);
		return false;
	}

	if (sentence_gsa.fix_type == MINMEA_GPGSA_FIX_NONE) {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;
}

/**
 * Get the seconds since last fix.
 */
uint16_t gps_get_seconds_since_last_fix(void) {
	return (uint16_t) (ztimer_now(ZTIMER_SEC) - last_fix_time);
}

/**
 * Get GPS UTC date/time representation to a UNIX timestamp.
 */
bool gps_get_time(struct timespec *ts) {

	if (!gps_get_fix()) {
		return false;
	}

	mutex_lock(&gps_mutex);
	if (has_sentence_rmc) {
		minmea_gettime(ts, &sentence_rmc.date, &sentence_rmc.time);
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;

}

bool gps_get_position(float *latitude, float *longitude, float *altitude) {

	if (!gps_get_fix()) {
		return false;
	}

	mutex_lock(&gps_mutex);
	if (has_sentence_gll) {
		*latitude = minmea_tocoord(&sentence_gll.latitude);
		*longitude = minmea_tocoord(&sentence_gll.longitude);
	} else if (has_sentence_rmc) {
		*latitude = minmea_tocoord(&sentence_rmc.latitude);
		*longitude = minmea_tocoord(&sentence_rmc.longitude);
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	if (!has_sentence_gga) {
		mutex_unlock(&gps_mutex);
		return true;
	}

	*altitude = minmea_tofloat(&sentence_gga.altitude);

	mutex_unlock(&gps_mutex);
	return true;
}

bool gps_get_speed_direction(float *speed_kph, float *true_track_degrees) {

	if (!gps_get_fix()) {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_lock(&gps_mutex);
	if (has_sentence_vtg) {
		*speed_kph = minmea_tofloat(&sentence_vtg.speed_kph);
		*true_track_degrees = minmea_tofloat(&sentence_vtg.true_track_degrees);
	} else if (has_sentence_rmc) {
		*speed_kph = minmea_tofloat(&sentence_rmc.speed);
		*true_track_degrees = minmea_tofloat(&sentence_rmc.course);
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;
}
/**
 * Get Dilution of Precision
 * @see https://en.wikipedia.org/wiki/Dilution_of_precision_(navigation)
 */
bool gps_get_dop(float *pdop, float *hdop, float *vdop) {

	if (!gps_get_fix()) {
		return false;
	}

	mutex_lock(&gps_mutex);
	if (has_sentence_gsa) {
		*pdop = minmea_tofloat(&sentence_gsa.pdop);
		*hdop = minmea_tofloat(&sentence_gsa.hdop);
		*vdop = minmea_tofloat(&sentence_gsa.vdop);
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;
}

bool gps_get_quality(int *fix_quality, int *satellites_tracked) {

	if (!gps_get_fix()) {
		return false;
	}

	mutex_lock(&gps_mutex);
	if (has_sentence_gga) {
		*fix_quality = sentence_gga.fix_quality;
		*satellites_tracked = sentence_gga.satellites_tracked;
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;
}

/**
 * Set RTC with GPS UTC date/time.
 */
bool gps_set_rtc_time(void) {

	if (!gps_get_fix()) {
		return false;
	}
	mutex_lock(&gps_mutex);
	struct timespec ts;
	if (has_sentence_rmc) {
		minmea_gettime(&ts, &sentence_rmc.date, &sentence_rmc.time);
		set_time_since_epoch(ts.tv_sec);
		//set_time_since_gps_epoch(ts.tv_sec);

		// TODO add PPS and milliseconds
	} else {
		mutex_unlock(&gps_mutex);
		return false;
	}

	mutex_unlock(&gps_mutex);
	return true;
}

static uint32_t error_cnt = 0;
static uint32_t unkwown_cnt = 0;

void parse_nmea(uint8_t c) {

	if (nmea_buffer_idx == 0) {
		memset(nmea_buffer, 0, NMEA_BUFFER_SIZE);
	}

	if (c == '\n') {
//		printf("\n");

#if GPS_UART_ENABLE_TRACE == 1
    	printf("INFO: Process NMEA sentence: %s\n",(const char *)nmea_buffer);
#endif

		if (minmea_check((const char*) nmea_buffer, true) == false) {
			printf("ERROR: NMEA BAD CHECKSUM: %s\n", nmea_buffer);

			// TODO Try to recover by finding next sentence starting with $

			// clear buffer
			parse_nmea_incr_global_badchecksum();
			nmea_buffer_idx = 0;


		} else {
			char talker[3];
			if (minmea_talker_id(talker, (const char*) nmea_buffer) == false) {
				printf("ERROR: NMEA No TalkerId\n");
			} else {
				// Determine sentence identifier.
				enum minmea_sentence_id s = minmea_sentence_id(
						(const char*) nmea_buffer, true);

				mutex_lock(&gps_mutex);

				switch (s) {
				case MINMEA_SENTENCE_GLL: // $GPGLL Sentence (Position)
					if (!parse_sentence_gll)
						break;
					if (!parse_nmea_gll((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_GSV: // $GPGSV Sentence (Satellites in view)
					if (!parse_sentence_gsv)
						break;
					if (!parse_nmea_gsv((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_VTG: // $GPVTG Sentence (Course over ground)
					if (!parse_sentence_vtg)
						break;
					if (!parse_nmea_vtg((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_ZDA: // $GPZDA Sentence (UTC and local date/time data)
					if (!parse_sentence_zda)
						break;
					if (!parse_nmea_zda((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_GGA: // $GPGGA Sentence (Fix data)
					if (!parse_sentence_gga)
						break;
					if (!parse_nmea_gga((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_RMC: // $GPRMC Sentence (Position and time)
					if (!parse_sentence_rmc)
						break;
					if (!parse_nmea_rmc((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_GSA: // $GPGSA Sentence (Active satellites)
					if (!parse_sentence_gsa)
						break;
					if (!parse_nmea_gsa((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_GST: // $GPGST Sentence (pseudorange noise statistics)
					if (!parse_sentence_gst)
						break;
					if (!parse_nmea_gst((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;

				case MINMEA_SENTENCE_GBS: // $GPGBS Sentence (GNSS satellite fault detection (RAIM support))
					if (!parse_sentence_gbs)
						break;
					if (!parse_nmea_gbs((const char*) nmea_buffer)) {
						error_cnt++;
						parse_nmea_incr_global_error();
					} else {
						parse_nmea_incr_global_valid();
					}
					;
					break;
				case MINMEA_INVALID:
					printf(
							"ERROR: Can not parse NMEA Invalid sentence %s : %d.\n",
							(const char*) nmea_buffer, s);
					error_cnt++;
					parse_nmea_incr_global_error();
					break;
				case MINMEA_UNKNOWN:
					printf(
							"ERROR: Can not parse NMEA Unknown sentence %s : %d.\n",
							(const char*) nmea_buffer, s);
					unkwown_cnt++;
					parse_nmea_incr_global_unknown();
					break;
				default:
					printf("ERROR: Can not parse NMEA sentence %s : %d\n",
							(const char*) nmea_buffer, s);
					error_cnt++;
					parse_nmea_incr_global_error();
					break;
				}

				mutex_unlock(&gps_mutex);

				if (error_cnt > 5) {
					gps_restart();
					error_cnt = 0;
					nmea_buffer_idx = 0;
					return;
				}

				nmea_buffer_idx = 0;

			}
		}
	} else if (c == '\r') {
		// Skip : next char is \n
	} else {
		if (nmea_buffer_idx < NMEA_BUFFER_SIZE - 1) {
			// last char is always \0 for avoid overflow
			nmea_buffer[nmea_buffer_idx] = c;
			nmea_buffer_idx++;
		} else {
			printf("BUFFER OVERFLOW\n");
			// clear buffer
			nmea_buffer_idx = 0;
		}
	}
}

void gps_print(void) {

	const uint16_t seconds_since_last_fix = gps_get_seconds_since_last_fix();
	if (!gps_get_fix() && seconds_since_last_fix > 2) {
		printf("WARN: GNSS no fix\n");
	}

	printf("INFO: GNSS Stats valid=%lu badchecksum=%lu error=%lu unknown=%lu\n",
			parse_nmea_global_valid, parse_nmea_global_badchecksum,
			parse_nmea_global_error, parse_nmea_global_unknown);
	printf("INFO: GNSS Last fix time : %lu (%u seconds ago)\n", last_fix_time,
			seconds_since_last_fix);

	if (!gps_get_fix() && seconds_since_last_fix > 2) {
		return;
	}

	bool res;
	struct timespec ts;
	res = gps_get_time(&ts);
	if (!res) {
		printf("WARN: GNSS no time available\n");
	} else {
		char str[32];
		size_t s = fmt_u64_dec(str, ts.tv_sec);
		str[s] = '\0';
		//printf("INFO: GNSS time=%lld.%.9ld\n", (long long) ts.tv_sec,
		//		ts.tv_nsec);
		printf("INFO: GNSS time sec=%s nsec=%lu\n", str, ts.tv_nsec);

	}

	float latitude, longitude, altitude;
	res = gps_get_position(&latitude, &longitude, &altitude);
	if (!res) {
		printf("WARN: GNSS no position available\n");
	} else {
		printf("INFO: GNSS lat=%f°, lon=%f°, alt=%.0fm\n", latitude, longitude,
				altitude);
		printf("INFO: GNSS OSM=https://www.openstreetmap.org/#map=17/%f/%f\n",
				latitude, longitude);
		printf("INFO: GNSS GoogleMap=https://www.google.fr/maps/@%f,%f,17z\n",
				latitude, longitude);
	}

	float speed_kph;
	float true_track_degrees;
	res = gps_get_speed_direction(&speed_kph, &true_track_degrees);
	if (!res) {
		printf("WARN: GNSS no speed and track available\n");
	} else {
		printf("INFO: GNSS speed=%.1f m/s (%.1f km/h), track=%.0f°\n",
				speed_kph * 1.852 / 3.6, speed_kph * 1.852, true_track_degrees);
	}

	float pdop, hdop, vdop;
	res = gps_get_dop(&pdop, &hdop, &vdop);
	if (!res) {
		printf("WARN: GNSS no DOP available\n");
	} else {
		printf("INFO: GNSS pdop=%.2f, hdop=%.2f, vdop=%.2f\n", pdop, hdop,
				vdop);
	}

	int fix_quality, satellites_tracked;
	res = gps_get_quality(&fix_quality, &satellites_tracked);
	if (!res) {
		printf("WARN: GNSS no quality available\n");
	} else {
		printf("INFO: GNSS fix_quality=%d, satellites_tracked=%d\n",
				fix_quality, satellites_tracked);
	}
}
