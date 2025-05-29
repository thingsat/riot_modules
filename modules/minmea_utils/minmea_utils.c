/*
 * Copyright (C) 2025 Université Grenoble Alpes
 */

/*
 * Utils for MINMEA
 *      Author: Didier Donsez
 *
 */

#include "minmea_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fmt.h"
#include "minmea.h"

static bool parse_nmea_gll(const char *sentence) {
	struct minmea_sentence_gll frame;
	int res = minmea_parse_gll(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GLL sentence\n");
		return false;
	} else {
		printf(" time=%d:%d:%d.%d", frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);

		print_str(" lat=");
		print_float(minmea_tocoord(&frame.latitude), 6);
		print_str(" lng=");
		print_float(minmea_tocoord(&frame.longitude), 6);

		printf(" status=%c mode=%c", frame.status, frame.mode);

		print_str("\nSUCCESS\n");
	}

	return true;
}

static bool parse_nmea_zda(const char *sentence) {
	struct minmea_sentence_zda frame;
	int res = minmea_parse_zda(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing ZDA sentence\n");
	} else {
		//printf("parsed UTC and local date/time data");
		printf(" date=%d-%d-%d %d:%d:%d.%d", frame.date.year, frame.date.month,
				frame.date.day, frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);
		printf(" offset=%d:%d", frame.hour_offset, frame.minute_offset);
		print_str("\nSUCCESS\n");
	}

	return true;
}

static bool parse_nmea_rmc(const char *sentence) {
	struct minmea_sentence_rmc frame;
	int res = minmea_parse_rmc(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing RMC sentence\n");
	} else {
		// printf("parsed Position and time: valid=%d", frame.valid);

		printf(" valid=%d", frame.valid);

		printf(" date=%d-%d-%d %d:%d:%d.%d", frame.date.year, frame.date.month,
				frame.date.day, frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);

		print_str(" lat=");
		print_float(minmea_tocoord(&frame.latitude), 6);
		print_str(" lng=");
		print_float(minmea_tocoord(&frame.longitude), 6);
		print_str(" speed=");
		print_float(minmea_tocoord(&frame.speed), 6);
		print_str(" course=");
		print_float(minmea_tocoord(&frame.course), 6);
		print_str(" variation=");
		print_float(minmea_tocoord(&frame.variation), 6);

		print_str("\nSUCCESS\n");
	}

	return true;
}

static bool parse_nmea_gbs(const char *sentence) {
	struct minmea_sentence_gbs frame;
	int res = minmea_parse_gbs(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GBS sentence\n");
	} else {

		printf(" time=%d:%d:%d.%d", frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);

		print_str(" err_latitude=");
		print_float(minmea_tocoord(&frame.err_latitude), 6);
		print_str(" err_longitude=");
		print_float(minmea_tocoord(&frame.err_longitude), 6);
		print_str(" err_altitude=");
		print_float(minmea_tocoord(&frame.err_altitude), 6);

		printf(" svid=%d", frame.svid);

		print_str(" prob=");
		print_float(minmea_tocoord(&frame.prob), 2);
		print_str(" bias=");
		print_float(minmea_tocoord(&frame.bias), 2);
		print_str(" stddev=");
		print_float(minmea_tocoord(&frame.stddev), 2);

		print_str("\nSUCCESS\n");
	}

	return true;
}

static bool parse_nmea_gga(const char *sentence) {
	struct minmea_sentence_gga frame;
	int res = minmea_parse_gga(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GGA sentence\n");
	} else {

		printf(" time=%d:%d:%d.%d", frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);

		print_str(" lat=");
		print_float(minmea_tocoord(&frame.latitude), 6);
		print_str(" lng=");
		print_float(minmea_tocoord(&frame.longitude), 6);

		printf(" fix_quality=%d", frame.fix_quality);
		printf(" satellites_tracked=%d", frame.satellites_tracked);


		print_str(" hdop=");
		print_float(minmea_tocoord(&frame.hdop), 6);
		print_str(" altitude=");
		print_float(minmea_tocoord(&frame.altitude), 6); printf("%c",frame.altitude_units);
		print_str(" height=");
		print_float(minmea_tocoord(&frame.height), 6); printf("%c",frame.height_units);
		print_str(" dgps_age=");
		print_float(minmea_tocoord(&frame.dgps_age), 6);

		print_str("\nSUCCESS\n");
	}

	return true;
}


