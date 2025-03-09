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


#define hazard_onset_time_of_week(m) (m->hazard_onset_time_of_week_lsb | (m->hazard_onset_time_of_week_msb<<6))

#define guidance_instructions_a(m) (m->guidance_instructions_a_lsb | (m->guidance_instructions_a_msb<<2))

#define ellipse_centre_latitude(m) (m->ellipse_centre_latitude_lsb | (m->ellipse_centre_latitude_msb<<8))

#define ellipse_centre_longitude(m) (m->ellipse_centre_longitude_lsb | (m->ellipse_centre_longitude_msb<<8))

#define ellipse_semi_minor_axis_length(m) (m->ellipse_semi_minor_axis_length_lsb | (m->ellipse_semi_minor_axis_length_msb<<2))

#define specific_settings(m) (m->specific_settings_lsb | (m->specific_settings_msb<<5))


/*
 * @brief Main Subject for Specific Settings (A17)
 * B1 – Improved resolution of main ellipse
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B1 {

	uint16_t c1_latitude_of_centre_of_main_ellipse :3;
	uint16_t c2_Longitude_of_centre_of_main_ellipse :3;
	uint16_t c3_semi_major_axis :3;
	uint16_t c4_semi_minor_axis :3;
	uint16_t reserved :3;
};
typedef struct CAMF_SpecificSettings_B1 CAMF_SpecificSettings_B1_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B2 – Position of centre of hazard
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B2 {

	uint16_t c5_delta_latitude_from_main_ellipse_centre: 7;
	uint16_t c6_delta_longitude_from_main_ellipse_centre: 7;
	uint16_t reserved: 1;
};
typedef struct CAMF_SpecificSettings_B2 CAMF_SpecificSettings_B2_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B3 – Secondary ellipse definition
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B3 {

	uint16_t c7_shift_of_second_ellipse_centre : 2;
	uint16_t c8_homothetic_factor_of_second_ellipse : 3;
	uint16_t c9_bearing_angle_of_second_ellipse : 5;
	uint16_t c10_guidance_library_for_second_ellipse : 5;
};
typedef struct CAMF_SpecificSettings_B3 CAMF_SpecificSettings_B3_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B4 – Quantitative and detailed information related to hazard - Earthquake
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B4_Earthquake {

	uint16_t d1_magnitude_on_richter_scale: 4;
	uint16_t d2_seismic_coefficient: 3;
	uint16_t d3_azimuth_from_centre_of_main_ellipse_to_epicentre: 4;
	uint16_t d4_vector_length_between_centre_of_main_ellipse_and_epicentre : 4;
};
typedef struct CAMF_SpecificSettings_B4_Earthquake CAMF_SpecificSettings_B4_Earthquake_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B4 – Quantitative and detailed information related to hazard - Tsunami / tidal wave
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B4_Tsunami {

	uint16_t d5_wave_height: 3;
	uint16_t reserved: 12;
};
typedef struct CAMF_SpecificSettings_B4_Tsunami CAMF_SpecificSettings_B4_Tsunami_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B4 – Quantitative and detailed information related to hazard - Cold wave / heat wave
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B4_Temp_wave {

	uint16_t d6_temperature_range: 4;
	uint16_t reserved: 11;
};
typedef struct CAMF_SpecificSettings_B4_Temp_wave CAMF_SpecificSettings_B4_Temp_wave_t;

/*
 * @brief Main Subject for Specific Settings (A17)
 * B4 – Quantitative and detailed information related to hazard - Tropical cyclone (hurricane)
 */
struct __attribute__((__packed__)) CAMF_SpecificSettings_B4_Tropical_cyclone {

	uint16_t d7_hurricane_categories: 3;
	uint16_t d8_wind_speed: 4;
	uint16_t d9_rainfall_amounts: 9;
	uint16_t reserved: 5;
};
typedef struct CAMF_SpecificSettings_B4_Tropical_cyclone CAMF_SpecificSettings_B4_Tropical_cyclone_t;

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
	 * @brief 5. Severity (A5)
	 */
	uint16_t severity :2;

	/*
	 * @brief 4. Hazard category & type (A4)
	 * @see Common Alerting Protocol http://docs.oasis-open.org/emergency/cap/v1.2/CAP-v1.2.html
	 * @see Common Alerting Protocol https://en.wikipedia.org/wiki/Common_Alerting_Protocol
	 */
	uint16_t hazard_category :7;

	/*
	 * @brief 6. Hazard onset: week number (A6)
	 */
	uint16_t hazard_onset_week_number :1;

	/*
	 * @brief 7. Hazard onset: time of the week
	 */
	uint16_t hazard_onset_time_of_week_lsb :6;
	uint16_t hazard_onset_time_of_week_msb :8;

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
	uint16_t guidance_instructions_a_lsb : 2;
	uint16_t guidance_instructions_a_msb : 3;

	/*
	 * @brief 11. Guidance instructions (A11)
	 */
	uint16_t guidance_instructions_b : 5;

	/*
	 * @brief 12. Ellipse centre latitude (A12)
	 */
	uint16_t ellipse_centre_latitude_lsb :8;
	uint16_t ellipse_centre_latitude_msb :8;

	/*
	 * @brief 13. Ellipse centre longitude (A13)
	 */
	uint16_t ellipse_centre_longitude_lsb :8;
	uint16_t ellipse_centre_longitude_msb :9;

	/*
	 * @brief 14. Ellipse semi-major axis length (A14)
	 */
	uint16_t ellipse_semi_major_axis_length :5;

	/*
	 * @brief 15. Ellipse semi-minor axis length (A15)
	 */
	uint16_t ellipse_semi_minor_axis_length_lsb :2;
	uint16_t ellipse_semi_minor_axis_length_msb :3;

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
	uint16_t specific_settings_lsb :5;
	uint16_t specific_settings_msb :10;

	uint16_t padding :6;
	
};

typedef struct CommonAlertMessageFormat CommonAlertMessageFormat_t;

#endif // CAMF_PAYLOAD_H
