/*
 * Copyright (C) 2020-2021 Université Grenoble Alpes
 */

#include <stdio.h>

extern char* banner;

/*
 * @brief banner command
 *
 * @param argc
 * @param argv
 */
int banner_cmd(int argc, char *argv[]) {
	(void) (argc);
	(void) (argv);
#ifdef BANNER
	printf("%s\n", BANNER);
#else
	printf("%s\n",banner);
#endif
  return 0;
}