static const char* get_minmea_faa_mode_str(const enum minmea_faa_mode mode) {
	switch(mode) {
	case MINMEA_FAA_MODE_AUTONOMOUS:
		return "autonomous";
	case MINMEA_FAA_MODE_DIFFERENTIAL:
		return "differential";
	case MINMEA_FAA_MODE_ESTIMATED:
		return "estimated";
	case MINMEA_FAA_MODE_SIMULATED:
		return "simulated";
	case MINMEA_FAA_MODE_NOT_VALID:
		return "not valid";
	case MINMEA_FAA_MODE_PRECISE:
		return "precise";
	default:
		return "unknown";
	}
}

static bool parse_nmea_vtg(const char *sentence) {
	struct minmea_sentence_vtg frame;
	int res = minmea_parse_vtg(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing VTG sentence\n");
	} else {
		print_str(" true_track_degrees=");
		print_float(minmea_tofloat(&frame.true_track_degrees), 2);
		print_str(" magnetic_track_degrees=");
		print_float(minmea_tofloat(&frame.magnetic_track_degrees), 2);
		print_str(" speed_knots=");
		print_float(minmea_tofloat(&frame.speed_knots), 1);
		print_str(" speed_kph=");
		print_float(minmea_tofloat(&frame.speed_kph), 1);

		print_str(" faa_mode=");
		print_str(get_minmea_faa_mode_str(frame.faa_mode));


		print_str("\nSUCCESS\n");
	}

	return true;
}


static const char* get_minmea_gsa_mode_str(const enum minmea_gsa_mode mode) {
	switch(mode) {
	case MINMEA_GPGSA_MODE_AUTO:
		return "auto";
	case MINMEA_GPGSA_MODE_FORCED:
		return "forced";
	default:
		return "unknown";
	}
}

static const char* get_minmea_gsa_fix_type_str(const enum minmea_gsa_fix_type type) {
	switch(type) {
	case MINMEA_GPGSA_FIX_NONE:
		return "none";
	case MINMEA_GPGSA_FIX_2D:
		return "2d";
	case MINMEA_GPGSA_FIX_3D:
		return "3d";
	default:
		return "unknown";
	}
}


static bool parse_nmea_gsa(const char *sentence) {
	struct minmea_sentence_gsa frame;
	int res = minmea_parse_gsa(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GSA sentence\n");
	} else {

		print_str(" mode=");
		print_str(get_minmea_gsa_mode_str(frame.mode));
		print_str(" fix_type=");
		print_str(get_minmea_gsa_fix_type_str(frame.fix_type));

		print_str(" pdop=");
		print_float(minmea_tofloat(&frame.pdop), 2);
		print_str(" hdop=");
		print_float(minmea_tofloat(&frame.hdop), 2);
		print_str(" vdop=");
		print_float(minmea_tofloat(&frame.vdop), 1);


		printf("sat=[ ");
		for(int s=0;s<11;s++) {
			printf("%d ",frame.sats[s]);
		}
		printf("]");


		print_str("\nSUCCESS\n");
	}

	return true;
}



static void print_minmea_sat_info(struct minmea_sat_info* info) {
	printf(" nr=%d elevation=%d azimuth=%d snr=%d",
			info->nr,
			info->elevation,
			info->azimuth,
			info->snr
	);
};


static bool parse_nmea_gsv(const char *sentence) {
	struct minmea_sentence_gsv frame;
	int res = minmea_parse_gsv(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GSV sentence\n");
	} else {

		printf(" total_msgs=%d msg_nr=%d total_sats=%d",
				frame.total_msgs,
				frame.msg_nr,
				frame.total_sats
		);

		printf("\n");
		for(int s=0;s<4;s++) {
			printf("\t%d:",s+1);
			print_minmea_sat_info(frame.sats+s);
			printf("\n");
		}

		print_str("\nSUCCESS\n");
	}

	return true;
}


