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
#include <math.h>
#include "fmt.h"

#include "cmac_utils.h"

#include "lora_mesh.h"

uint8_t lora_mesh_get_snr_u(const float snr) {
	int8_t _snr = floor(snr);
	return _snr >> 2;
}

float lora_mesh_get_snr_f(const uint8_t snr) {
	return (float)((int8_t) (snr << 2));
}


uint8_t lora_mesh_get_channel(const uint32_t freq_hz, const uint32_t* frequency_plan, const uint32_t frequency_plan_len) {
	uint8_t i;
	for(i=0; i<frequency_plan_len; i++) {
		if(frequency_plan[i] == freq_hz) {
			return i;
		}
	}
	return 0xffU;
}


bool lora_mesh_build_uplink(
		uint8_t *frame_buffer,
		uint8_t *size,
		const uint8_t hop_count,
		const uint16_t uplink_id,
		const uint8_t datarate,
		const float rssi,
		const float snr,
		const uint8_t channel,
		const uint32_t relay_id,
		const uint8_t *phypayload,
		const uint8_t phypayload_size,
		const uint8_t *signing_key
) {
	if(phypayload_size > LORAWAN_PHYPAYLOAD_LEN - 14) {
		return false;
	}

	MeshLoRa_Uplink_t *uplink = (MeshLoRa_Uplink_t*)frame_buffer;
	*size = 14 + phypayload_size;

	uplink->mtype = LORAMESH_MTYPE_PROPRIETARY;
	uplink->payload_type = LORAMESH_PAYLOAD_TYPE_UPLINK;
	uplink->hop_count = hop_count;
	uplink->uplink_id = uplink_id;
	uplink->datarate = datarate;
	uplink->rssi = floor(rssi)*-1;
	uplink->snr = lora_mesh_get_snr_u(snr);
	uplink->channel = channel;
	uplink->relay_id = relay_id;

	memcpy(frame_buffer+10, phypayload, phypayload_size);

	uint32_t mic;
	Cmac_calculate_mic(frame_buffer, signing_key, (10+phypayload_size), &mic);
	memcpy(frame_buffer+(10+phypayload_size), &mic, sizeof(mic));

	return true;
};


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

/** get hop count */
bool lora_mesh_get_hop_count(const uint8_t *frame_buffer,
		const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;
	return mhdr->hop_count;
}


/** Get Relay Id */
uint32_t lora_mesh_get_relay_id(const uint8_t *frame_buffer,
		const uint8_t size) {
	(void) size;

	const MeshLoRa_MHDR_t *mhdr = (const MeshLoRa_MHDR_t*) frame_buffer;

	uint32_t relay_id = 0;

	switch (mhdr->payload_type) {
	case LORAMESH_PAYLOAD_TYPE_UPLINK:
		relay_id  = ((MeshLoRa_Uplink_t*) frame_buffer)->relay_id;
		break;
	case LORAMESH_PAYLOAD_TYPE_DOWNLINK:
		relay_id  = ((MeshLoRa_Downlink_t*) frame_buffer)->relay_id;
		break;
	case LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE:
		relay_id  = ((MeshLoRa_RelayHeartbeat_t*) frame_buffer)->relay_id;
		break;
	default:
		break;
	}
	return relay_id;
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
			"LoRaMesh Uplink: hopcount=%d uplink_id=%u datarate=%u rssi=%0.1f snr=%0.1f channel=%d relay_id=%08lX mic=%08lX payload=",
			uplink->hop_count, uplink->uplink_id, uplink->datarate,
			(float)-1.0*uplink->rssi, lora_mesh_get_snr_f(uplink->snr), uplink->channel,
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
