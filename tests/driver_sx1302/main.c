/*
 SX1302 LGW commands
 Copyright (c) 2021-2022 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "shell.h"

#include "lgw_cmd.h"

#ifndef NO_SHELL
static const shell_command_t shell_commands[] = {
	{ "lgw", "LoRa gateway commands", lgw_cmd },
	{ NULL, NULL, NULL }
};
#endif

int main(void) {

	puts("=========================================");
	puts("SX1302 Driver Test Application");
	puts("Copyright (c) 2021-2023 UGA CSUG LIG");
	puts("=========================================");

#ifdef NO_SHELL
	lgw_cmd(2, (char*[]){"lgw","start"});
	lgw_cmd(2, (char*[]){"lgw","eui"});
	lgw_cmd(2, (char*[]){"lgw","listen"});
#else
	/* start the shell */
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif
	return 0;
}
