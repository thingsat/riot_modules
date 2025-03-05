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
    printf("country_id:                      %d\n",m->country_id);
    printf("country_id:                      %d\n",m->country_id);
    printf("provider_id:                     %d\n",m->provider_id);
    printf("hazard_category:                 %d (%s)\n",m->hazard_category, camf_a4_en[m->hazard_category][0]); // TODO check camf_a4_en_nb_rows
    printf("severity:                        %d\n",m->severity);
    printf("hazard_onset_week_number:        %d\n",m->hazard_onset_week_number);
    printf("hazard_onset_time_of_week:       %d\n",m->hazard_onset_time_of_week);
    printf("hazard_duration:                 %d\n",m->hazard_duration);
    printf("guidance_library_selection:      %d\n",m->guidance_library_selection);

    printf("guidance_library_version:        %d\n",m->guidance_library_version);
    printf("guidance_instructions_a:         %d\n",m->guidance_instructions_a);
    printf("guidance_instructions_b:         %d\n",m->guidance_instructions_b);

    printf("ellipse_centre_latitude:         %d\n",m->ellipse_centre_latitude);
    printf("ellipse_centre_longitude:        %d\n",m->ellipse_centre_longitude);
    printf("ellipse_semi_major_axis_length:  %d\n",m->ellipse_semi_major_axis_length);
    printf("ellipse_semi_minor_axis_length:  %d\n",m->ellipse_semi_minor_axis_length);
    printf("ellipse_azimuth_angle:           %d\n",m->ellipse_azimuth_angle);
    printf("main_subject:                    %d\n",m->main_subject);
    printf("specific_settings:               %d\n",m->specific_settings);
}
