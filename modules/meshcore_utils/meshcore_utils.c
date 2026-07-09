/*
 * Copyright (C) 2025 Université Grenoble Alpes
 */

/*
 * Utils for MeshCore
 *
 * Packet format (v1, firmware v1.12.0):
 * [header][transport_codes(optional)][path_length][path][payload]
 *
 * @see https://github.com/meshcore-dev/MeshCore/blob/main/docs/packet_format.md
 * @see https://github.com/meshcore-dev/MeshCore/blob/main/docs/payloads.md
 */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "meshcore_utils.h"

/* ------------------------------------------------------------------------- */
/* Internal offset helpers                                                    */
/* ------------------------------------------------------------------------- */

/** Offset of the path_length byte : 1 (header) + 4 optional transport codes */
static uint8_t _path_length_offset(const uint8_t *frame_buffer) {
	return ((frame_buffer[0] & MESHCORE_PH_ROUTE_MASK) == MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD
			|| (frame_buffer[0] & MESHCORE_PH_ROUTE_MASK) == MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT) ?
			5 : 1;
}

/** Offset of the first path byte */
static uint8_t _path_offset(const uint8_t *frame_buffer) {
	return _path_length_offset(frame_buffer) + 1;
}

/** Offset of the first payload byte */
static uint16_t _payload_offset(const uint8_t *frame_buffer, const uint8_t frame_size) {
	return (uint16_t) _path_offset(frame_buffer)
			+ (uint16_t) meshcore_get_path_byte_len(frame_buffer, frame_size);
}

/** Read a 16-bit Little Endian integer */
static uint16_t _get_u16_le(const uint8_t *p) {
	return (uint16_t) p[0] | ((uint16_t) p[1] << 8);
}

/** Read a 32-bit Little Endian integer */
static uint32_t _get_u32_le(const uint8_t *p) {
	return (uint32_t) p[0] | ((uint32_t) p[1] << 8) | ((uint32_t) p[2] << 16)
			| ((uint32_t) p[3] << 24);
}

/* ------------------------------------------------------------------------- */
/* Packet-level functions                                                     */
/* ------------------------------------------------------------------------- */

/** Check Size */
bool meshcore_check_valid_frame_size(const uint8_t *frame_buffer, const uint8_t frame_size) {
	if (frame_size < 2) {
		return false;
	}
	// header + optional transport codes + path_length
	return frame_size >= _path_offset(frame_buffer);
}

/** Check Frame */
bool meshcore_check_valid_frame(const uint8_t *frame_buffer, const uint8_t frame_size) {

	if (!meshcore_check_valid_frame_size(frame_buffer, frame_size)) {
		return false;
	}

	// reserved path hash size (0b11 ie 4 bytes) : dropped by the firmware
	if (meshcore_get_path_hash_size(frame_buffer, frame_size) > 3) {
		return false;
	}

	const uint8_t path_byte_len = meshcore_get_path_byte_len(frame_buffer, frame_size);

	// dropped by the firmware when larger than MAX_PATH_SIZE
	if (path_byte_len > MESHCORE_MAX_PATH_SIZE) {
		return false;
	}

	const uint16_t payload_offset = _payload_offset(frame_buffer, frame_size);

	// truncated path
	if (payload_offset > frame_size) {
		return false;
	}

	// dropped by the firmware when larger than MAX_PACKET_PAYLOAD
	if ((uint16_t)(frame_size - payload_offset) > MESHCORE_MAX_PACKET_PAYLOAD) {
		return false;
	}

	return true;
}

/** Get Header byte */
uint8_t meshcore_get_header(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return frame_buffer[0];
}

/** Get Route Type */
uint8_t meshcore_get_route_type(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return frame_buffer[0] & MESHCORE_PH_ROUTE_MASK;
}

/** Is Route Flood */
bool meshcore_is_route_flood(const uint8_t *frame_buffer, const uint8_t frame_size) {
	const uint8_t route_type = meshcore_get_route_type(frame_buffer, frame_size);
	return route_type == MESHCORE_ROUTE_TYPE_FLOOD
			|| route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD;
}

