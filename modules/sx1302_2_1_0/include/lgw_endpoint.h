/*
 SX1302 LGW commands
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef _LGW_ENDPOINT_H
#define _LGW_ENDPOINT_H

/*
 * Endpoint information for transmitting and repeating LoRa frames
 */

/*
 * @see https://github.com/csu-grenoble/flatsat/tree/main/Hardware/sx1302_modules
 */
typedef enum {
	RAK5146_SPI_GNSS, // + PPS
	RAK5146_USB_GNSS, // + PPS
	RAK5146_SPI_NO_GNSS,
	RAK5146_USB_NO_GNSS,
	WM1302_SPI,
	WM1303_SPI,
	NEBRA1303_SPI,
	HT1303_SPI,
	UNKNOWN_SX130X_MODULE,
	FIX_GATEWAY,
	OTHER,
} lgw_sx130x_module_t;

typedef struct  {
	char* label;
	uint32_t devaddr;
	uint64_t deveui;
	uint8_t nwkskey[16];
	uint8_t appskey[16];
	uint32_t fcntup; // initial frame counter for data up
	lgw_sx130x_module_t module;
	// NB: module can be used for configuring GNSS and PPS

} lorawan_endpoint_t;

extern lorawan_endpoint_t* lgw_sx130x_endpoint;

#endif //_LGW_ENDPOINT_H
