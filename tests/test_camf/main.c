/*
 * Copyright (C) 2025 Université Grenoble Alpes
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test the CAMF library
 *
 * @author      Didier DONSEZ, Université Grenoble Alpes
 *
 * @}
 */

 #include <stdio.h>
 #include <stdint.h>
 #include <stdlib.h>
 
 // #include <shell.h>
 #include "camf_payload.h"
 #include "camf_utils.h"
 
 #include "msg/fire.h"

static const uint32_t camf_message_test_2_ba[4] = {
    0b01111100011011111011100100101010,
    0b11111111001100001001001001000011,
    0b10101001000000000000000110111111,
    0b00000000000000000111111010000100
    
    };
 
static const CommonAlertMessageFormat_t* camf_message_test_2 = (const CommonAlertMessageFormat_t*)camf_message_test_2_ba;

 /* main */
 int main(void)
 {
    printf("\n************ CAMF test program ***********\n");
 
    printf("\n************ CAMF Fire Message ***********\n");
    camf_message_printf(&camf_sample_msg_fire);

    printf("\n************ CAMF Fire Message ***********\n");
    camf_message_printf(camf_message_test_2);

     /* run the shell */
 //    char line_buf[SHELL_DEFAULT_BUFSIZE];
 //    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
 
     return 0;
 }