/** Is Route Direct */
bool meshcore_is_route_direct(const uint8_t *frame_buffer, const uint8_t frame_size) {
	const uint8_t route_type = meshcore_get_route_type(frame_buffer, frame_size);
	return route_type == MESHCORE_ROUTE_TYPE_DIRECT
			|| route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT;
}

/** Has Transport Codes */
bool meshcore_has_transport_codes(const uint8_t *frame_buffer, const uint8_t frame_size) {
	const uint8_t route_type = meshcore_get_route_type(frame_buffer, frame_size);
	return route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD
			|| route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT;
}

/** Get Transport Code 1 */
uint16_t meshcore_get_transport_code_1(const uint8_t *frame_buffer, const uint8_t frame_size) {
	if (!meshcore_has_transport_codes(frame_buffer, frame_size) || frame_size < 5) {
		return 0;
	}
	return _get_u16_le(frame_buffer + 1);
}

/** Get Transport Code 2 */
uint16_t meshcore_get_transport_code_2(const uint8_t *frame_buffer, const uint8_t frame_size) {
	if (!meshcore_has_transport_codes(frame_buffer, frame_size) || frame_size < 5) {
		return 0;
	}
	return _get_u16_le(frame_buffer + 3);
}

/** Get Payload Type */
uint8_t meshcore_get_payload_type(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return (frame_buffer[0] >> MESHCORE_PH_TYPE_SHIFT) & MESHCORE_PH_TYPE_MASK;
}

/** Get Payload Version */
uint8_t meshcore_get_payload_version(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return (frame_buffer[0] >> MESHCORE_PH_VER_SHIFT) & MESHCORE_PH_VER_MASK;
}

/** Get Path Hash Size */
uint8_t meshcore_get_path_hash_size(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return (frame_buffer[_path_length_offset(frame_buffer)] >> 6) + 1;
}

/** Get Path Hash Count */
uint8_t meshcore_get_path_hash_count(const uint8_t *frame_buffer, const uint8_t frame_size) {
	(void) frame_size;
	return frame_buffer[_path_length_offset(frame_buffer)] & 63;
}

/** Get Path Byte Length */
uint8_t meshcore_get_path_byte_len(const uint8_t *frame_buffer, const uint8_t frame_size) {
	return meshcore_get_path_hash_count(frame_buffer, frame_size)
			* meshcore_get_path_hash_size(frame_buffer, frame_size);
}

/** Get Path */
const uint8_t* meshcore_get_path(const uint8_t *frame_buffer, const uint8_t frame_size) {
	if (meshcore_get_path_byte_len(frame_buffer, frame_size) == 0) {
		return NULL;
	}
	return frame_buffer + _path_offset(frame_buffer);
}

/** Does the path contain node_hash */
bool meshcore_path_contains(const uint8_t *frame_buffer, const uint8_t frame_size, const uint8_t node_hash) {

	// only legacy 1-byte path hashes are compared
	if (meshcore_get_path_hash_size(frame_buffer, frame_size) != 1) {
		return false;
	}

	const uint8_t hash_count = meshcore_get_path_hash_count(frame_buffer, frame_size);
	const uint8_t *path = frame_buffer + _path_offset(frame_buffer);

	for (uint8_t i = 0; i < hash_count; i++) {
		if (path[i] == node_hash) {
			return true;
		}
	}
	return false;
}

/** Get Payload */
const uint8_t* meshcore_get_payload(const uint8_t *frame_buffer, const uint8_t frame_size, uint8_t *payload_size) {

	const uint16_t payload_offset = _payload_offset(frame_buffer, frame_size);

	if (payload_offset >= frame_size) {
		*payload_size = 0;
		return NULL;
	}

	*payload_size = frame_size - payload_offset;
	return frame_buffer + payload_offset;
}

/* ------------------------------------------------------------------------- */
/* Payload-level functions (payload version 1)                                */
/* ------------------------------------------------------------------------- */

