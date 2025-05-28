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

uint8_t lora_mesh_phypayload[LORAWAN_PHYPAYLOAD_LEN - 14U];

uint8_t signing_key[]= {0xca, 0xfe, 0xba, 0xbe, 0x12, 0x34, 0x56, 0x78, 0xca, 0xfe, 0xba, 0xbe, 0x12, 0x34, 0x56, 0x78};


uint8_t buf[LORAWAN_PHYPAYLOAD_LEN];
uint8_t buf_size;


static uint8_t _encode(void){

	// https://en.wikipedia.org/wiki/Hexspeak
	uint8_t frame[] = {0xca, 0xfe, 0xba, 0xbe, 0xde, 0xad, 0xbe, 0xef};

	lora_mesh_build_uplink(
		buf,
		&buf_size,
		2,
		0x1234,
		5,
		130,
		10,
		0,
		0xdeedbeef,
		frame,
		sizeof(frame),
		signing_key
	);

	return buf_size;
}

static void _decode(void){

	bool res = lora_mesh_check_mic(buf, buf_size, signing_key);
	if (res) {
		puts("MIC check: SUCCESS");
	} else {
		puts("MIC check: FAILED");
	}

	lora_mesh_printf_frame(buf, buf_size);
}


int main(void) {

	(void)_encode();
	(void)_decode();


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
