/*
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Simple test of the meshcore_utils module
 *
 * Sample packets from https://analyzer.letsmesh.net/packets (see packets.txt)
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "meshcore_utils.h"

#include "shell.h"

static unsigned int _failures = 0;

#define CHECK(cond, msg) do { \
	if (cond) { \
		printf("PASS: %s\n", msg); \
	} else { \
		printf("FAIL: %s\n", msg); \
		_failures++; \
	} \
} while (0)

/*
 * Sample packets from https://analyzer.letsmesh.net/packets (see packets.txt)
 */

// DIRECT + CONTROL (DISCOVER_RESP), zero-hop
static const char packet1_hex[] =
		"2E00921DEAC837BDF829C488E0DB5DEC2A20002A1E821A632CCD298E81B446F83AEDEC614091C93D";

// FLOOD + GRP_TXT, 1 hop (path = 08)
static const char packet2_hex[] =
		"15010881A3D5D44009AC7C9BB87E7B8179F591033D0052C545CB94A470F7F1B8F325544041B8044E83445F939B390CDC76B0C35344D7";

// FLOOD + ADVERT, 1 hop (path = 08), appdata with location and name
static const char packet3_hex[] =
		"1101089DE04AA7CC71536C0103FA121A6D46765C0E8CC88D85136FE741E538D0"
		"99962131E24F6ADC26019E167B015EF7F597B706D23F8CA8F84EA43A68C44284"
		"EEE4F9C858A3C52BB6A813BCDED1FA4C2FCFFDB8BC6B33A3199E765A09EFAC9E"
		"F92109B4A3AC08921AD3CA0201B53AFA32357468205374205320262031387468"
		"204176652053";

/*
 * Generated packet corpus (see generate_inc.sh) :
 * packets.inc from packets.txt (valid frames),
 * unvalid_packets.inc from unvalid_packets.txt (frames that must be rejected).
 */
#include "packets.inc"
#include "unvalid_packets.inc"

static void _test_packet_corpus(void) {
	puts("");
	printf("---------- packets.inc corpus : %u valid, %u unvalid ----------\n",
			packet_list_len, unvalid_packet_list_len);

	char msg[64];
	for (unsigned int i = 0; i < packet_list_len; i++) {
		snprintf(msg, sizeof(msg), "packets.inc[%u] : valid frame", i);
		CHECK(meshcore_check_valid_frame(packet_list[i].packet,
				(uint8_t) packet_list[i].len), msg);
		meshcore_printf(packet_list[i].packet, (uint8_t) packet_list[i].len);
	}

	for (unsigned int i = 0; i < unvalid_packet_list_len; i++) {
		snprintf(msg, sizeof(msg), "unvalid_packets.inc[%u] : rejected", i);
		CHECK(!meshcore_check_valid_frame(unvalid_packet_list[i].packet,
				(uint8_t) unvalid_packet_list[i].len), msg);
	}
}

static int _hex2bin(const char *hex, uint8_t *bin, const unsigned int bin_maxlen) {
	const unsigned int hexlen = strlen(hex);
	if (hexlen % 2 != 0 || hexlen / 2 > bin_maxlen) {
		return -1;
	}
	for (unsigned int i = 0; i < hexlen / 2; i++) {
		unsigned int byte;
		if (sscanf(hex + 2 * i, "%2x", &byte) != 1) {
			return -1;
		}
		bin[i] = (uint8_t) byte;
	}
	return (int) (hexlen / 2);
}