/** Get Destination Hash */
uint8_t meshcore_get_destination_hash(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 1) ? payload[0] : 0;
}

/** Get Source Hash */
uint8_t meshcore_get_source_hash(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 2) ? payload[1] : 0;
}

/** Get Channel Hash */
uint8_t meshcore_get_channel_hash(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 1) ? payload[0] : 0;
}

/** Get Cipher MAC */
uint16_t meshcore_get_cipher_mac(const uint8_t *frame_buffer, const uint8_t frame_size) {

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	if (payload == NULL) {
		return 0;
	}

	const uint8_t payload_type = meshcore_get_payload_type(frame_buffer, frame_size);
	switch (payload_type) {
	case MESHCORE_PAYLOAD_TYPE_GRP_TXT:
	case MESHCORE_PAYLOAD_TYPE_GRP_DATA:
		// [channel_hash][MAC:2][ciphertext]
		return (payload_size >= 3) ? _get_u16_le(payload + 1) : 0;
	case MESHCORE_PAYLOAD_TYPE_ANON_REQ:
		// [dest_hash][pub_key:32][MAC:2][ciphertext]
		return (payload_size >= 35) ? _get_u16_le(payload + 33) : 0;
	default:
		// [dest_hash][src_hash][MAC:2][ciphertext] (REQ, RESPONSE, TXT_MSG, PATH)
		return (payload_size >= 4) ? _get_u16_le(payload + 2) : 0;
	}
}

/** Get ACK checksum */
uint32_t meshcore_get_ack_checksum(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 4) ? _get_u32_le(payload) : 0;
}

/** Get Advert Public Key */
const uint8_t* meshcore_get_advert_public_key(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 32) ? payload : NULL;
}

/** Get Advert Timestamp */
uint32_t meshcore_get_advert_timestamp(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 36) ? _get_u32_le(payload + 32) : 0;
}

/** Get Advert Signature */
const uint8_t* meshcore_get_advert_signature(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	return (payload != NULL && payload_size >= 100) ? payload + 36 : NULL;
}

/** Get Advert Appdata Flags */
uint8_t meshcore_get_advert_appdata_flags(const uint8_t *frame_buffer, const uint8_t frame_size) {
	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	// appdata starts after pubkey(32) + timestamp(4) + signature(64)
	return (payload != NULL && payload_size >= 101) ? payload[100] : 0;
}

/** Get Advert Location */
bool meshcore_get_advert_location(const uint8_t *frame_buffer, const uint8_t frame_size, int32_t *latitude, int32_t *longitude) {

	const uint8_t flags = meshcore_get_advert_appdata_flags(frame_buffer, frame_size);
	if ((flags & MESHCORE_ADV_LATLON_MASK) == 0) {
		return false;
	}

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	if (payload == NULL || payload_size < 101 + 8) {
		return false;
	}

	*latitude = (int32_t) _get_u32_le(payload + 101);
	*longitude = (int32_t) _get_u32_le(payload + 105);
	return true;
}

/** Get Advert Name */
uint8_t meshcore_get_advert_name(const uint8_t *frame_buffer, const uint8_t frame_size, char *name, const uint8_t name_maxlen) {

	if (name == NULL || name_maxlen == 0) {
		return 0;
	}

	name[0] = '\0';

	const uint8_t flags = meshcore_get_advert_appdata_flags(frame_buffer, frame_size);
	if ((flags & MESHCORE_ADV_NAME_MASK) == 0) {
		return 0;
	}

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	if (payload == NULL) {
		return 0;
	}

	// appdata : flags(1) + optional lat/lon(8) + optional feature1(2) + optional feature2(2) + name
	uint16_t name_offset = 101;
	if (flags & MESHCORE_ADV_LATLON_MASK) {
		name_offset += 8;
	}
	if (flags & MESHCORE_ADV_FEAT1_MASK) {
		name_offset += 2;
	}
	if (flags & MESHCORE_ADV_FEAT2_MASK) {
		name_offset += 2;
	}

	if (name_offset >= payload_size) {
		return 0;
	}

	uint8_t name_len = payload_size - name_offset;
	if (name_len > name_maxlen - 1) {
		name_len = name_maxlen - 1;
	}

	memcpy(name, payload + name_offset, name_len);
	name[name_len] = '\0';
	return name_len;
}

