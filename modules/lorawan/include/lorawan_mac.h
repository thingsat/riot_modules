/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/*
 * Utils for the LoRaWAN MAC
 *
 */

#ifndef LORAWAN_MAC_H
#define LORAWAN_MAC_H

#include <inttypes.h>
#include <stdbool.h>

#if 0
// From https://github.com/Lora-net/LoRaMac-node/blob/master/src/mac/LoRaMacTypes.h
/*!
 * LoRaMAC frame types
 *
 * LoRaWAN Specification V1.0.2, chapter 4.2.1, table 1
 */
typedef enum eLoRaMacFrameType
{
    /*!
     * LoRaMAC join request frame
     */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /*!
     * LoRaMAC join accept frame
     */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /*!
     * LoRaMAC unconfirmed up-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /*!
     * LoRaMAC unconfirmed down-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /*!
     * LoRaMAC confirmed up-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /*!
     * LoRaMAC confirmed down-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /*!
     * LoRaMAC Rejoin Request
     */
    FRAME_TYPE_REJOIN                = 0x06,
    /*!
     * LoRaMAC proprietary frame
     */
    FRAME_TYPE_PROPRIETARY           = 0x07,
} LoRaMacFrameType_t;


/** Get Type */
bool lorawan_check_valid_frame_size(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRaWAN Size  */
// TODO bool lorawan_is_lorawan_size(const uint8_t *frame_buffer, const uint8_t size, const uint8_t datarate)

/** Get frame MIC */
uint32_t lorawan_get_mic(const uint8_t *frame_buffer, const uint8_t size);

/** Get Type */
uint8_t lorawan_get_type(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRaWAN Join Frame */
bool lorawan_is_joinframe(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRaWAN Data Frame */
bool lorawan_is_dataframe(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRaWAN Uplink */
bool lorawan_is_uplink(const uint8_t *frame_buffer, const uint8_t size);

/** Get Version */
uint8_t lorawan_get_version(const uint8_t *frame_buffer, const uint8_t size);

/** Get FHDR - FCnt */
uint16_t lorawan_get_fcnt(const uint8_t *frame_buffer, const uint8_t size);

/** Get FHDR - DevAddr */
uint32_t lorawan_get_devaddr(const uint8_t *frame_buffer, const uint8_t size);

/** Get FHDR - FCtrl -ADR */
bool lorawan_get_fctrl_adr(const uint8_t *frame_buffer, const uint8_t size);

/** Get FHDR - FCtrl - ACK */
bool lorawan_get_fctrl_ack(const uint8_t *frame_buffer, const uint8_t size);

/** Get FHDR - FCtrl - Pending */
bool lorawan_get_fctrl_pending(const uint8_t *frame_buffer, const uint8_t size);

/** Get FPort */
uint8_t lorawan_get_fport(const uint8_t *frame_buffer, const uint8_t size);

/** Get FPayload */
uint8_t* lorawan_get_fpayload(const uint8_t *frame_buffer, const uint8_t size);

/** Get FPayload size */
uint8_t lorawan_get_fpayload_size(const uint8_t *frame_buffer, const uint8_t size);

void lorawan_printf_dtup(const uint8_t *frame_buffer, const uint8_t size);

/** Get JoinEUI */
uint64_t lorawan_get_joineui(const uint8_t *frame_buffer, const uint8_t size);

/** Get DevEUI */
uint64_t lorawan_get_deveui(const uint8_t *frame_buffer, const uint8_t size);

/** Get DevNonce */
uint16_t lorawan_get_devnonce(const uint8_t *frame_buffer, const uint8_t size);

/** Get LoRaWAN datarate */
uint8_t lorawan_get_datarate(uint8_t sf, uint32_t bw);

void lorawan_printf_jreq(const uint8_t *frame_buffer, const uint8_t size);

void lorawan_printf_payload(const uint8_t *frame_buffer, const uint8_t size);

#endif

/** check the MIC of a uplink frame */
bool lorawan_check_mic_up(
		const uint32_t devaddr, const uint32_t fcnt,
		const uint8_t *nwkskey,
		const uint8_t *frame_buffer, const uint8_t frame_size);

/** check the MIC of a downlink frame */
bool lorawan_check_mic_dn(
		const uint32_t devaddr, const uint32_t fcnt,
		const uint8_t *nwkskey,
		const uint8_t *frame_buffer, const uint8_t frame_size);

/*
 * @brief Prepare a Data Up Frame
 * TODO add FOpt
 */
void lorawan_prepare_up_dataframe(
		const bool confirmed,
		const uint32_t devaddr,
		const uint8_t fctrl,
		const uint32_t fcnt,
		const uint32_t fport,
		const uint8_t *fpayload,
		const uint8_t fpayload_size,
		const uint8_t *nwkskey,
		const uint8_t *appskey,
		uint8_t *frame_buffer,
		uint8_t *frame_size
		);

/*
 * @brief Prepare a Data Dn Frame
 * TODO add FOpt
 */
void lorawan_prepare_dn_dataframe(
		const bool confirmed,
		const uint32_t devaddr,
		const uint8_t fctrl,
		const uint32_t fcnt,
		const uint32_t fport,
		const uint8_t *fpayload,
		const uint8_t fpayload_size,
		const uint8_t *nwkskey,
		const uint8_t *appskey,
		uint8_t *frame_buffer,
		uint8_t *frame_size
		);

#endif
