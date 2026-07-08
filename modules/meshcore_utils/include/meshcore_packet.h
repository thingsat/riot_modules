/*
 * Copyright (C) 2026 Université Grenoble Alpes - CSUG - LIG
 */

/*
 * Utils for MeshCore mesh network
 */

#ifndef _MESHCORE_UTILS_PACKET_H_
#define _MESHCORE_UTILS_PACKET_H_

// From https://github.com/meshcore-dev/MeshCore/blob/main/src/Packet.h

#include "meshcore_constants.h"

// Packet::header values
#define PH_ROUTE_MASK     0x03   // 2-bits
#define PH_TYPE_SHIFT        2
#define PH_TYPE_MASK      0x0F   // 4-bits
#define PH_VER_SHIFT         6
#define PH_VER_MASK       0x03   // 2-bits

#define ROUTE_TYPE_TRANSPORT_FLOOD   0x00    // flood mode + transport codes
#define ROUTE_TYPE_FLOOD             0x01    // flood mode, needs 'path' to be built up (max 64 bytes)
#define ROUTE_TYPE_DIRECT            0x02    // direct route, 'path' is supplied
#define ROUTE_TYPE_TRANSPORT_DIRECT  0x03    // direct route + transport codes

#define PAYLOAD_TYPE_REQ         0x00    // request (prefixed with dest/src hashes, MAC) (enc data: timestamp, blob)
#define PAYLOAD_TYPE_RESPONSE    0x01    // response to REQ or ANON_REQ (prefixed with dest/src hashes, MAC) (enc data: timestamp, blob)
#define PAYLOAD_TYPE_TXT_MSG     0x02    // a plain text message (prefixed with dest/src hashes, MAC) (enc data: timestamp, text)
#define PAYLOAD_TYPE_ACK         0x03    // a simple ack
#define PAYLOAD_TYPE_ADVERT      0x04    // a node advertising its Identity
#define PAYLOAD_TYPE_GRP_TXT     0x05    // an (unverified) group text message (prefixed with channel hash, MAC) (enc data: timestamp, "name: msg")
#define PAYLOAD_TYPE_GRP_DATA    0x06    // an (unverified) group datagram (prefixed with channel hash, MAC) (enc data: data_type(uint16), data_len, blob)
#define PAYLOAD_TYPE_ANON_REQ    0x07    // generic request (prefixed with dest_hash, ephemeral pub_key, MAC) (enc data: ...)
#define PAYLOAD_TYPE_PATH        0x08    // returned path (prefixed with dest/src hashes, MAC) (enc data: path, extra)
#define PAYLOAD_TYPE_TRACE       0x09    // trace a path, collecting SNI for each hop
#define PAYLOAD_TYPE_MULTIPART   0x0A    // packet is one of a set of packets
#define PAYLOAD_TYPE_CONTROL     0x0B    // a control/discovery packet
//...
#define PAYLOAD_TYPE_RAW_CUSTOM   0x0F    // custom packet as raw bytes, for applications with custom encryption, payloads, etc

#define PAYLOAD_VER_1       0x00   // 1-byte src/dest hashes, 2-byte MAC
#define PAYLOAD_VER_2       0x01   // FUTURE (eg. 2-byte hashes, 4-byte MAC ??)
#define PAYLOAD_VER_3       0x02   // FUTURE
#define PAYLOAD_VER_4       0x03   // FUTURE

typedef struct Packet {

  uint8_t header;
  uint16_t payload_len, path_len;
  uint16_t transport_codes[2];
  uint8_t path[MAX_PATH_SIZE];
  uint8_t payload[MAX_PACKET_PAYLOAD];

  uint8_t _sf;
  int8_t _snr;
  int16_t _rssi;

} Packet_t;

  /**
   * \brief  save entire packet as a blob
   * \param dest  (OUT) destination buffer (assumed to be MAX_MTU_SIZE)
   * \returns  the packet length
   */
  const uint8_t Packet_writeTo(const Packet_t* packet, uint8_t dest[]);

  /**
   * \brief  restore this packet from a blob (as created using writeTo())
   * \param  src  (IN) buffer containing blob
   * \param  len  the packet length (as returned by writeTo())
   * \returns  true if successful, else false 
   */
  bool readFrom(Packet_t* packet, const uint8_t src[], uint8_t len);

#endif