/* ------------------------------------------------------------------------- */
/* Encoding                                                                   */
/* ------------------------------------------------------------------------- */

/** Build packet */
bool meshcore_build_packet(uint8_t *frame_buffer, uint8_t *frame_size,
		const uint8_t route_type, const uint8_t payload_type, const uint8_t payload_version,
		const uint16_t transport_code_1, const uint16_t transport_code_2,
		const uint8_t *path, const uint8_t path_hash_count, const uint8_t path_hash_size,
		const uint8_t *payload, const uint8_t payload_size) {

	if (frame_buffer == NULL || frame_size == NULL) {
		return false;
	}

	if (route_type > MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT
			|| payload_type > MESHCORE_PAYLOAD_TYPE_RAW_CUSTOM
			|| payload_version > MESHCORE_PAYLOAD_VER_4) {
		return false;
	}

	if (path_hash_count > 63 || path_hash_size < 1 || path_hash_size > 3) {
		return false;
	}

	const uint16_t path_byte_len = (uint16_t) path_hash_count * (uint16_t) path_hash_size;
	if (path_byte_len > MESHCORE_MAX_PATH_SIZE) {
		return false;
	}

	if (payload_size > MESHCORE_MAX_PACKET_PAYLOAD) {
		return false;
	}

	const bool has_transport = (route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD
			|| route_type == MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT);

	const uint16_t total_size = 1 + (has_transport ? 4 : 0) + 1 + path_byte_len + payload_size;
	if (total_size > MESHCORE_MAX_TRANS_UNIT) {
		return false;
	}

	uint16_t offset = 0;

	// header : 0bVVPPPPRR
	frame_buffer[offset++] = (uint8_t)((payload_version << MESHCORE_PH_VER_SHIFT)
			| (payload_type << MESHCORE_PH_TYPE_SHIFT) | route_type);

	// transport codes (Little Endian)
	if (has_transport) {
		frame_buffer[offset++] = (uint8_t)(transport_code_1 & 0xFF);
		frame_buffer[offset++] = (uint8_t)(transport_code_1 >> 8);
		frame_buffer[offset++] = (uint8_t)(transport_code_2 & 0xFF);
		frame_buffer[offset++] = (uint8_t)(transport_code_2 >> 8);
	}

	// path_length : bits 6-7 store (hash_size - 1), bits 0-5 store hop count
	frame_buffer[offset++] = (uint8_t)(((path_hash_size - 1) << 6) | (path_hash_count & 63));

	// path
	if (path_byte_len > 0) {
		if (path == NULL) {
			return false;
		}
		memcpy(frame_buffer + offset, path, path_byte_len);
		offset += path_byte_len;
	}

	// payload
	if (payload_size > 0) {
		if (payload == NULL) {
			return false;
		}
		memcpy(frame_buffer + offset, payload, payload_size);
		offset += payload_size;
	}

	*frame_size = (uint8_t) offset;
	return true;
}

/* ------------------------------------------------------------------------- */
/* Repeater helpers                                                           */
/* ------------------------------------------------------------------------- */

