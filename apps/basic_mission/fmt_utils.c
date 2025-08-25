/*
 Basic Mission
 Copyright (c) 2021-2025 UGA CSUG LIG

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <stdio.h>

#include "fmt_utils.h"

void fmt_printf_ba(const uint8_t *ba, size_t len, const char *sep)
{
    for (unsigned int i = 0; i < len; i++) {
    	if(i > 0) {
            printf("%s", sep);
    	}
        printf("%02x", ba[i]);
    }
}
