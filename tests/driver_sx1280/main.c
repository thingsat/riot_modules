/*
 * Copyright (C) 2020-2023 Université Grenoble Alpes
 */

/**
 * @file
 * @brief       Shell for testing the SX1280 as a LoRa 2.4 GHz modem.
 *
 * @author      Nicolas Albarel
 * @author      Didier Donsez
 * @author      Olivier Alphand
 */

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmt.h"


#include "shell.h"

#include "sx1280_cmd.h"
#include "sx1280_rangetest_cmd.h"

#if RANGETEST != 1

static const shell_command_t shell_commands[] = {
    { "sx1280", "LoRa sx1280 basic commands", sx1280_cmd },
    { "rangetest", "Range Test", sx1280_rangetest_cmd },
    { NULL, NULL, NULL }
};
#endif

int main(void)
{
    sx1280_init_and_reboot_on_failure();

#if RANGETEST == 1
    char *argv[] = { "rangetest", "1000000", "24", "5", "2422000", "2425000", "2479000" };
    // rangetest <number of packet> <packet size> <delay in seconds> <channel1_in_khz> <channel2_in_khz> <channel3_in_khz>
    (void)sx1280_rangetest_cmd(7, argv );
    puts("rebooting ...");
    pm_reboot();
#else
    /* define buffer to be used by the shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];

    /* define own shell commands */
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
#endif

    return 0;
}
