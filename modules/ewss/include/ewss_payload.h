/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */

/*
 * Thingsat Mission Scenarii :: Data structures
 */

/*
 * Types for the format of the payload into the LoRaWAN frames sent and received by the Thingsat payload
 */


#ifndef LORAWAN_EWSS_PAYLOAD_H
#define LORAWAN_EWSS_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>

#include "lorawan_service.h"

/*
 * FPort definition for typing the payload format
 */

// Emergency Warning Satellite Service (Galileo like)
#define FPORT_DN_EWSS_PAYLOAD					(17U)

/*
The Common Alert Message Format

@see https://www.gsc-europa.eu/sites/default/files/sites/all/files/EWSS-CAMF_v1.0.pdf

The Common Alert Message Format (CAMF) is based on 122 bits, to be encapsulated in the signals of
the satellite navigation systems. This size is driven by design constraints applying to some satellite
constellations, where only a limited number of bits is available for messaging. The CAMF has been
developed with these constraints. However, if satellite navigation providers have more space in their
signals (i.e. spare bits), it is still possible to use these extra bits for extending the information contained
in the EWM (e.g. for adding more severity levels, or more instructions).
*/

/*
1. Message type (A1)
00 Test
01 Alert
10 Update
11 All Clear
*/

#define EWSS_MESSAGE_TYPE_MASK			0b11
#define EWSS_MESSAGE_TYPE_TEST			0b00
#define EWSS_MESSAGE_TYPE_ALERT			0b01
#define EWSS_MESSAGE_TYPE_UPDATE		0b10
#define EWSS_MESSAGE_TYPE_ALL_CLEAR		0b11


// 00 Unknown
// 01 Moderate – possible threat to life or property
// 10 Severe – Significant threat to life or property
// 11 Extreme – Extraordinary threat to life or property

#define EWSS_SEVERITY_MASK				0b11
#define EWSS_SEVERITY_UNKNOWN			0b00
#define EWSS_SEVERITY_MODERATE			0b01
#define EWSS_SEVERITY_SEVERE			0b10
#define EWSS_SEVERITY_EXTREME			0b11


#define EWSS_SEVERITY_MASK				0b11
#define EWSS_SEVERITY_UNKNOWN			0b00


#define EWSS_WEEK_MASK					0b1
#define EWSS_WEEK_CURRENT				0b0
#define EWSS_WEEK_NEXTR					0b1


#define EWSS_DURATION_MASK				0b11
#define EWSS_DURATION_UNKNOWN			0b00
#define EWSS_DURATION_LESS_6H			0b01
#define EWSS_DURATION_6H_12H			0b10
#define EWSS_DURATION_12H_24H			0b11

#define EWSS_GUIDANCE_LIB_SELECTION_MASK			0b1
#define EWSS_GUIDANCE_LIB_SELECTION_INTL			0b0
#define EWSS_GUIDANCE_LIB_SELECTION_COUNTRY			0b1

/*
00 B1 – Improved resolution of main ellipse
01 B2 – Position of centre of hazard
10 B3 – Secondary ellipse definition
11 B4 – Quantitative and detailed information related to hazard
*/
#define EWSS_MAIN_SUBJECT_MASK							0b11
#define EWSS_MAIN_SUBJECT_IMPROVED_RESOLUTION			0b00
#define EWSS_MAIN_SUBJECT_POS_CENTRE					0b01
#define EWSS_MAIN_SUBJECT_SECOND_ELLIPSE_DEF			0b10
#define EWSS_MAIN_SUBJECT_QUANT_DETAILS_INFO			0b11


/**
 * @brief Common Alert Message Format
 * Transport: Data Frame Up
 */
struct __attribute__((__packed__)) CommonAlertMessageFormat
{
	/*
	 * @brief 1. Message type (A1)
	 */
	uint16_t message_type :2;

	/*
	 * @brief 2. Country / Region ID (A2)
	 */
	uint16_t country_id :9;

	/*
	 * @brief 3. Provider identifier (A3)
	 */
	uint16_t provider_id :5;

