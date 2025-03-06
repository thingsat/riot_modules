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
 

static const uint64_t camf_message_test_0_ba[2] = {
    0b00000000000000000000000000000000,
    0b00000000000000000000000000000000,
};

static const CommonAlertMessageFormat_t* camf_message_test_0 = (const CommonAlertMessageFormat_t*)camf_message_test_0_ba;

static const uint64_t camf_message_test_1_ba[2] = {
    0b00000000000000000000000000000001,
    0b00000000000000000000000000000000,
};

static const CommonAlertMessageFormat_t* camf_message_test_1 = (const CommonAlertMessageFormat_t*)camf_message_test_1_ba;


static const uint64_t camf_message_test_2_ba[2] = {
//  0b00000000000000000000000000000000,
    0b00001101111111101011100100101011,
    0b00000000000000000000000000001001,
//  0b0000000000000000000000000000 10 01,
};
/*
	uint64_t message_type :2;
	uint64_t country_id :9;
	uint64_t provider_id :5;

	uint64_t severity :2;
	uint64_t hazard_category :7;
	uint64_t hazard_onset_week_number :1;

	uint64_t hazard_onset_time_of_week :14; // 01 0000 1101 1111 WEDNESDAY - 11:58 PM

	uint64_t hazard_duration :2;
	uint64_t guidance_library_selection :1;
	uint64_t guidance_library_version :3;
	uint64_t guidance_instructions_a : 5;
	uint64_t guidance_instructions_b : 5;
*/



static const CommonAlertMessageFormat_t* camf_message_test_2 = (const CommonAlertMessageFormat_t*)camf_message_test_2_ba;


 /* main */
 int main(void)
 {
    printf("\n************ CAMF test program ***********\n");
 
 //   printf("\n************ CAMF Message 0 ***********\n");
//    camf_message_printf(camf_message_test_0);
    (void)camf_message_test_0;
//    printf("\n************ CAMF Message 1 ***********\n");
//    camf_message_printf(camf_message_test_1);
    (void)camf_message_test_1;
    printf("\n************ CAMF Message 2 ***********\n");
    camf_message_printf(camf_message_test_2);

     /* run the shell */
 //    char line_buf[SHELL_DEFAULT_BUFSIZE];
 //    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
 
     return 0;
 }