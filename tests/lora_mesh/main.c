/*
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#include "lora_mesh.h"


#include "shell.h"


static const shell_command_t shell_commands[] = {
	{ NULL, NULL, NULL }
};



const MeshLoRa_Uplink_t uplink = {
	.mtype = LORAMESH_MTYPE_PROPRIETARY,
	.payload_type = LORAMESH_PAYLOAD_TYPE_UPLINK,
	.hop_count = 2,
	.uplink_id = 1234,
	.datarate = 1,
	.rssi = 120,
	.snr = 10,
	.channel = 1,
	.relay_id = 0x12345678,
};

	uint8_t lora_mesh_phypayload[LORAWAN_PHYPAYLOAD_LEN - 14U];

	uint8_t frame[] = {0x00, 0x01, 0x02, 0x03};



int main(void) {

	uint8_t buf[LORAWAN_PHYPAYLOAD_LEN];
	uint32_t mic = 0x0A0B0C0D;

	memcpy(buf, &uplink, 10);
	memcpy(buf + 10, frame, sizeof(frame));
	memcpy(buf + 10 + sizeof(frame), &mic, sizeof(mic));

	lora_mesh_printf_frame(buf, 10 + sizeof(frame) + sizeof(mic));


	puts("");

	puts("=========================================");
	puts("Copyright (c) 2021-2025 UGA CSUG LIG");
	puts("=========================================");


	puts("\nBOARD: " RIOT_BOARD "\n");

	/* start the shell */
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

	return 0;
}
