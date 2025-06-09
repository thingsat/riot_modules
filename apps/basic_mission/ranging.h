/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef APPS_BASIC_MISSION_RANGING_H_
#define APPS_BASIC_MISSION_RANGING_H_

#include <stdint.h>
#include <stdbool.h>

#include "lorawan_ranging_payload.h"

bool ranging_get_fpayload_1(
	/**
	 * @brief FPayload to fill
	 */
	uint8_t *fpayload,
	/**
	 * @brief size of FPayload to fill
	 */
	uint8_t *fpayload_size,
	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	const uint8_t txpower,
	/**
	 * @brief Tx Power in dBm
	 * Useful for ADR
	 */
	const bool ftime
);

#endif /* APPS_BASIC_MISSION_RANGING_H_ */
