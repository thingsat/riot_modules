/*
 * aiop13_wrapper.h
 *
 *  Created on: 3 juin 2022
 *  Author: donsez
 */

#ifndef AIOP13_WRAPPER_H_
#define AIOP13_WRAPPER_H_

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef void* P13Predict_t;

EXTERNC P13Predict_t P13Predict_new(const char *p_ccSatName, const char *p_ccl1, const char *p_ccl2);

EXTERNC void P13Predict_destroy(P13Predict_t instance);

EXTERNC void P13Predict_predict(P13Predict_t instance, const int p_iyear, const int p_imonth, const int p_iday, const int p_ih, const int p_im, int p_is);

EXTERNC void P13Predict_latlon(P13Predict_t instance, double *p_dlat, double *p_dlon);


#endif /* AIOP13_WRAPPER_H_ */
