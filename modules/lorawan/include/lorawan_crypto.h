/*
 * Copyright (C) 2020-2022 Université Grenoble Alpes
 */

/*
 * Utils for the MIC control
 *
 * @Remark: the MIC is similar to the LoRaWAN frames MIC (which is based on CMAC).
 *
 */

#ifndef LORAWAN_CRYPTO_H
#define LORAWAN_CRYPTO_H

#include <inttypes.h>
#include <stdbool.h>

#define LORAMAC_MIC_LEN								(sizeof(uint32_t))

/*!
 * Frame direction definition for up-link communications
 */
#define LORAMAC_DIR_UPLINK							(0U)

/*!
 * Frame direction definition for down-link communications
 */
#define LORAMAC_DIR_DOWNLINK						(1U)

/*
 * @brief Compute the MIC of LoRaWAN Data frames
 * frame_size should not include the MIC len
 */
void lorawan_cmac_calculate_mic(uint8_t *frame_buffer, uint16_t frame_size, const uint8_t *nwkskey, uint32_t devaddr, uint8_t dir, uint32_t sequenceCounter, uint32_t *mic);

/*
 * @brief Check the MIC of LoRaWAN Data frames
 * frame_size should include the MIC len
 */
bool lorawan_cmac_check_mic(const uint8_t *frame_buffer, uint16_t frame_size, const uint8_t *nwkskey, uint32_t devaddr, uint8_t dir, uint32_t sequenceCounter);

/*
 * @brief Encrypt the payload of LoRaWAN Data frames
 */
void lorawan_payload_encrypt(const uint8_t *payload_buffer, const uint16_t payload_size, const uint8_t *key, const uint32_t devaddr, const uint8_t dir, const uint32_t sequenceCounter, uint8_t *encrypt_buffer);

/*
 * @brief Decrypt the payload of LoRaWAN Data frames
 */
void lorawan_payload_decrypt(const uint8_t *payload_buffer, const uint16_t payload_size, const uint8_t *key, const uint32_t devaddr, const uint8_t dir, const uint32_t sequenceCounter, uint8_t *decrypt_buffer);

#endif
