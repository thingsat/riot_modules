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
 

 static const uint8_t camf_message_test_0_ba[16] = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
};

static const CommonAlertMessageFormat_t* camf_message_test_0 = (const CommonAlertMessageFormat_t*)camf_message_test_0_ba;

static const uint8_t camf_message_test_1_ba[16] = {
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000,
};

static const CommonAlertMessageFormat_t* camf_message_test_1 = (const CommonAlertMessageFormat_t*)camf_message_test_1_ba;


 /* main */
 int main(void)
 {
    printf("\n************ CAMF test program ***********\n");
 
    camf_message_printf(camf_message_test_0);
    camf_message_printf(camf_message_test_1);

     /* run the shell */
 //    char line_buf[SHELL_DEFAULT_BUFSIZE];
 //    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
 
     return 0;
 }