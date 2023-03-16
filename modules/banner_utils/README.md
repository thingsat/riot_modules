# Banner utils

## Usage



1) Add the following line into the Makefile
```makefile
USEMODULE += banner_utils
EXTERNAL_MODULE_DIRS += ../../modules/banner_utils
```

2) Add the banner.c file into the application project
```c
char* banner="\
========================================================================\n\
dBBBBBBP dBP dBP dBP dBBBBb  dBBBBb.dBBBBP dBBBBBb  dBBBBBBP\n\
                        dBP        BP           BB          \n\
 dBP   dBBBBBP dBP dBP dBP dBBBB   `BBBBb   dBP BB   dBP    \n\
dBP   dBP dBP dBP dBP dBP dB' BB      dBP  dBP  BB  dBP     \n\
dBP   dBP dBP dBP dBP dBP dBBBBBB dBBBBP'  dBBBBBBB dBP     \n\
\n\
   dBBBBBb dBBBBBb dBP dBP dBP    dBBBBP dBBBBBb     dBBBBb\n\
       dB'      BB    dBP        dB'.BP       BB        dB'\n\
   dBBBP'   dBP BB   dBP dBP    dB'.BP    dBP BB   dBP dB' \n\
  dBP      dBP  BB  dBP dBP    dB'.BP    dBP  BB  dBP dB'  \n\
 dBP      dBBBBBBB dBP dBBBBP dBBBBP    dBBBBBBB dBBBBB'   \n\
\n\
Copyright (c) 2020-2021 Universite Grenoble Alpes, CSUG\n\
Contact: www.csug.fr\n\
========================================================================\n\n\n";
```


3) Add the following lines into main.c
```c
#include "banner_utils.h"

static const shell_command_t _commands[] = {
		{ "banner", "Show banner", banner_cmd },
...
		{ NULL, NULL, NULL },
};

int main(void) {
	banner_cmd(0,NULL);
	char line_buf[SHELL_DEFAULT_BUFSIZE];
	shell_run(_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
	return 0;
}

```

## For generating ASCII banners
* [ASCII banner tool](https://manytools.org/hacker-tools/ascii-banner/)
