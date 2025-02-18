/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/*
 * Utils for the LoRaWAN MAC
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fmt.h"

#include "lorawan_printf.h"


/** Get Type */
bool lorawan_check_valid_frame_size(const uint8_t *frame_buffer,
		const uint8_t size) {
	if (size < 1) {
		return false;
	}

	if (lorawan_get_version(frame_buffer, size) != 0) {
		return false;
	}

	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_PROPRIETARY: {
		return false;
	}
	case FRAME_TYPE_REJOIN:
	case FRAME_TYPE_JOIN_REQ: {
		return size >= (1 + 3 + 3 + 4 + 1 + 1 + 4)
				&& size <= (1 + 3 + 3 + 4 + 1 + 1 + 4 + 16);
	}
	case FRAME_TYPE_JOIN_ACCEPT: {
		return size == (1 + 8 + 8 + 2 + 4);
	}
	default: {
		// All Data frame
		return size >= (1 + 4 + 1 + 2 + 4);
	}
	}
}

/** Get Type */
uint8_t lorawan_get_type(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	return frame_buffer[0] >> 5;
}

/** Is LoRaWAN Join Frame */
bool lorawan_is_joinframe(const uint8_t *frame_buffer, const uint8_t size) {
	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_JOIN_REQ:
	case FRAME_TYPE_REJOIN: {
		return true;
	}
	default: {
		return false;
	}
	}
}

/** Is LoRaWAN Data Frame */
bool lorawan_is_dataframe(const uint8_t *frame_buffer, const uint8_t size) {
	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_DATA_CONFIRMED_DOWN:
	case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
	case FRAME_TYPE_DATA_CONFIRMED_UP:
	case FRAME_TYPE_DATA_UNCONFIRMED_UP: {
		return true;
	}
	default: {
		return false;
	}
	}
}

/** Is Confirmed LoRaWAN Data Frame */
bool lorawan_is_confirmed(const uint8_t *frame_buffer, const uint8_t size) {
	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_DATA_CONFIRMED_DOWN:
	case FRAME_TYPE_DATA_CONFIRMED_UP: {
		return true;
	}
	default: {
		return false;
	}
	}
}

/** Is Unconfirmed LoRaWAN Data Frame */
bool lorawan_is_unconfirmed(const uint8_t *frame_buffer, const uint8_t size) {
	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
	case FRAME_TYPE_DATA_UNCONFIRMED_UP: {
		return true;
	}
	default: {
		return false;
	}
	}
}



/** Is LoRaWAN Uplink */
bool lorawan_is_uplink(const uint8_t *frame_buffer, const uint8_t size) {
	switch (lorawan_get_type(frame_buffer, size)) {
	case FRAME_TYPE_JOIN_REQ:
	case FRAME_TYPE_REJOIN:
	case FRAME_TYPE_DATA_CONFIRMED_UP:
	case FRAME_TYPE_DATA_UNCONFIRMED_UP: {
		return true;
	}
	default: {
		return false;
	}
	}
}

/** Is LoRaWAN Size  */
// TODO bool lorawan_is_lorawan_size(const uint8_t *frame_buffer, const uint8_t size, const uint8_t datarate)
/** Get Version */
uint8_t lorawan_get_version(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	return frame_buffer[0] & 0b00011111;
}

/** Get frame MIC */
uint32_t lorawan_get_mic(const uint8_t *frame_buffer, const uint8_t size) {
	uint32_t mic = 0;
	mic = frame_buffer[size - 4];
	mic |= frame_buffer[size - 3] << 8;
	mic |= frame_buffer[size - 2] << 16;
	mic |= frame_buffer[size - 1] << 24;
	return mic;
}

/** Get FHDR - FCnt */
uint16_t lorawan_get_fcnt(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint16_t fcnt = 0;
	fcnt = frame_buffer[6];
	fcnt |= frame_buffer[7] << 8;
	return fcnt;
}

