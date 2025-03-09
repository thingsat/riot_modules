/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */


/*
 * CAMF utilities functions
 */


 #ifndef CAMF_UTILS_H
 #define CAMF_UTILS_H
 
 #include <stdint.h>
 #include <stdbool.h>

 #include "camf_payload.h"

float get_C1_Refined_Ellipse_Centre_Location_Latitude(uint32_t v);

float get_C2_Refined_Ellipse_Centre_Location_Longitude(uint32_t v);

float get_C3_Refined_Ellipse_Semi_Major_Axis_Length(uint32_t v);

float get_C4_Refined_Ellipse_Semi_Minor_Axis_Length(uint32_t v);

float get_C5_Delta_Latitude_from_Main_Ellipse_Centre(uint32_t v);

float get_C6_Delta_Longitude_from_Main_Ellipse_Centre(uint32_t v);

float get_Ellipse_Azimuth_Angle(uint32_t v);

/**
 * @brief check if a point (x, y) is inside an ellipse with center (h, k) and semi-axes a and b
 */
bool camf_is_into_ellipse(float x, float y, float h, float k, float a, float b);

void camf_message_printf(const CommonAlertMessageFormat_t* m);

#endif // CAMF_UTILS_H