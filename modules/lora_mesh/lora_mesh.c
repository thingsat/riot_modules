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

#include "lora_mesh.h"

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
uint8_t lorawan_get_type(const uint8_t *frame_buffer, const uint8_t size) {

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
	uint32_t mic = lora_mesh_get_mic(frame_buffer, size);

	return true;
}

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_uplink(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_UPLINK;
}

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_downlink(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_DOWNLINK;
}

/** Is LoRa Mesh Relay Heartbeat */
bool lora_mesh_is_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size) {
	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->payload_type == LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE;
}

/** Get LoRaWAN PhyPayload */
uint8_t lora_mesh_get_payload_size(const uint8_t *frame_buffer,
		const uint8_t size) {
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
uint8_t* lora_mesh_get_payload(const uint8_t *frame_buffer, const uint8_t size) {
	return frame_buffer + 11;
}

void lora_mesh_printf_uplink(const uint8_t *frame_buffer, const uint8_t size) {
	const MeshLoRa_Uplink_t *uplink = (const MeshLoRa_Uplink_t*) frame_buffer;

	printf(
			"LoRaMesh Uplink: hopcount=%n uplink_id=%u datarate=%u rssi=%d snr=%d channel=%d relay_id=%08lX mic=%08lX",
			uplink->hop_count, uplink->uplink_id, uplink->datarate,
			-uplink->rssi, uplink->snr, uplink->channel, uplink->snr,
			uplink->relay_id, lora_mesh_get_mic(frame_buffer, size));
}

void lora_mesh_printf_downlink(const uint8_t *frame_buffer, const uint8_t size) {
	printf(
			"LoRaMesh Downlink: hopcount=%n uplink_id=%u downlink_datarate=%u downlink_frequency=%d tx_power=%d delay=%d relay_id=%08lX mic=%08lX",
			uplink->hop_count, uplink->uplink_id, uplink->downlink_datarate,
			-uplink->downlink_frequency, uplink->tx_power, uplink->delay,
			uplink->relay_id, lora_mesh_get_mic(frame_buffer, size));
}

void lora_mesh_printf_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size) {
	printf(
			"LoRaMesh Relay Heartbeat: hopcount=%n timestamp=%lu relay_id=%08lX mic=%08lX",
			uplink->hop_count, uplink->timestamp, uplink->relay_id,
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