static void _test_decode_packet1(void) {

	puts("");
	puts("---------- packet 1 : DIRECT + CONTROL (DISCOVER_RESP) ----------");

	uint8_t frame[MESHCORE_MAX_TRANS_UNIT];
	const int size = _hex2bin(packet1_hex, frame, sizeof(frame));
	CHECK(size > 0, "packet 1 : hex decoding");

	CHECK(meshcore_check_valid_frame(frame, size), "packet 1 : valid frame");
	CHECK(meshcore_get_route_type(frame, size) == MESHCORE_ROUTE_TYPE_DIRECT,
			"packet 1 : route type is DIRECT");
	CHECK(meshcore_get_payload_type(frame, size) == MESHCORE_PAYLOAD_TYPE_CONTROL,
			"packet 1 : payload type is CONTROL");
	CHECK(meshcore_get_payload_version(frame, size) == MESHCORE_PAYLOAD_VER_1,
			"packet 1 : payload version is 1");
	CHECK(!meshcore_has_transport_codes(frame, size), "packet 1 : no transport codes");
	CHECK(meshcore_get_path_hash_count(frame, size) == 0, "packet 1 : zero-hop path");

	uint8_t payload_size;
	const uint8_t *payload = meshcore_get_payload(frame, size, &payload_size);
	CHECK(payload != NULL && payload_size == 38, "packet 1 : payload size is 38");
	CHECK(payload != NULL && ((payload[0] >> 4) & 0x0F) == MESHCORE_CONTROL_DISCOVER_RESP,
			"packet 1 : control subtype is DISCOVER_RESP");

	meshcore_printf(frame, size);
}

static void _test_decode_packet2(void) {

	puts("");
	puts("---------- packet 2 : FLOOD + GRP_TXT ----------");

	uint8_t frame[MESHCORE_MAX_TRANS_UNIT];
	const int size = _hex2bin(packet2_hex, frame, sizeof(frame));
	CHECK(size > 0, "packet 2 : hex decoding");

	CHECK(meshcore_check_valid_frame(frame, size), "packet 2 : valid frame");
	CHECK(meshcore_get_route_type(frame, size) == MESHCORE_ROUTE_TYPE_FLOOD,
			"packet 2 : route type is FLOOD");
	CHECK(meshcore_get_payload_type(frame, size) == MESHCORE_PAYLOAD_TYPE_GRP_TXT,
			"packet 2 : payload type is GRP_TXT");
	CHECK(meshcore_get_path_hash_count(frame, size) == 1, "packet 2 : 1-hop path");
	CHECK(meshcore_get_path_hash_size(frame, size) == 1, "packet 2 : 1-byte path hashes");

	const uint8_t *path = meshcore_get_path(frame, size);
	CHECK(path != NULL && path[0] == 0x08, "packet 2 : path[0] is 0x08");
	CHECK(meshcore_path_contains(frame, size, 0x08), "packet 2 : path contains 0x08");
	CHECK(!meshcore_path_contains(frame, size, 0xC5), "packet 2 : path does not contain 0xC5");

	CHECK(meshcore_get_channel_hash(frame, size) == 0x81, "packet 2 : channel hash is 0x81");
	CHECK(meshcore_get_cipher_mac(frame, size) == 0xD5A3, "packet 2 : cipher MAC is 0xD5A3");

	meshcore_printf(frame, size);
}

static void _test_decode_packet3(void) {

	puts("");
	puts("---------- packet 3 : FLOOD + ADVERT ----------");

	uint8_t frame[MESHCORE_MAX_TRANS_UNIT];
	const int size = _hex2bin(packet3_hex, frame, sizeof(frame));
	CHECK(size > 0, "packet 3 : hex decoding");

	CHECK(meshcore_check_valid_frame(frame, size), "packet 3 : valid frame");
	CHECK(meshcore_get_route_type(frame, size) == MESHCORE_ROUTE_TYPE_FLOOD,
			"packet 3 : route type is FLOOD");
	CHECK(meshcore_get_payload_type(frame, size) == MESHCORE_PAYLOAD_TYPE_ADVERT,
			"packet 3 : payload type is ADVERT");
	CHECK(meshcore_get_path_hash_count(frame, size) == 1, "packet 3 : 1-hop path");

	const uint8_t *public_key = meshcore_get_advert_public_key(frame, size);
	CHECK(public_key != NULL && public_key[0] == 0x9D, "packet 3 : node hash is 0x9D");

	const uint8_t flags = meshcore_get_advert_appdata_flags(frame, size);
	CHECK((flags & MESHCORE_ADV_NAME_MASK) != 0, "packet 3 : advert has a name");

	char name[64];
	const uint8_t name_len = meshcore_get_advert_name(frame, size, name, sizeof(name));
	CHECK(name_len > 0, "packet 3 : advert name decoded");
	if (name_len > 0) {
		printf("INFO: advert name : \"%s\"\n", name);
	}

	meshcore_printf(frame, size);
}

