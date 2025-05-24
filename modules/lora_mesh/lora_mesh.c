/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Utils for the LoRa Mesh
 *
 * @link https://www.chirpstack.io/docs/chirpstack-gateway-mesh/protocol.html#downlink-payload-format
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fmt.h"

#include "cmac_utils.h"

#include "lora_mesh.h"


static bool _check_mic(const uint8_t *buf, const size_t buf_len,
		const uint8_t *cmac_key) {
	mic_t mic_to_check;
	memcpy(&mic_to_check, buf + (buf_len - sizeof(mic_t)), sizeof(mic_t));
	return Cmac_check_mic(buf, cmac_key, buf_len - sizeof(mic_t), mic_to_check);
}


/** Check valid mesh frame */
bool lora_mesh_check_valid_frame(const uint8_t *frame_buffer,
		const uint8_t size) {

	// 14 uplink
	// 15 downlink
	// 13 relay heartbeat

	if (size < 13) {
		return false;
	}

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	if (mhdr->mtype != LORAMESH_MTYPE_PROPRIETARY || mhdr->payload_type == 0) {
		return false;
	}

	switch (mhdr->payload_type) {
	case LORAMESH_PAYLOAD_TYPE_UPLINK:
		if (size < 14)
			return false;
		break;
	case LORAMESH_PAYLOAD_TYPE_DOWNLINK:
		if (size < 15)
			return false;
		break;
	case LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE:
		if (size < 13)
			return false;
		break;
	default:
		return false;
		break;
	}

	return true;
}

/** Get Type */
uint8_t lora_mesh_get_type(const uint8_t *frame_buffer, const uint8_t size) {

	(void) size;
	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	return mhdr->payload_type;
}

/** Get frame MIC */
uint32_t lora_mesh_get_mic(const uint8_t *frame_buffer, const uint8_t size) {
	uint32_t mic;
	memcpy(&mic, frame_buffer + (size - sizeof(uint32_t)), sizeof(uint32_t));
	return mic;
}

/** Check frame MIC */
bool lora_mesh_check_mic(const uint8_t *frame_buffer, const uint8_t size,
		const uint8_t *aes_key) {

	(void) aes_key;
	uint32_t mic = lora_mesh_get_mic(frame_buffer, size);
	// TODO check mic
	(void) mic;
	return true;
}

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_uplink(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_UPLINK;
}

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_downlink(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_DOWNLINK;
}

/** Is LoRa Mesh Relay Heartbeat */
bool lora_mesh_is_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE;
}

/** Get LoRaWAN PhyPayload */
uint8_t lora_mesh_get_payload_size(const uint8_t *frame_buffer,
		const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	switch (mhdr->payload_type) {
	case LORAMESH_PAYLOAD_TYPE_UPLINK:
		return size - 14;
		break;
	case LORAMESH_PAYLOAD_TYPE_DOWNLINK:
		return size - 15;
		break;
	default:
		return 0;
		break;
	}
}

/** Get LoRaWAN PhyPayload  */
const uint8_t* lora_mesh_get_payload(const uint8_t *frame_buffer, const uint8_t size,
		uint8_t *lorawan_phypayload_size) {

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	switch (mhdr->payload_type) {

	case LORAMESH_PAYLOAD_TYPE_UPLINK:
		*lorawan_phypayload_size = size - 14;
		return frame_buffer + 10;
		break;
	case LORAMESH_PAYLOAD_TYPE_DOWNLINK:
		*lorawan_phypayload_size = size - 15;
		return frame_buffer + 11;
		break;
	default:
		*lorawan_phypayload_size = 0;
		return 0;
		break;
	}
}

static void printf_ba(const uint8_t *ba, size_t len)
{
    for (unsigned int i = 0; i < len; i++) {
        printf("%02x", ba[i]);
    }
}


void lora_mesh_printf_uplink(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_Uplink_t *uplink = (const MeshLoRa_Uplink_t*) frame_buffer;

	printf(
			"LoRaMesh Uplink: hopcount=%d uplink_id=%u datarate=%u rssi=%d snr=%d channel=%d relay_id=%08lX mic=%08lX payload=",
			uplink->hop_count, uplink->uplink_id, uplink->datarate,
			-uplink->rssi, uplink->snr, uplink->channel,
			uplink->relay_id, lora_mesh_get_mic(frame_buffer, size));
	printf_ba(frame_buffer+10, size-14);
}

void lora_mesh_printf_downlink(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_Downlink_t *downlink = (const MeshLoRa_Downlink_t*) frame_buffer;

	printf(
			"LoRaMesh Downlink: hopcount=%d uplink_id=%u downlink_datarate=%u downlink_frequency=%d tx_power=%d delay=%d relay_id=%08lX mic=%08lX payload=",
			downlink->hop_count, downlink->uplink_id, downlink->downlink_datarate,
			-downlink->downlink_frequency, downlink->tx_power, downlink->delay,
			downlink->relay_id, lora_mesh_get_mic(frame_buffer, size));
	printf_ba(frame_buffer+11, size-15);

}

void lora_mesh_printf_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size) {
	const MeshLoRa_RelayHeartbeat_t *relayheartbeat = (const MeshLoRa_RelayHeartbeat_t*) frame_buffer;

	printf(
			"LoRaMesh Relay Heartbeat: hopcount=%d timestamp=%lu relay_id=%08lX mic=%08lX",
			relayheartbeat->hop_count, relayheartbeat->timestamp, relayheartbeat->relay_id,
			lora_mesh_get_mic(frame_buffer, size));
}

void lora_mesh_printf_frame(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	switch (mhdr->payload_type) {
	case LORAMESH_PAYLOAD_TYPE_UPLINK:
		lora_mesh_printf_uplink(frame_buffer, size);
		break;
	case LORAMESH_PAYLOAD_TYPE_DOWNLINK:
		lora_mesh_printf_downlink(frame_buffer, size);
		break;
	case LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE:
		lora_mesh_printf_relay_heartbeat(frame_buffer, size);
		break;
	default:
		printf("unknown proprietary frame");
		break;
	}
}
