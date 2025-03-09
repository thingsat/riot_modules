/*
 * Copyright (C) 2025 Université Grenoble Alpes
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Common Alert Message for FIRE - Forest fire
 *
 * @author      Didier DONSEZ, Université Grenoble Alpes
 *
 * @}
 */
static const CommonAlertMessageFormat_t camf_sample_msg_fire = {
	message_type: 2, // UPDATE
	country_id: 74, // FRANCE
	provider_id: 5, // Random
	severity: 0b11, // EXTERME
	hazard_category: 0b0011011, // FIRE - Forest fire 
	hazard_onset_week_number: 0b0, // Current
	hazard_onset_time_of_week_lsb: 0b011111, // 01000011 011111 WEDNESDAY - 11: 58 PM
	hazard_onset_time_of_week_msb: 0b01000011,
	hazard_duration: 2, // 6h <= Duration < 12h
	guidance_library_selection: 0,
	guidance_library_version: 2,
	guidance_instructions_a_lsb: 0b00010 & 0b11,
	guidance_instructions_a_msb: 0b00010 >> 2,
	guidance_instructions_b: 0b00110, // IC-B-07 Pay attention to announcements made by the police, fire brigade and by officials.
	ellipse_centre_latitude_lsb: 0b1011111111111111 & 0b11111111, // 44,999°
	ellipse_centre_latitude_msb: 0b1011111111111111 >> 8,
	ellipse_centre_longitude_lsb: 0b10000000000000001 & 0b11111111, // 0.004°
	ellipse_centre_longitude_msb: 0b10000000000000001 >> 8, // 0.004°
	ellipse_semi_major_axis_length: 0b11110, // 1848,727 km
	ellipse_semi_minor_axis_length_lsb: 0b10010 & 0b11, // 49.439 km
	ellipse_semi_minor_axis_length_msb: 0b10010 >> 2,
	ellipse_azimuth_angle: 0b010000, // 45°
	main_subject: 0b00, // B4 - Quantitative and detailed information related to hazard category and type
	specific_settings_lsb: 0b111110000011111 & 0b11111,
	specific_settings_msb: 0b11111000001111 >> 5,
	padding: 0,
};


/*
00000000000000000000000000000000 00000000000000000000000000000000
00000000 100010001001001001000011 01111100011011111011100100101010

000000000000000001111 11 010000 100-10 11110 10000000000000001 10111111
11111111 00110 000-10 010 0 10 01000011-011111 0 0011011 11 - 10111 001001010 10

0b01111100011011111011100100101010,
0b11111111001100001001001001000011,
0b10101001000000000000000110111111,
0b00000000000000000111111010000100


************ CAMF Message 2 ***********
message_type:                    2 (Update)
country_id:                      74 (France)
provider_id:                     23
severity:                        3 (Extreme)
hazard_category:                 27 (FIRE - Fumes - An often-noxious suspension of particles in the air.)
hazard_onset_week_number:        0 (Current)
hazard_onset_time_of_week:       4319  (WED 23:59)
hazard_duration:                 2 (6 <= Duration < 12)
guidance_library_selection:      0 (International Guidance Library)
guidance_library_version:        2
guidance_instructions_a:         2 (IC-A-03 : You are in the danger zone, leave the area immediately and reach the evacuation point indicated by the area plotted in yellow. Listen to radio or media for directions and information.)
guidance_instructions_b:         6 (IC-B-07 : Pay attention to announcements made by the police, fire brigade and by officials.)
ellipse_centre_latitude:         49151 (44.999317)
ellipse_centre_longitude:        65537 (0.004121)
ellipse_semi_major_axis_length:  20 (1.736000)
ellipse_semi_minor_axis_length:  18 (1.584000)
ellipse_azimuth_angle:           16 (-45.702499)
main_subject:                    3 (B4 - Quantitative and detailed information related to hazard category and type : Option to provide quantitative or more detailed information related to the hazard category and type.)
specific_settings:               15

*/