static void _test_encode_decode(void) {

	puts("");
	puts("---------- encode / decode round trip ----------");

	// build a flood ACK with a 2-hop path
	const uint8_t path[] = { 0x11, 0x22 };
	const uint8_t ack_payload[] = { 0xDE, 0xAD, 0xBE, 0xEF };

	uint8_t frame[MESHCORE_MAX_TRANS_UNIT];
	uint8_t frame_size;

	bool res = meshcore_build_packet(frame, &frame_size,
			MESHCORE_ROUTE_TYPE_FLOOD, MESHCORE_PAYLOAD_TYPE_ACK, MESHCORE_PAYLOAD_VER_1,
			0, 0,
			path, 2, 1,
			ack_payload, sizeof(ack_payload));
	CHECK(res, "encode : build flood ACK packet");
	CHECK(frame_size == 1 + 1 + 2 + 4, "encode : frame size is 8");

	CHECK(meshcore_check_valid_frame(frame, frame_size), "decode : valid frame");
	CHECK(meshcore_get_route_type(frame, frame_size) == MESHCORE_ROUTE_TYPE_FLOOD,
			"decode : route type is FLOOD");
	CHECK(meshcore_get_payload_type(frame, frame_size) == MESHCORE_PAYLOAD_TYPE_ACK,
			"decode : payload type is ACK");
	CHECK(meshcore_get_path_hash_count(frame, frame_size) == 2, "decode : 2-hop path");
	CHECK(meshcore_get_ack_checksum(frame, frame_size) == 0xEFBEADDE,
			"decode : ACK checksum (Little Endian)");

	// build a transport flood packet and check the transport codes
	res = meshcore_build_packet(frame, &frame_size,
			MESHCORE_ROUTE_TYPE_TRANSPORT_FLOOD, MESHCORE_PAYLOAD_TYPE_ACK, MESHCORE_PAYLOAD_VER_1,
			0xCAFE, 0xBABE,
			NULL, 0, 1,
			ack_payload, sizeof(ack_payload));
	CHECK(res, "encode : build transport flood ACK packet");
	CHECK(meshcore_has_transport_codes(frame, frame_size), "decode : has transport codes");
	CHECK(meshcore_get_transport_code_1(frame, frame_size) == 0xCAFE,
			"decode : transport code 1 is 0xCAFE");
	CHECK(meshcore_get_transport_code_2(frame, frame_size) == 0xBABE,
			"decode : transport code 2 is 0xBABE");

	// oversized payload must be rejected
	uint8_t big_payload[MESHCORE_MAX_PACKET_PAYLOAD + 1];
	memset(big_payload, 0xAA, sizeof(big_payload));
	res = meshcore_build_packet(frame, &frame_size,
			MESHCORE_ROUTE_TYPE_FLOOD, MESHCORE_PAYLOAD_TYPE_RAW_CUSTOM, MESHCORE_PAYLOAD_VER_1,
			0, 0,
			NULL, 0, 1,
			big_payload, sizeof(big_payload));
	CHECK(!res, "encode : oversized payload is rejected");
}

