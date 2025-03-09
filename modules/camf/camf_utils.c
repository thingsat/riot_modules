/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */

/*
 * CAMF utilities functions
 */

#include <stdio.h>

#include "camf_messages.h"
#include "camf_utils.h"


static const char* DOW_STR[7] = {"MON","TUE","WED","THU","FRI","SAT","SUN"};


float get_A12_Ellipse_Centre_Latitude(uint32_t v) {
    return     -90.000 + v*0.002746624;
}

float get_A13_Ellipse_Centre_Longitude(uint32_t v) {
    return     -180.000 + v*0.002746603;
}

float get_A14_Ellipse_semi_major_axis(uint32_t v) {
    return     0.216 + v*(0.292-0.216);
}

float get_A15_Ellipse_semi_minor_axis(uint32_t v) {
    return  get_A14_Ellipse_semi_major_axis(v);
}

float get_C1_Refined_Ellipse_Centre_Location_Latitude(uint32_t v) {
    return 0.000343328  * v;
}

float get_C2_Refined_Ellipse_Centre_Location_Longitude(uint32_t v) {
    return 0.000343325  * v;
}

float get_C3_Refined_Ellipse_Semi_Major_Axis_Length(uint32_t v) {
    return 0.125 * v;
}

float get_C4_Refined_Ellipse_Semi_Minor_Axis_Length(uint32_t v) {
    return 0.125 * v;
}

float get_C5_Delta_Latitude_from_Main_Ellipse_Centre(uint32_t v) {
    return -10 + (0.15625 * v);
}

float get_C6_Delta_Longitude_from_Main_Ellipse_Centre(uint32_t v) {
    return -10 + (0.15625 * v);
}

float get_Ellipse_Azimuth_Angle(uint32_t v) {
    return -90 + v*((90.00 + 87.19)/64);
}

/**
 * @brief check if a point (x, y) is inside an ellipse with center (h, k) and semi-axes a and b
 */
bool camf_is_into_ellipse(float x, float y, float h, float k, float a, float b) {
    //  ((x - h)² / a²) + ((y - k)² / b²) <= 1
    float e = ((x - h) * (x - h)) / (a * a) + ((y - k) * (y - k)) / (b * b);
    return e <= 1.0;
}

void camf_message_printf(const CommonAlertMessageFormat_t* m) {
    printf("message_type:                    %d (%s)\n",m->message_type, camf_a1_en[m->message_type][0]); // TODO check camf_a1_en_nb_rows
    printf("country_id:                      %d (%s)\n",m->country_id, camf_a2_en[m->country_id][1]);
    printf("provider_id:                     %d\n",m->provider_id);

    printf("severity:                        %d (%s)\n",m->severity, camf_a5_en[m->severity][0]); // TODO check camf_a4_en_nb_rows
    printf("hazard_category:                 %d (%s - %s - %s)\n",m->hazard_category, camf_a4_en[m->hazard_category][0], camf_a4_en[m->hazard_category][1], camf_a4_en[m->hazard_category][2]); // TODO check camf_a4_en_nb_rows
 
    printf("hazard_onset_week_number:        %d (%s)\n",m->hazard_onset_week_number, camf_a6_en[m->hazard_onset_week_number][0]); // TODO check camf_a4_en_nb_rows

    const uint16_t _hazard_onset_time_of_week = hazard_onset_time_of_week(m);
    const uint16_t dow = _hazard_onset_time_of_week/(24*60);
    const uint16_t hour = (_hazard_onset_time_of_week%(24*60))/60;
    const uint16_t minute = (_hazard_onset_time_of_week%(24*60))%60;

    //printf("hazard_onset_time_of_week_lsb:       %d\n",m->hazard_onset_time_of_week_lsb);
    //printf("hazard_onset_time_of_week_msb:       %d\n",m->hazard_onset_time_of_week_msb);

    printf("hazard_onset_time_of_week:       %d  (%s %2d:%2d)\n",_hazard_onset_time_of_week, DOW_STR[dow], hour, minute);
    printf("hazard_duration:                 %d (%s)\n",m->hazard_duration, camf_a8_en[m->hazard_duration][0]); // TODO check camf_a4_en_nb_rows

    printf("guidance_library_selection:      %d (%s)\n",m->guidance_library_selection, camf_a9_en[m->guidance_library_selection][0]); // TODO check camf_a4_en_nb_rows
    printf("guidance_library_version:        %d\n",m->guidance_library_version);


    const uint16_t _guidance_instructions_a = guidance_instructions_a(m);

    printf("guidance_instructions_a:         %d (%s : %s)\n",guidance_instructions_a(m), camf_a11a_en[guidance_instructions_a(m)][0], camf_a11a_en[guidance_instructions_a(m)][1]); // TODO check camf_a4_en_nb_rows
    printf("guidance_instructions_b:         %d (%s : %s)\n",m->guidance_instructions_b, camf_a11b_en[m->guidance_instructions_b][0], camf_a11b_en[m->guidance_instructions_b][1]); // TODO check camf_a4_en_nb_rows

    printf("ellipse_centre_latitude:         %d (%f)\n",
        ellipse_centre_latitude(m),
        get_A12_Ellipse_Centre_Latitude(ellipse_centre_latitude(m)));

    printf("ellipse_centre_longitude:        %d (%f)\n",
        ellipse_centre_longitude(m),
        get_A13_Ellipse_Centre_Longitude(ellipse_centre_longitude(m))
    );
    printf("ellipse_semi_major_axis_length:  %d (%f)\n",
        m->ellipse_semi_major_axis_length,
        get_A14_Ellipse_semi_major_axis(m->ellipse_semi_major_axis_length)
    );
    printf("ellipse_semi_minor_axis_length:  %d (%f)\n",
        ellipse_semi_minor_axis_length(m),
        get_A15_Ellipse_semi_minor_axis(ellipse_semi_minor_axis_length(m))
    );
    printf("ellipse_azimuth_angle:           %d (%f)\n",
        m->ellipse_azimuth_angle,
        get_Ellipse_Azimuth_Angle(m->ellipse_azimuth_angle)
    );
    printf("main_subject:                    %d (%s - %s : %s)\n",m->main_subject, camf_a17_en[m->main_subject][0], camf_a17_en[m->main_subject][1], camf_a17_en[m->main_subject][2]);

    printf("specific_settings:               %d\n",specific_settings(m));
}
