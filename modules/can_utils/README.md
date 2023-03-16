# RIOT :: CAN :: Shell commands


* [Setup](./instructions-can-setup.md)
* [Instructions](instructions-can.md)



# Build

## Makefile
```makefile
# -----------------------------
# CAN
# -----------------------------
# The ST Microelectronics L9616 CAN transceiver is compatible with the NXP TJA1042 CAN transceiver
TRX_TO_BUILD ?= tja1042

ifneq (,$(filter tja1042,$(TRX_TO_BUILD)))
USEMODULE += tja1042
ifeq ($(BOARD),thingsat-f4)
# PB0 is not used by the board
	TJA1042_STB_PIN ?= GPIO_PIN\(1,0\)
else
# PA0 (default value given into the examples)
	TJA1042_STB_PIN ?= GPIO_PIN\(0,0\)
endif
	CFLAGS += -DTJA1042_STB_PIN=$(TJA1042_STB_PIN)
endif

ifeq ($(BOARD),nucleo-f446re)
# Enabling CAN2 (aka can1)
CFLAGS += -DCAN_DLL_NUMOF=2
CFLAGS += -DPAYLOAD_CAN=\"can1\"
endif

ifeq ($(BOARD),nucleo-f446ze)
# Enable CAN2 and CAN3 (aka can1 and can2)
CFLAGS += -DCAN_DLL_NUMOF=1
CFLAGS += -DPAYLOAD_CAN=\"can0\"
endif

ifeq ($(BOARD),nucleo-f722ze)
CFLAGS += -DCAN_DLL_NUMOF=1
CFLAGS += -DPAYLOAD_CAN=\"can0\"
endif

ifeq ($(BOARD),nucleo-f767zi)
# Enable CAN2 and CAN3 (aka can1 and can2)
CFLAGS += -DCAN_DLL_NUMOF=1
CFLAGS += -DPAYLOAD_CAN=\"can0\"
endif

# set default bitrate
CFLAGS += -DCANDEV_STM32_DEFAULT_BITRATE=1000000U
# set default sampling-point
CFLAGS += -DCANDEV_STM32_DEFAULT_SPT=750

# This is the maximum number of frame the driver can receive simultaneously 
# CFLAGS += -DCAN_STM32_RX_MAIL_FIFO=12


```




## can_dev.c files to add into the application project
```c

#include <stddef.h>
#include "can/can_trx.h"


#ifdef MODULE_TJA1042
#include "tja1042.h"
tja1042_trx_t tja1042 = { .trx.driver = &tja1042_driver,
                          .stb_pin = TJA1042_STB_PIN
};
#endif

#ifdef MODULE_NCV7356
#include "ncv7356.h"
ncv7356_trx_t ncv7356 = { .trx.driver = &ncv7356_driver,
                          .mode0_pin = NCV7356_MODE0_PIN,
                          .mode1_pin = NCV7356_MODE1_PIN
};
#endif

can_trx_t *can_devs[] = {
#ifdef MODULE_TJA1042
    (can_trx_t *)&tja1042,
#endif
#ifdef MODULE_NCV7356
    (can_trx_t *)&ncv7356,
#endif
    NULL,
};
```
