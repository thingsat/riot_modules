/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */


/*
 * CAMF message structure
 */


#ifndef CAMF_PAYLOAD_H
#define CAMF_PAYLOAD_H

#include <stdint.h>
#include <stdbool.h>

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

#define CAMF_MESSAGE_TYPE_MASK			0b11
#define CAMF_MESSAGE_TYPE_TEST			0b00
#define CAMF_MESSAGE_TYPE_ALERT			0b01
#define CAMF_MESSAGE_TYPE_UPDATE		0b10
#define CAMF_MESSAGE_TYPE_ALL_CLEAR		0b11


// 00 Unknown
// 01 Moderate – possible threat to life or property
// 10 Severe – Significant threat to life or property
// 11 Extreme – Extraordinary threat to life or property

#define CAMF_SEVERITY_MASK				0b11
#define CAMF_SEVERITY_UNKNOWN			0b00
#define CAMF_SEVERITY_MODERATE			0b01
#define CAMF_SEVERITY_SEVERE			0b10
#define CAMF_SEVERITY_EXTREME			0b11


#define CAMF_SEVERITY_MASK				0b11
#define CAMF_SEVERITY_UNKNOWN			0b00


#define CAMF_WEEK_MASK					0b1
#define CAMF_WEEK_CURRENT				0b0
#define CAMF_WEEK_NEXTR					0b1


#define CAMF_DURATION_MASK				0b11
#define CAMF_DURATION_UNKNOWN			0b00
#define CAMF_DURATION_LESS_6H			0b01
#define CAMF_DURATION_6H_12H			0b10
#define CAMF_DURATION_12H_24H			0b11

#define CAMF_GUIDANCE_LIB_SELECTION_MASK			0b1
#define CAMF_GUIDANCE_LIB_SELECTION_INTL			0b0
#define CAMF_GUIDANCE_LIB_SELECTION_COUNTRY			0b1

/*
00 B1 – Improved resolution of main ellipse
01 B2 – Position of centre of hazard
10 B3 – Secondary ellipse definition
11 B4 – Quantitative and detailed information related to hazard
*/
#define CAMF_MAIN_SUBJECT_MASK							0b11
#define CAMF_MAIN_SUBJECT_IMPROVED_RESOLUTION			0b00
#define CAMF_MAIN_SUBJECT_POS_CENTRE					0b01
#define CAMF_MAIN_SUBJECT_SECOND_ELLIPSE_DEF			0b10
#define CAMF_MAIN_SUBJECT_QUANT_DETAILS_INFO			0b11


/**
 * @brief Common Alert Message Format
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
	uint16_t main_subject :2;

	/*
	 * @brief 18. Specific settings (A18)
	 */
	uint16_t specific_settings :15;

};

typedef struct CommonAlertMessageFormat CommonAlertMessageFormat_t;

#endif // CAMF_PAYLOAD_H
