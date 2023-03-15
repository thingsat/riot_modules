#include "aiop13_wrapper.h"
#include "AioP13.h"

P13Predict_t P13Predict_new(const char *p_ccSatName, const char *p_ccl1, const char *p_ccl2)
{
    return new P13Predict(p_ccSatName, p_ccl1, p_ccl2);
}

void P13Predict_destroy(P13Predict_t instance)
{
	P13Predict* typed_ptr = static_cast<P13Predict*>(instance);
    delete typed_ptr;
}

void P13Predict_predict(P13Predict_t instance, const int p_iyear, const int p_imonth, const int p_iday, const int p_ih, const int p_im, int p_is)
{
	P13Predict* typed_self = static_cast<P13Predict*>(instance);
	typed_self->predict(p_iyear, p_imonth, p_iday, p_ih, p_im, p_is);
}

void P13Predict_latlon(P13Predict_t instance, double* p_dlat, double* p_dlon)
{
	P13Predict* typed_self = static_cast<P13Predict*>(instance);
	typed_self->latlon(*p_dlat, *p_dlon);
}


