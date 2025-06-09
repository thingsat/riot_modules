/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef APPS_BASIC_MISSION_APRS_H_
#define APPS_BASIC_MISSION_APRS_H_

#define FPORT_APRS_PAYLOAD            (14U)

bool aprs_get_fpayload(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size
);


bool aprs_get_ax25(
		/**
		 * @brief FPayload to fill
		 */
		uint8_t *fpayload,
		/**
		 * @brief size of FPayload to fill
		 */
		uint8_t *fpayload_size
);


#endif /* APPS_BASIC_MISSION_APRS_H_ */
