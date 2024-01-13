/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2019 Semtech

Description:
    LoRa concentrator HAL auxiliary functions

License: Revised BSD License, see LICENSE.TXT file include in the project

Port to RIOT by Didier DONSEZ & Olivier ALPHAND, Université Grenoble-Alpes

*/

#include <xtimer.h>


void wait_ms(unsigned long a)
{
	xtimer_msleep(a);
}