/** Repeat a flood packet : append self_hash to the path */
bool meshcore_repeat_flood(const uint8_t *rx_frame_buffer, const uint8_t rx_frame_size,
		uint8_t *tx_frame_buffer, uint8_t *tx_frame_size, const uint8_t self_hash) {

	if (!meshcore_check_valid_frame(rx_frame_buffer, rx_frame_size)) {
		return false;
	}

	if (!meshcore_is_route_flood(rx_frame_buffer, rx_frame_size)) {
		return false;
	}

	// only legacy 1-byte path hashes are handled (as in firmware v1.12.0 and older)
	if (meshcore_get_path_hash_size(rx_frame_buffer, rx_frame_size) != 1) {
		return false;
	}

	// loop prevention : do not repeat a packet already repeated by this node
	if (meshcore_path_contains(rx_frame_buffer, rx_frame_size, self_hash)) {
		return false;
	}

	const uint8_t hash_count = meshcore_get_path_hash_count(rx_frame_buffer, rx_frame_size);
	if (hash_count >= 63 || (uint16_t)(hash_count + 1) > MESHCORE_MAX_PATH_SIZE) {
		return false;
	}

	if ((uint16_t)(rx_frame_size + 1) > MESHCORE_MAX_TRANS_UNIT) {
		return false;
	}

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(rx_frame_buffer, rx_frame_size, &payload_size);

	uint8_t new_path[MESHCORE_MAX_PATH_SIZE];
	const uint8_t *path = meshcore_get_path(rx_frame_buffer, rx_frame_size);
	if (path != NULL) {
		memcpy(new_path, path, hash_count);
	}
	new_path[hash_count] = self_hash;

	return meshcore_build_packet(tx_frame_buffer, tx_frame_size,
			meshcore_get_route_type(rx_frame_buffer, rx_frame_size),
			meshcore_get_payload_type(rx_frame_buffer, rx_frame_size),
			meshcore_get_payload_version(rx_frame_buffer, rx_frame_size),
			meshcore_get_transport_code_1(rx_frame_buffer, rx_frame_size),
			meshcore_get_transport_code_2(rx_frame_buffer, rx_frame_size),
			new_path, hash_count + 1, 1,
			payload, payload_size);
}

/** Repeat a direct packet : pop the first path hash if it is self_hash */
bool meshcore_repeat_direct(const uint8_t *rx_frame_buffer, const uint8_t rx_frame_size,
		uint8_t *tx_frame_buffer, uint8_t *tx_frame_size, const uint8_t self_hash) {

	if (!meshcore_check_valid_frame(rx_frame_buffer, rx_frame_size)) {
		return false;
	}

	if (!meshcore_is_route_direct(rx_frame_buffer, rx_frame_size)) {
		return false;
	}

	// only legacy 1-byte path hashes are handled (as in firmware v1.12.0 and older)
	if (meshcore_get_path_hash_size(rx_frame_buffer, rx_frame_size) != 1) {
		return false;
	}

	const uint8_t hash_count = meshcore_get_path_hash_count(rx_frame_buffer, rx_frame_size);
	if (hash_count == 0) {
		// packet has reached the end of its path : nothing to relay
		return false;
	}

	const uint8_t *path = meshcore_get_path(rx_frame_buffer, rx_frame_size);
	if (path == NULL || path[0] != self_hash) {
		// packet is not routed through this node
		return false;
	}

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(rx_frame_buffer, rx_frame_size, &payload_size);

	return meshcore_build_packet(tx_frame_buffer, tx_frame_size,
			meshcore_get_route_type(rx_frame_buffer, rx_frame_size),
			meshcore_get_payload_type(rx_frame_buffer, rx_frame_size),
			meshcore_get_payload_version(rx_frame_buffer, rx_frame_size),
			meshcore_get_transport_code_1(rx_frame_buffer, rx_frame_size),
			meshcore_get_transport_code_2(rx_frame_buffer, rx_frame_size),
			path + 1, hash_count - 1, 1,
			payload, payload_size);
}

/* ------------------------------------------------------------------------- */
/* Debug                                                                      */
/* ------------------------------------------------------------------------- */

/** Get Route Type as string */
const char* meshcore_get_route_type_str(const uint8_t route_type) {
	switch (route_type) {
	case MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD:
		return "TRANSPORT_FLOOD";
	case MESHCORE_ROUTE_TYPE_FLOOD:
		return "FLOOD";
	case MESHCORE_ROUTE_TYPE_DIRECT:
		return "DIRECT";
	case MESHCORE_ROUTE_TYPE_TRANSPORT_DIRECT:
		return "TRANSPORT_DIRECT";
	default:
		return "UNKNOWN";
	}
}

