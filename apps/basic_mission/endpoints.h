/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * This file contains the identity of SX1302/3 boards used for testing
 */

#ifndef TESTS_DRIVER_SX1302_ENDPOINTS_H_
#define TESTS_DRIVER_SX1302_ENDPOINTS_H_

#include <stdbool.h>
#include "lgw_endpoint.h"

extern lorawan_endpoint_t *lgw_sx130x_endpoint;

bool set_endpoint(void);

const lorawan_endpoint_t* endpoint_get_endpoint(const uint32_t devaddr);



#endif /* TESTS_DRIVER_SX1302_ENDPOINTS_H_ */
