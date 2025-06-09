/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef APPS_BASIC_MISSION_TELEMETRY_H_
#define APPS_BASIC_MISSION_TELEMETRY_H_

#include "lorawan_telemetry_payload.h"

bool telemetry_get_fpayload(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size,
		/**
		 * @brief us counter at TX
		 */
		const uint32_t		tx_uscount,
		/**
		 * @brief us co;unter at the previous ranging message TX (fcntup - 1)
		 */
		const uint32_t		tx_uscount_prev,
		/**
		 * @brief Ranging Status
		 * 1 for GPS_FIX, 2 for FTIME, TxMode ()
		 */
		const uint8_t		status,
		/**
		 * @brief Tx Power in dBm
		 * Useful for ADR
		 */
		const uint8_t		txpower
);

#endif /* APPS_BASIC_MISSION_TELEMETRY_H_ */