/** Get FHDR - DevAddr */
uint32_t lorawan_get_devaddr(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint32_t devaddr = 0;
	devaddr = frame_buffer[1];
	devaddr |= frame_buffer[2] << 8;
	devaddr |= frame_buffer[3] << 16;
	devaddr |= frame_buffer[4] << 24;
	return devaddr;
}

/** Get FHDR - FCtrl -ADR */
bool lorawan_get_fctrl_adr(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint8_t fctrl = frame_buffer[5] >> 7;
	return (fctrl & 0x01) == 1;
}

/** Get FHDR - FCtrl - ACK */
bool lorawan_get_fctrl_ack(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint8_t fctrl = frame_buffer[5] >> 5;
	return (fctrl & 0x01) == 1;
}

/** Get FHDR - FCtrl - Pending */
bool lorawan_get_fctrl_pending(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint8_t fctrl = frame_buffer[5] >> 4;
	return (fctrl & 0x01) == 1;
}

/** Get FHDR - FCtrl - fOptLen */
inline bool lorawan_get_fctrl_fOptLen(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	return frame_buffer[5] & 0x0F;
}

/** Get FPort */
uint8_t lorawan_get_fport(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	const uint8_t fOptLen = lorawan_get_fctrl_fOptLen(frame_buffer, size);
	if (size == (1 + 4 + 1 + 2 + fOptLen + 4)) {
		// No Port, No Payload
		return 0xFF;
	} else {
		return frame_buffer[1 + 4 + 1 + 2 + fOptLen];
	}
}

/** Get FPayload */
uint8_t* lorawan_get_fpayload(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	const uint8_t fOptLen = lorawan_get_fctrl_fOptLen(frame_buffer, size);
	if (size == (1 + 4 + 1 + 2 + fOptLen + 4)) {
		// No Port, No Payload
		return NULL;
	} else {
		return (uint8_t*) (frame_buffer + (1 + 4 + 1 + 2 + fOptLen + 1));
	}
}

/** Get FPayload size */
uint8_t lorawan_get_fpayload_size(const uint8_t *frame_buffer,
		const uint8_t size) {
	(void) size;
	const uint8_t fOptLen = lorawan_get_fctrl_fOptLen(frame_buffer, size);
	if (size <= (1 + 4 + 1 + 2 + fOptLen + 4)) {
		// No Port, No Payload
		// REMARK : not <= instead of == since fOptLen may be not correct
		return 0;
	} else {
		return size - (1 + 4 + 1 + 2 + fOptLen + 1 + 4);
	}
}

void lorawan_printf_dtup(const uint8_t *frame_buffer, const uint8_t size) {

	bool confirmed = (frame_buffer[0] >> 7) == 1;

	// TODO add fOpts (size)

	printf("(%s): devaddr=%08lX fcnt=%u fport=%u%s%s%s mic=%08lX",
			confirmed ? "CONFIRMED" : "UNCONFIRMED",
			lorawan_get_devaddr(frame_buffer, size),
			lorawan_get_fcnt(frame_buffer, size),
			lorawan_get_fport(frame_buffer, size),

			lorawan_get_fctrl_adr(frame_buffer, size) ? " adr=on" : " adr=off",
			lorawan_get_fctrl_ack(frame_buffer, size) ? " ack" : "",
			lorawan_get_fctrl_pending(frame_buffer, size) ? " pending" : "",

			lorawan_get_mic(frame_buffer, size));

#ifdef COMMISSION_TEST_DEVICE_DEVADDR
	if(lorawan_get_devaddr(frame_buffer, size)==COMMISSION_TEST_DEVICE_DEVADDR) {
		printf(
				"\n"
				"----------------------------------------------------------\n"
				"THIS IS THE TEST DEVICE FOR COMMISSIONING\n"
				"----------------------------------------------------------\n"
				"\n"
				);
	}
#endif
}