/** Get Payload Type as string */
const char* meshcore_get_payload_type_str(const uint8_t payload_type) {
	switch (payload_type) {
	case MESHCORE_PAYLOAD_TYPE_REQ:
		return "REQ";
	case MESHCORE_PAYLOAD_TYPE_RESPONSE:
		return "RESPONSE";
	case MESHCORE_PAYLOAD_TYPE_TXT_MSG:
		return "TXT_MSG";
	case MESHCORE_PAYLOAD_TYPE_ACK:
		return "ACK";
	case MESHCORE_PAYLOAD_TYPE_ADVERT:
		return "ADVERT";
	case MESHCORE_PAYLOAD_TYPE_GRP_TXT:
		return "GRP_TXT";
	case MESHCORE_PAYLOAD_TYPE_GRP_DATA:
		return "GRP_DATA";
	case MESHCORE_PAYLOAD_TYPE_ANON_REQ:
		return "ANON_REQ";
	case MESHCORE_PAYLOAD_TYPE_PATH:
		return "PATH";
	case MESHCORE_PAYLOAD_TYPE_TRACE:
		return "TRACE";
	case MESHCORE_PAYLOAD_TYPE_MULTIPART:
		return "MULTIPART";
	case MESHCORE_PAYLOAD_TYPE_CONTROL:
		return "CONTROL";
	case MESHCORE_PAYLOAD_TYPE_RAW_CUSTOM:
		return "RAW_CUSTOM";
	default:
		return "RESERVED";
	}
}

