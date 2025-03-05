/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Author : Didier Donsez, Université Grenoble Alpes
 */

#ifndef _CAMF_MESSAGES_H_
#define _CAMF_MESSAGES_H_

#include "values/camf_18_3_1.h"
#include "values/camf_18_3_2.h"
#include "values/camf_18_3_3.h"

//  language dependent messages

#ifndef EWSS_CAMF_INCLUDE_EN
#define EWSS_CAMF_INCLUDE_EN    1
#endif

#ifndef EWSS_CAMF_INCLUDE_FR
#define EWSS_CAMF_INCLUDE_FR    1
#endif

#if EWSS_CAMF_INCLUDE_EN == 1
#include "camf_messages_en.h"
#endif

#if EWSS_CAMF_INCLUDE_FR == 1
#include "camf_messages_fr.h"
#endif

#endif