/** Get JoinEUI */
uint64_t lorawan_get_joineui(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint64_t joineui;
	joineui = (uint64_t) frame_buffer[1];
	joineui |= (uint64_t) frame_buffer[2] << 8;
	joineui |= (uint64_t) frame_buffer[3] << 16;
	joineui |= (uint64_t) frame_buffer[4] << 24;
	joineui |= (uint64_t) frame_buffer[5] << 32;
	joineui |= (uint64_t) frame_buffer[6] << 40;
	joineui |= (uint64_t) frame_buffer[7] << 48;
	joineui |= (uint64_t) frame_buffer[8] << 56;
	return joineui;
}

/** Get DevEUI */
uint64_t lorawan_get_deveui(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint64_t deveui;
	deveui = (uint64_t) frame_buffer[8 + 1];
	deveui |= (uint64_t) frame_buffer[8 + 2] << 8;
	deveui |= (uint64_t) frame_buffer[8 + 3] << 16;
	deveui |= (uint64_t) frame_buffer[8 + 4] << 24;
	deveui |= (uint64_t) frame_buffer[8 + 5] << 32;
	deveui |= (uint64_t) frame_buffer[8 + 6] << 40;
	deveui |= (uint64_t) frame_buffer[8 + 7] << 48;
	deveui |= (uint64_t) frame_buffer[8 + 8] << 56;
	return deveui;
}

/** Get DevNonce */
uint16_t lorawan_get_devnonce(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint16_t devnonce;
	devnonce = (uint64_t) frame_buffer[8 + 9];
	devnonce |= (uint64_t) frame_buffer[8 + 10] << 8;
	return devnonce;
}

void lorawan_printf_jreq(const uint8_t *frame_buffer, const uint8_t size) {

	uint64_t joineui = lorawan_get_joineui(frame_buffer, size);
	uint64_t deveui = lorawan_get_deveui(frame_buffer, size);
	uint16_t devnonce = lorawan_get_devnonce(frame_buffer, size);
	uint32_t mic = lorawan_get_mic(frame_buffer, size);

	char eui_hex[20] = { 0 };
	fmt_u64_hex(eui_hex, deveui);
	printf("JREQ: deveui=0x%s", eui_hex);
	fmt_u64_hex(eui_hex, joineui);
	printf(" joineui=0%s devnonce=%04X mic=%08lX", eui_hex, devnonce, mic);
}

void lorawan_printf_payload(const uint8_t *frame_buffer, const uint8_t size) {

	if (size < 5)
		return;

	const uint8_t frame_version = lorawan_get_version(frame_buffer, size);
	if (frame_version != 0) {
		printf("PROPRIETARY: Version = %02X\n", frame_version);
		return;
	}

	const uint8_t frame_type = lorawan_get_type(frame_buffer, size);

	switch (frame_type) {
	case FRAME_TYPE_JOIN_REQ:
	case FRAME_TYPE_REJOIN: {
		if (size != 23)
			return;

		lorawan_printf_jreq(frame_buffer, size);
		printf("\n");
	}
		break;
	case FRAME_TYPE_JOIN_ACCEPT: {
		if (size <= 17 || size >= (17 + 16))
			return;

		printf("JACC: mic=%08lX\n", lorawan_get_mic(frame_buffer, size));
	}
		break;
	case FRAME_TYPE_DATA_UNCONFIRMED_UP:
	case FRAME_TYPE_DATA_CONFIRMED_UP: {
		if (size < 12)
			return;

		printf("DTUP ");
		lorawan_printf_dtup(frame_buffer, size);
		printf("\n");

	}
		break;
	case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
	case FRAME_TYPE_DATA_CONFIRMED_DOWN: {
		if (size < 12)
			return;

		printf("DTDN ");
		lorawan_printf_dtup(frame_buffer, size);
		printf("\n");
	}
		break;
	default:
		printf("PROPRIETARY:\n");
	}
}

/** Get LoRaWAN datarate (for eu868) */
uint8_t lorawan_get_datarate(uint8_t sf, uint32_t bw) {
	if (bw == 125000 && sf <= 7) {
		return 12 - sf;
	} else if (bw == 250000 && sf == 7) {
		return 6;
	} else {
		return 7;
	}
}
