/*
 Thingsat project

 GPS over UART
 Copyright (c) 2021-2023 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

/*
 * Author: Didier Donsez, Université Grenoble Alpes
 */

#ifndef _GPS_UART_H
#define _GPS_UART_H

int gps_start(const unsigned dev, const uint32_t baud);
void gps_restart(void);
void gps_power_off_on(void);
int gps_cmd(int argc, char **argv);

#endif
