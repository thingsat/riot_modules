/* Generated at Thu Jul  9 12:54:40 UTC 2026 by unknown from unvalid_packets.txt by generate_inc.sh -- DO NOT EDIT */

#ifndef MESHCORE_PACKET_LIST_T_DEFINED
#define MESHCORE_PACKET_LIST_T_DEFINED
typedef struct packet_list { unsigned int len; const unsigned char *packet; } packet_list_t;
#endif

// FF
static const unsigned char unvalid_packet_0[] = {
  0xff, 
};

// 000102
static const unsigned char unvalid_packet_1[] = {
  0x00, 0x01, 0x02, 
};

// 1141
static const unsigned char unvalid_packet_2[] = {
  0x11, 0x41, 
};

// 110208
static const unsigned char unvalid_packet_3[] = {
  0x11, 0x02, 0x08, 
};

static const packet_list_t unvalid_packet_list[] = {
  { (unsigned int) sizeof(unvalid_packet_0), unvalid_packet_0 },
  { (unsigned int) sizeof(unvalid_packet_1), unvalid_packet_1 },
  { (unsigned int) sizeof(unvalid_packet_2), unvalid_packet_2 },
  { (unsigned int) sizeof(unvalid_packet_3), unvalid_packet_3 },
};

static const unsigned int unvalid_packet_list_len = 4u;
