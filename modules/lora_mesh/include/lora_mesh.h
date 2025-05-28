/*
 * Copyright (C) 2020-2025 Université Grenoble Alpes
 */

/*
 * Utils for the LoRa Mesh
 *
 * @link https://www.chirpstack.io/docs/chirpstack-gateway-mesh/protocol.html#downlink-payload-format
 */

#ifndef LORA_MESH_H
#define LORA_MESH_H

#include <inttypes.h>
#include <stdbool.h>

#define LORAWAN_PHYPAYLOAD_LEN					(256U)

#define LORAMESH_MTYPE_PROPRIETARY				(0b111)
#define LORAMESH_PAYLOAD_TYPE_UPLINK			(0b11)
#define LORAMESH_PAYLOAD_TYPE_DOWNLINK			(0b01)
#define LORAMESH_PAYLOAD_TYPE_RELAYHEARTRATE	(0b10)

// Extension proposal
#define LORAMESH_PAYLOAD_TYPE_STATS				(0b00)

/**
 * MeshLoRa MHDR
 */
struct __attribute__((packed)) MeshLoRa_MHDR {

	unsigned int mtype :3;

	unsigned int payload_type :2;

	unsigned int hop_count :3;
};

typedef struct MeshLoRa_MHDR MeshLoRa_MHDR_t;

/**
 * MeshLoRa Uplink
 */
struct __attribute__((packed)) MeshLoRa_Uplink {

	unsigned int mtype :3;

	unsigned int payload_type :2;

	unsigned int hop_count :3;

	unsigned int uplink_id :12;

	unsigned int datarate :4;

	uint8_t rssi;

	uint8_t snr;

	uint8_t channel;

	uint32_t relay_id;

	uint8_t lora_mesh_phypayload[LORAWAN_PHYPAYLOAD_LEN - 14U];

};

typedef struct MeshLoRa_Uplink MeshLoRa_Uplink_t;

/**
 * MeshLoRa Downlink
 */
struct __attribute__((packed)) MeshLoRa_Downlink {

	unsigned int mtype :3;

	unsigned int payload_type :2;

	unsigned int hop_count :3;

	unsigned int uplink_id :12;

	unsigned int downlink_datarate :4;

	unsigned int downlink_frequency :24;

	unsigned int tx_power :4;

	unsigned int delay :4;

	uint32_t relay_id;

	uint8_t lora_mesh_phypayload[LORAWAN_PHYPAYLOAD_LEN - 15U];

};

typedef struct MeshLoRa_Downlink MeshLoRa_Downlink_t;

/**
 * MeshLoRa Relay Heartbeat
 */
struct __attribute__((packed)) MeshLoRa_RelayHeartbeat {

	unsigned int mtype :3;

	unsigned int payload_type :2;

	unsigned int hop_count :3;

	uint32_t timestamp;

	uint32_t relay_id;

	uint8_t relay_path[28U];

};

typedef struct MeshLoRa_RelayHeartbeat MeshLoRa_RelayHeartbeat_t;


/**
 * MeshLoRa Stats (Extension proposal)
 */
struct __attribute__((packed)) MeshLoRa_Stats {

	unsigned int mtype :3;

	unsigned int payload_type :2;

	unsigned int hop_count :3;

	uint32_t relay_id;

	uint16_t stats_id; // LSB of uint32_t

	uint16_t uptime; // LSB of uint32_t

	uint16_t rx; // Number of frames since boot. LSB of uint32_t

	uint16_t rx_ok; // Number of frames (CRC_OK) since boot. LSB of uint32_t

	uint16_t rx_relay; // Number of relayed frames (uplink) since boot. LSB of uint32_t

	uint16_t tx; // Number of tx frames since boot. LSB of uint32_t

	uint16_t tx_relay; // Number of relayed tx frames (downlink) since boot. LSB of uint32_t
};

typedef struct MeshLoRa_Stats MeshLoRa_Stats_t;


/** Check valid mesh frame */
bool lora_mesh_check_valid_frame(const uint8_t *frame_buffer,
		const uint8_t size);

/** Get Type */
uint8_t lora_mesh_get_type(const uint8_t *frame_buffer, const uint8_t size);

/** Get frame MIC */
uint32_t lora_mesh_get_mic(const uint8_t *frame_buffer, const uint8_t size);

/** Check frame MIC */
bool lora_mesh_check_mic(const uint8_t *frame_buffer, const uint8_t size,
		const uint8_t *aes_key);

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_uplink(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRa Mesh Join Frame */
bool lora_mesh_is_downlink(const uint8_t *frame_buffer, const uint8_t size);

/** Is LoRa Mesh Relay Heartbeat */
bool lora_mesh_is_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size);

/** Get LoRaWAN PhyPayload */
const uint8_t* lora_mesh_get_payload(const uint8_t *frame_buffer, const uint8_t size,
		uint8_t *lorawan_phypayload_size);

/** Get LoRaWAN PhyPayload size */
uint8_t lora_mesh_get_payload_size(const uint8_t *frame_buffer,
		const uint8_t size);

void lora_mesh_printf_uplink(const uint8_t *frame_buffer, const uint8_t size);

void lora_mesh_printf_downlink(const uint8_t *frame_buffer, const uint8_t size);

void lora_mesh_printf_relay_heartbeat(const uint8_t *frame_buffer,
		const uint8_t size);

void lora_mesh_printf_frame(const uint8_t *frame_buffer, const uint8_t size);


bool lora_mesh_build_uplink(
	uint8_t *frame_buffer,
	uint8_t *size,
	const uint8_t hop_count,
	const uint16_t uplink_id,
	const uint8_t datarate,
	const uint8_t rssi,
	const uint8_t snr,
	const uint8_t channel,
	const uint32_t relay_id,
	const uint8_t *phypayload,
	const uint8_t phypayload_size,
	const uint8_t *signing_key
);


#endif
