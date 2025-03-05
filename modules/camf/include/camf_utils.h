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

/**
 * @brief check if a point (x, y) is inside an ellipse with center (h, k) and semi-axes a and b
 */
bool camf_is_into_ellipse(float x, float y, float h, float k, float a, float b);

void camf_message_printf(const CommonAlertMessageFormat_t* m);

#endif // CAMF_UTILS_H