static void _test_repeat(void) {

	puts("");
	puts("---------- repeater helpers ----------");

	const uint8_t self_hash = 0xC5;

	// flood repeat : append self hash to packet 2
	uint8_t rx[MESHCORE_MAX_TRANS_UNIT];
	const int rx_size = _hex2bin(packet2_hex, rx, sizeof(rx));

	uint8_t tx[MESHCORE_MAX_TRANS_UNIT];
	uint8_t tx_size;

	bool res = meshcore_repeat_flood(rx, rx_size, tx, &tx_size, self_hash);
	CHECK(res, "repeat flood : packet 2 is repeated");
	CHECK(tx_size == rx_size + 1, "repeat flood : frame grows by 1 byte");
	CHECK(meshcore_get_path_hash_count(tx, tx_size) == 2, "repeat flood : path has 2 hops");

	const uint8_t *path = meshcore_get_path(tx, tx_size);
	CHECK(path != NULL && path[0] == 0x08 && path[1] == self_hash,
			"repeat flood : self hash appended to path");

	// payload must be unchanged
	uint8_t rx_payload_size, tx_payload_size;
	const uint8_t *rx_payload = meshcore_get_payload(rx, rx_size, &rx_payload_size);
	const uint8_t *tx_payload = meshcore_get_payload(tx, tx_size, &tx_payload_size);
	CHECK(rx_payload != NULL && tx_payload != NULL
			&& rx_payload_size == tx_payload_size
			&& memcmp(rx_payload, tx_payload, rx_payload_size) == 0,
			"repeat flood : payload is unchanged");

	// loop prevention : repeating our own retransmission must fail
	uint8_t tx2[MESHCORE_MAX_TRANS_UNIT];
	uint8_t tx2_size;
	res = meshcore_repeat_flood(tx, tx_size, tx2, &tx2_size, self_hash);
	CHECK(!res, "repeat flood : loop prevention (self hash already in path)");

	// direct repeat : pop self hash
	const uint8_t direct_path[] = { self_hash, 0x22 };
	const uint8_t dummy_payload[] = { 0x01, 0x02, 0x03, 0x04 };
	uint8_t direct[MESHCORE_MAX_TRANS_UNIT];
	uint8_t direct_size;
	res = meshcore_build_packet(direct, &direct_size,
			MESHCORE_ROUTE_TYPE_DIRECT, MESHCORE_PAYLOAD_TYPE_TXT_MSG, MESHCORE_PAYLOAD_VER_1,
			0, 0,
			direct_path, 2, 1,
			dummy_payload, sizeof(dummy_payload));
	CHECK(res, "repeat direct : build direct packet");

	res = meshcore_repeat_direct(direct, direct_size, tx, &tx_size, self_hash);
	CHECK(res, "repeat direct : packet routed through this node is repeated");
	CHECK(meshcore_get_path_hash_count(tx, tx_size) == 1, "repeat direct : path has 1 hop left");
	path = meshcore_get_path(tx, tx_size);
	CHECK(path != NULL && path[0] == 0x22, "repeat direct : self hash popped from path");

	// direct repeat : packet not routed through this node must fail
	res = meshcore_repeat_direct(direct, direct_size, tx, &tx_size, 0x99);
	CHECK(!res, "repeat direct : packet not routed through this node is skipped");
}

static int meshcore_decode_cmd(int argc, char *argv[]) {

	if (argc != 2) {
		printf("usage: %s <hex_frame>\n", argv[0]);
		return EXIT_FAILURE;
	}

	uint8_t frame[MESHCORE_MAX_TRANS_UNIT];
	const int size = _hex2bin(argv[1], frame, sizeof(frame));
	if (size <= 0) {
		puts("ERROR: bad hex frame");
		return EXIT_FAILURE;
	}

	meshcore_printf(frame, size);
	return EXIT_SUCCESS;
}

static const shell_command_t shell_commands[] = {
	{ "mcdecode", "decode a MeshCore frame (hex)", meshcore_decode_cmd },
	{ NULL, NULL, NULL }
};

int main(void) {

	_test_decode_packet1();
	_test_decode_packet2();
	_test_decode_packet3();
	_test_packet_corpus();
	_test_encode_decode();
	_test_repeat();

	puts("");
	if (_failures == 0) {
		puts("ALL TESTS PASSED");
	} else {
		printf("%u TEST(S) FAILED\n", _failures);
	}

	puts("");
	puts("=========================================");
	puts("Copyright (c) 2021-2025 UGA CSUG LIG");
	puts("=========================================");

	/* start shell */
	puts("All up, running the shell now");
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

	return EXIT_SUCCESS;
}
