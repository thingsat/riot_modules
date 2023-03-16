/*
 * Copyright (C) 2020-2021 Université Grenoble Alpes
 */

#ifndef RIOTBOOT_UTILS_H
#define RIOTBOOT_UTILS_H

#ifdef MODULE_RIOTBOOT


int riotboot_cmd(int argc, char *argv[]);

#endif

uint32_t Firmware_get_firmware_version(void);

uint8_t Firmware_get_slot_id(void);

#endif
