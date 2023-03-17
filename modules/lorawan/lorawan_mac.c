/*
 * Copyright (C) 2020-2022 Université Grenoble Alpes
 */

/*
 * Utils for the LoRaWAN MAC
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "fmt.h"

#include "lorawan_crypto.h"
#include "lorawan_mac.h"

static void lorawan_prepare_dataframe(
		const uint8_t dir,
		const bool confirmed,
		const uint32_t devaddr,
		const uint8_t fctrl,
		const uint32_t fcnt,
		const uint32_t fport,
		const uint8_t *fpayload,
		const uint8_t fpayload_size,
		const uint8_t *nwkskey,
		const uint8_t *appskey,
		uint8_t *frame_buffer,
		uint8_t *frame_size
		) {


	// No fOpt
	// const uint8_t foptlen = 0;

	// Reset buffer
	memset(frame_buffer, 0, 255); // TODO add constant instead of 255

	uint8_t len = 0;

	// Frame Type
	if(dir == LORAMAC_DIR_DOWNLINK) {
		frame_buffer[len++] = (confirmed ?
						FRAME_TYPE_DATA_CONFIRMED_DOWN :
						FRAME_TYPE_DATA_UNCONFIRMED_DOWN) << 5;
	} else {
		frame_buffer[len++] = (confirmed ?
						FRAME_TYPE_DATA_CONFIRMED_UP :
						FRAME_TYPE_DATA_UNCONFIRMED_UP) << 5;
	}

	// DevAddr
	frame_buffer[len++] = devaddr & 0xFF;
	frame_buffer[len++] = (devaddr >> 8) & 0xFF;
	frame_buffer[len++] = (devaddr >> 16) & 0xFF;
	frame_buffer[len++] = (devaddr >> 24) & 0xFF;

	// Frame Ctrl (ADR...)
	frame_buffer[len++] = fctrl; /* FCTrl (FOptLen = 0) */

	// FCnt (32 bit counter)
	frame_buffer[len++] = (uint8_t) fcnt;
	frame_buffer[len++] = (uint8_t) (fcnt >> 8) & 0xFF;

	// No FOpt

	// FPort
	frame_buffer[len++] = fport;

	// Encrypt fpayload with AppSKey (since fPort can not be 0)
	lorawan_payload_encrypt(
			fpayload,
			fpayload_size,
			fport==0 ? nwkskey : appskey,
			devaddr,
			dir,
			fcnt,
			frame_buffer + len);

	len += fpayload_size;

	// Add MIC computed with NwkSKey
	uint32_t mic;
	lorawan_cmac_calculate_mic(
			frame_buffer,
			len,
			nwkskey,
			devaddr,
			LORAMAC_DIR_UPLINK,
			fcnt,
			&mic);

	frame_buffer[len++] = mic & 0xFF;
	frame_buffer[len++] = ( mic >> 8 ) & 0xFF;
	frame_buffer[len++] = ( mic >> 16 ) & 0xFF;
	frame_buffer[len++] = ( mic >> 24 ) & 0xFF;

	*frame_size = len;
}


void lorawan_prepare_up_dataframe(
		const bool confirmed,
		const uint32_t devaddr,
		const uint8_t fctrl,
		const uint32_t fcnt,
		const uint32_t fport,
		const uint8_t *fpayload,
		const uint8_t fpayload_size,
		const uint8_t *nwkskey,
		const uint8_t *appskey,
		uint8_t *frame_buffer,
		uint8_t *frame_size
		) {

	lorawan_prepare_dataframe(
			LORAMAC_DIR_UPLINK,
			confirmed,
			devaddr,
			fctrl,
			fcnt,
			fport,
			fpayload,
			fpayload_size,
			nwkskey,
			appskey,
			frame_buffer,
			frame_size
	);
}

void lorawan_prepare_dn_dataframe(
		const bool confirmed,
		const uint32_t devaddr,
		const uint8_t fctrl,
		const uint32_t fcnt,
		const uint32_t fport,
		const uint8_t *fpayload,
		const uint8_t fpayload_size,
		const uint8_t *nwkskey,
		const uint8_t *appskey,
		uint8_t *frame_buffer,
		uint8_t *frame_size
		) {

	lorawan_prepare_dataframe(
			LORAMAC_DIR_DOWNLINK,
			confirmed,
			devaddr,
			fctrl,
			fcnt,
			fport,
			fpayload,
			fpayload_size,
			nwkskey,
			appskey,
			frame_buffer,
			frame_size
	);
}



//uint64_t swap_uint64( uint64_t val )
//{
//    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
//    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
//    return (val << 32) | (val >> 32);
//}

/** Get Type */
bool lorawan_check_valid_frame_size(const uint8_t *frame_buffer, const uint8_t size) {
	if (size < 1) {
		return false;
	}

	if(lorawan_get_version(frame_buffer,size)!=0) {
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

/** Get FPort */
uint8_t lorawan_get_fport(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint8_t fOptLen = frame_buffer[1] & 0x0F;
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
	uint8_t fOptLen = frame_buffer[1] & 0x0F;
	if (size == (1 + 4 + 1 + 2 + fOptLen + 4)) {
		// No Port, No Payload
		return NULL;
	} else {
		return (uint8_t*) (frame_buffer + (1 + 4 + 1 + 2 + fOptLen + 1));
	}
}

/** Get FPayload size */
uint8_t lorawan_get_fpayload_size(const uint8_t *frame_buffer, const uint8_t size) {
	(void) size;
	uint8_t fOptLen = frame_buffer[1] & 0x0F;
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
	if(frame_version != 0) {
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
