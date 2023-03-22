/*
 * Copyright (C) 2023 Université Grenoble Alpes
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test the geofence library
 *
 * @author      Didier DONSEZ
 *
 * @}
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// #include <shell.h>
#include <geofence.h>


/* main */
int main(void)
{
    printf("\n************ Geofence test program ***********\n");

	printf("Grenoble : %d\n", check_geofence(45.16667, 5.71667));
    print_geofence(45.16667, 5.71667);
	printf("New York : %d\n", check_geofence(40.712784,-74.005941));
    print_geofence(40.712784,-74.005941);
	printf("Tahiti   : %d\n", check_geofence(-17.65946,-149.417145));
    print_geofence(-17.65946,-149.417145);

    printf("\n");

    print_geofence(0,0);
    print_geofence(89,0);
    print_geofence(90,0);
    print_geofence(-89,0);
    print_geofence(-90,0);


    /* run the shell */
//    char line_buf[SHELL_DEFAULT_BUFSIZE];
//    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
