/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>

#include "endpoints.h"
#include "mission.h"
#include "lgw_cmd.h"

void mission(struct lgw_pkt_tx_s *lgw_pkt_tx_s) {
	(void) lgw_pkt_tx_s;
	printf("INFO: call mission\n");

	if (lgw_sx130x_endpoint == NULL) {
		puts("ERROR: lgw_sx130x_endpoint is null : Can not start TX bench");
	} else {
		lgw_cmd(11, (char*[] ) { "lgw", "bench", "1", "7", "125", "8",
						"12", "on", "false", "32", "1000" });
	}

}