/** Print packet */
void meshcore_printf(const uint8_t *frame_buffer, const uint8_t frame_size) {

	if (!meshcore_check_valid_frame(frame_buffer, frame_size)) {
		printf("not a valid meshcore frame\n");
		return;
	}

	const uint8_t route_type = meshcore_get_route_type(frame_buffer, frame_size);
	const uint8_t payload_type = meshcore_get_payload_type(frame_buffer, frame_size);

	printf("route_type:      %s (%u)\n", meshcore_get_route_type_str(route_type), route_type);
	printf("payload_type:    %s (0x%02X)\n", meshcore_get_payload_type_str(payload_type), payload_type);
	printf("payload_version: %u\n", meshcore_get_payload_version(frame_buffer, frame_size));

	if (meshcore_has_transport_codes(frame_buffer, frame_size)) {
		printf("transport_code1: 0x%04X\n", meshcore_get_transport_code_1(frame_buffer, frame_size));
		printf("transport_code2: 0x%04X\n", meshcore_get_transport_code_2(frame_buffer, frame_size));
	}

	const uint8_t hash_count = meshcore_get_path_hash_count(frame_buffer, frame_size);
	const uint8_t hash_size = meshcore_get_path_hash_size(frame_buffer, frame_size);
	printf("path:            %u hop(s) x %u byte(s) :", hash_count, hash_size);
	const uint8_t *path = meshcore_get_path(frame_buffer, frame_size);
	if (path != NULL) {
		for (uint8_t j = 0; j < meshcore_get_path_byte_len(frame_buffer, frame_size); j++) {
			printf(" %02X", path[j]);
		}
	}
	printf("\n");

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame_buffer, frame_size, &payload_size);
	printf("payload_size:    %u\n", payload_size);

	if (payload == NULL) {
		return;
	}

	switch (payload_type) {

	case MESHCORE_PAYLOAD_TYPE_ACK:
		printf("ack_checksum:    0x%08" PRIX32 "\n", meshcore_get_ack_checksum(frame_buffer, frame_size));
		break;

	case MESHCORE_PAYLOAD_TYPE_ADVERT: {
		const uint8_t *public_key = meshcore_get_advert_public_key(frame_buffer, frame_size);
		if (public_key != NULL) {
			printf("public_key:      ");
			for (uint8_t j = 0; j < 32; j++) {
				printf("%02X", public_key[j]);
			}
			printf("\n");
			printf("node_hash:       0x%02X\n", public_key[0]);
		}
		printf("timestamp:       %" PRIu32 "\n", meshcore_get_advert_timestamp(frame_buffer, frame_size));
		const uint8_t flags = meshcore_get_advert_appdata_flags(frame_buffer, frame_size);
		printf("appdata_flags:   0x%02X\n", flags);
		int32_t latitude, longitude;
		if (meshcore_get_advert_location(frame_buffer, frame_size, &latitude, &longitude)) {
			printf("latitude:        %" PRIi32 " (x 1E-6 deg)\n", latitude);
			printf("longitude:       %" PRIi32 " (x 1E-6 deg)\n", longitude);
		}
		char name[64];
		if (meshcore_get_advert_name(frame_buffer, frame_size, name, sizeof(name)) > 0) {
			printf("name:            %s\n", name);
		}
		break;
	}

	case MESHCORE_PAYLOAD_TYPE_REQ:
	case MESHCORE_PAYLOAD_TYPE_RESPONSE:
	case MESHCORE_PAYLOAD_TYPE_TXT_MSG:
	case MESHCORE_PAYLOAD_TYPE_PATH:
		printf("dest_hash:       0x%02X\n", meshcore_get_destination_hash(frame_buffer, frame_size));
		printf("src_hash:        0x%02X\n", meshcore_get_source_hash(frame_buffer, frame_size));
		printf("cipher_mac:      0x%04X\n", meshcore_get_cipher_mac(frame_buffer, frame_size));
		printf("ciphertext_size: %u\n", (payload_size >= 4) ? payload_size - 4 : 0);
		break;

	case MESHCORE_PAYLOAD_TYPE_ANON_REQ:
		printf("dest_hash:       0x%02X\n", meshcore_get_destination_hash(frame_buffer, frame_size));
		if (payload_size >= 33) {
			printf("public_key:      ");
			for (uint8_t j = 0; j < 32; j++) {
				printf("%02X", payload[1 + j]);
			}
			printf("\n");
		}
		printf("cipher_mac:      0x%04X\n", meshcore_get_cipher_mac(frame_buffer, frame_size));
		printf("ciphertext_size: %u\n", (payload_size >= 35) ? payload_size - 35 : 0);
		break;

	case MESHCORE_PAYLOAD_TYPE_GRP_TXT:
	case MESHCORE_PAYLOAD_TYPE_GRP_DATA:
		printf("channel_hash:    0x%02X\n", meshcore_get_channel_hash(frame_buffer, frame_size));
		printf("cipher_mac:      0x%04X\n", meshcore_get_cipher_mac(frame_buffer, frame_size));
		printf("ciphertext_size: %u\n", (payload_size >= 3) ? payload_size - 3 : 0);
		break;

	case MESHCORE_PAYLOAD_TYPE_CONTROL: {
		const uint8_t sub_type = (payload[0] >> 4) & 0x0F;
		printf("control_subtype: 0x%02X\n", sub_type);
		if (sub_type == MESHCORE_CONTROL_DISCOVER_RESP && payload_size >= 6) {
			printf("node_type:       0x%02X\n", payload[0] & 0x0F);
			printf("snr:             %d (x 0.25 dB)\n", (int8_t) payload[1]);
			printf("tag:             0x%08" PRIX32 "\n", _get_u32_le(payload + 2));
			if (payload_size > 6) {
				printf("public_key:      ");
				for (uint8_t j = 6; j < payload_size; j++) {
					printf("%02X", payload[j]);
				}
				printf("\n");
			}
		} else if (sub_type == MESHCORE_CONTROL_DISCOVER_REQ && payload_size >= 6) {
			printf("prefix_only:     %s\n", (payload[0] & 0x01) ? "true" : "false");
			printf("type_filter:     0x%02X\n", payload[1]);
			printf("tag:             0x%08" PRIX32 "\n", _get_u32_le(payload + 2));
		}
		break;
	}

	default:
		break;
	}

	printf("payload:         ");
	for (uint8_t j = 0; j < payload_size; j++) {
		printf("%02X ", payload[j]);
	}
	printf("\n");
}