	/*
	 * @brief 4. Hazard category & type (A4)
	 */
	uint16_t hazard_category :9;

	/*
	 * @brief 5. Severity (A5)
	 */
	uint16_t severity :2;

	/*
	 * @brief 6. Hazard onset: week number (A6)
	 */
	uint16_t hazard_onset_week_number :1;

	/*
	 * @brief 7. Hazard onset: time of the week
	 */
	uint16_t hazard_onset_time_of_week :14;

	/*
	 * @brief 8. Hazard duration (A8)
	 */
	uint16_t hazard_duration :2;

	/*
	 * @brief 9. Guidance library selection (A9)
	 */
	uint16_t guidance_library_selection :1;

	/*
	 * @brief 10. Guidance library version (A10)
	 */
	uint16_t guidance_library_version :3;

	/*
	 * @brief 11. Guidance instructions (A11)
	 */
	uint16_t guidance_instructions_a : 5;

	/*
	 * @brief 11. Guidance instructions (A11)
	 */
	uint16_t guidance_instructions_b : 5;

	/*
	 * @brief 12. Ellipse centre latitude (A12)
	 */
	uint16_t ellipse_centre_latitude :16;

	/*
	 * @brief 13. Ellipse centre longitude (A13)
	 */
	uint32_t ellipse_centre_longitude :17;

	/*
	 * @brief 14. Ellipse semi-major axis length (A14)
	 */
	uint16_t ellipse_semi_major_axis_length :5;

	/*
	 * @brief 15. Ellipse semi-minor axis length (A15)
	 */
	uint16_t ellipse_semi_minor_axis_length :5;

	/*
	 * @brief 16. Ellipse azimuth angle (A16)
	 */
	uint16_t ellipse_azimuth_angle :6;

	/*
	 * @brief 17. Main subject (A17)
	 */
	uint16_t naimn_subject :2;

	/*
	 * @brief 18. Specific settings (A18)
	 */
	uint16_t specific_settings :15;

};

typedef struct CommonAlertMessageFormat CommonAlertMessageFormat_t;


/*
 *  @brief Common Alert Message LoRa Frame
 * Transport: Data Frame Dn (Ground -> Sat) : CRC=on, IQ=inverted
 */
struct __attribute__((__packed__)) CommonAlertMessageFrame
{
	/*
	 * @brief Header
	 */
	ServiceMessageHeader_t header;

	/*
	 * @brief Common Alert Message
	 */
	CommonAlertMessageFormat_t cam;

};

typedef struct CommonAlertMessageFrame CommonAlertMessageFrame_t;


/*
 * EWSSDnMessagePayload
 * @brief EWSS Dn Message Payload
 * Transport: Data Frame Up (Ground -> Sat)
 */
struct __attribute__((__packed__)) EWSSDnMessagePayload
{
	/*
	 * @brief Id of the order for CommonAlertMessageFormat broadcast
	 * Note: idempotent number
	 * Note: if a new payload wiith the same id but a newer start_time, the new message cancels and replaces the previous one
	 */
	uint32_t id;

    /*
     * @brief Start time (epoch in seconds)
     * 0 for cancelling the broadcast
     */
    uint32_t  start_time;

    /*
     * @brief Duration (in minutes)
     */
    uint16_t  duration;

    /*
     * @brief Geohash of the ground square to broadcast
     * TODO should be improved with ellipse
     * @see https://en.wikipedia.org/wiki/Geohash
     */
	uint32_t geohash;

    /*
     * @brief Precision of the geohash
     */
	uint8_t geohash_precision;

    /*
     * @brief Period of message repetition (in seconds)
     */
	uint8_t period;

    /*
     * @brief  alert message to broadcast
     */
	CommonAlertMessageFormat_t alert_message;

};

typedef struct EWSSDnMessagePayload EWSSDnMessagePayload_t;

void CommonAlertMessageFormat_printf(const CommonAlertMessageFormat_t* p);

void EWSSDnMessagePayload_printf(const EWSSDnMessagePayload_t* p, const uint8_t payload_size);


#endif // LORAWAN_EWSS_PAYLOAD_H
