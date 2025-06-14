

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