static bool parse_nmea_gst(const char *sentence) {
	struct minmea_sentence_gst frame;
	int res = minmea_parse_gst(&frame, sentence);
	if (!res) {
		print_str("FAILURE: error parsing GST sentence\n");
	} else {
		printf(" time=%d:%d:%d.%d", frame.time.hours, frame.time.minutes,
				frame.time.seconds, frame.time.microseconds);

		print_str(" rms_deviation=");
		print_float(minmea_tofloat(&frame.rms_deviation), 2);
		print_str(" semi_major_deviation=");
		print_float(minmea_tofloat(&frame.semi_major_deviation), 2);
		print_str(" semi_minor_deviation=");
		print_float(minmea_tofloat(&frame.semi_minor_deviation), 2);

		print_str(" semi_major_orientation=");
		print_float(minmea_tofloat(&frame.semi_major_orientation), 2);


		print_str(" latitude_error_deviation=");
		print_float(minmea_tofloat(&frame.latitude_error_deviation), 2);
		print_str(" longitude_error_deviation=");
		print_float(minmea_tofloat(&frame.longitude_error_deviation), 2);
		print_str(" altitude_error_deviation=");
		print_float(minmea_tofloat(&frame.altitude_error_deviation), 2);

		print_str("\nSUCCESS\n");
	}

	return true;
}


#define NMEA_BUFFER_SIZE 256
static uint8_t nema_buffer[NMEA_BUFFER_SIZE];
static int nema_buffer_idx = 0;

bool minmea_parse_and_print(uint8_t c) {

	if (nema_buffer_idx == 0) {
		memset(nema_buffer, 0, NMEA_BUFFER_SIZE);
	}

	if (c == '\n') {
		bool res;
		printf("\n");

		if (minmea_check((const char*) nema_buffer, true) == false) {
			printf("BAD CHECKSUM: %s\n", nema_buffer);
			res = false;
		} else {
			char talker[3];
			if (minmea_talker_id(talker, (const char*) nema_buffer) == false) {
				printf("No TalkerId\n");
				res = false;
			} else {

				// Determine sentence identifier.
				enum minmea_sentence_id s = minmea_sentence_id(
						(const char*) nema_buffer, true);
				switch (s) {
				case MINMEA_SENTENCE_GLL: // $GPGLL Sentence (Position)
					printf("GLL: ");
					res = parse_nmea_gll((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_ZDA: // $GPZDA Sentence (UTC and local date/time data)
					printf("ZDA: ");
					res = parse_nmea_zda((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_RMC: // $GPRMC Sentence (Position and time)
					printf("RMC: ");
					res = parse_nmea_rmc((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_GBS: // $GPGBS Sentence (???)
					printf("GBS: ");
					res = parse_nmea_gbs((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_GGA: // $GPGGA Sentence (Fix data)
					printf("GGA: ");
					res = parse_nmea_gga((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_GSA: // $GPGSA Sentence (Active satellites)
					printf("GSA: ");
					res = parse_nmea_gsa((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_GST: // $GPGST Sentence (pseudorange noise statistics)
					printf("GST: ");
					res = parse_nmea_gst((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_GSV: // $GPGSV Sentence (Satellites in view)
					printf("GSV: ");
					res = parse_nmea_gsv((const char*) nema_buffer);
					break;

				case MINMEA_SENTENCE_VTG: // $GPVTG Sentence (Course over ground)
					printf("VTG: ");
					res = parse_nmea_vtg((const char*) nema_buffer);
					break;

				case MINMEA_INVALID:
					printf("INVALID: %s\n", nema_buffer);
					res = false;
					break;
				case MINMEA_UNKNOWN:
					printf("UNKNOWN: %s\n", nema_buffer);
					res = false;
					break;
				default:
					res = false;
					break;
				}
			}
		}
		//reset the nema_buffer
		nema_buffer_idx = 0;
		return res;
	} else if (c == '\r') {
		// Skip : next char is \n
	} else {
		if (nema_buffer_idx < NMEA_BUFFER_SIZE - 1) {
			// last char is always \0 for avoid overflow
			nema_buffer[nema_buffer_idx] = c;
			nema_buffer_idx++;
		} else {
			printf("BUFFER OVERFLOW\n");
			return false;

		}
	}

	return true;
}

/*

TODO

static bool valid = false;
static bool date = ;
static float latitude  = ;
static float longitude = ;
static float altitude = ;
static int satellites = ;


bool minmea_get_postion() {

}
 */

