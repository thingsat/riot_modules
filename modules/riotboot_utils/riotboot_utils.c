/*
 * Copyright (C) 2020-2021 Université Grenoble Alpes
 */

#ifdef MODULE_RIOTBOOT
#include "riotboot/slot.h"
#include "periph/pm.h"

#define ENABLE_DEBUG 		(ENABLE_DEBUG_RIOTBOOT_UTILS)

#define PREFIX 	"riotboot: "

/*
 * @brief riotboot_cmd command
 *
 * @param argc
 * @param argv
 */
int riotboot_cmd(int argc, char *argv[]) {
	(void) (argc);
	(void) (argv);

	int current_slot = riotboot_slot_current();
	if (current_slot != -1) {
		/* Sometimes, udhcp output messes up the following printfs.  That
		 * confuses the test script. As a workaround, just disable interrupts
		 * for a while.
		 */
		irq_disable();

		for (unsigned s = 0; s < NUM_SLOTS; s++) {
			printf(PREFIX "Slot %d starting at addr=%p, offset=%x, size=%x\n",
					s, (void*) riotboot_slot_get_image_startaddr(s),
					riotboot_slot_offset(s), riotboot_slot_size(s));
		}

		printf(PREFIX "Running from slot %d starting at addr=%p\n",
				current_slot, (void*) riotboot_slot_get_hdr(current_slot));
		riotboot_slot_print_hdr(current_slot);

#if ENABLE_DEBUG == 1
		printf(PREFIX "HDR of slot %d:\n",riotboot_slot_other());
		riotboot_slot_print_hdr(riotboot_slot_other());

		printf(PREFIX "Delta version between the current slot %d and the other slot %d: %ld\n",
				current_slot, riotboot_slot_other(),
				riotboot_slot_get_hdr(current_slot)->version - riotboot_slot_get_hdr(riotboot_slot_other())->version);
#endif
		irq_enable();
	} else {
		printf(PREFIX "You're not running riotboot\n");
	}

	return 0;
}

uint32_t Firmware_get_firmware_version(void) {
	uint32_t firmware_version = 0;

	int current_slot = riotboot_slot_current();
	if (current_slot != -1) {
		irq_disable();
		firmware_version = riotboot_slot_get_hdr(current_slot)->version;
		irq_enable();
	}
	return firmware_version;
}

uint8_t Firmware_get_slot_id(void) {
	return riotboot_slot_current();
}

#else

uint32_t Firmware_get_firmware_version(void) {
	return 0;
}

uint8_t Firmware_get_slot_id(void) {
	return 0;
}

#endif



