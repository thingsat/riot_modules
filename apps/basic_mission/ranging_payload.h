/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef APPS_BASIC_MISSION_RANGING_PAYLOAD_H_
#define APPS_BASIC_MISSION_RANGING_PAYLOAD_H_

#define ENABLE_DEBUG		ENABLE_DEBUG_RANGING
#include "debug.h"

#include "ranging.h"


void ranging_payload_range1_printf(const Ranging01Payload_t* p);

void ranging_payload_range2_printf(const Ranging2Payload_t* p);

void ranging_payload_range3_printf(const Ranging3Payload_t* p);

#endif /* APPS_BASIC_MISSION_RANGING_PAYLOAD_H_ */
