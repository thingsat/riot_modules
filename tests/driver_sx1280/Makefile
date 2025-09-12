#
# Copyright (C) 2020-2023 Université Grenoble Alpes, ANS Innovation
#

APPLICATION=driver_sx1280_shell

EXTERNAL_BOARD_DIRS ?= ../../boards
BOARD_WHITELIST := thingsat-l0 nucleo-l053r8 nucleo-l073rz nucleo-l432kc nucleo-f411re nucleo-f401re nucleo-f446re nucleo-f446ze nucleo-f103rb olimexino-stm32
# BOARD_WHITELIST += esp32-wroom-32 lora-e5-dev bluepill bluepill-128kib

BOARD ?= nucleo-f446re

USEMODULE += sx1280_raw_driver
EXTERNAL_MODULE_DIRS += ../../modules/


USEMODULE += hashes
USEMODULE += crypto

USEMODULE += shell

USEMODULE += fmt
USEMODULE += printf_float

USEMODULE += xtimer
USEMODULE += random


# -----------------------------
# sx1280_utils
# -----------------------------

CFLAGS += -DISR_STACKSIZE=2048U

ifdef DEVADDR
CFLAGS += -DDEVADDR=$(DEVADDR)
endif

ifdef FPORT
CFLAGS += -DFPORT=$(FPORT)
endif

ifdef RECEIVE_WINDOWS_IN_SECONDS
CFLAGS += -DRECEIVE_WINDOWS_IN_SECONDS=$(RECEIVE_WINDOWS_IN_SECONDS)
endif

ifdef RECEIVE_WINDOWS_TIMEOUT_IN_SECONDS
CFLAGS += -DRECEIVE_WINDOWS_TIMEOUT_IN_SECONDS=$(RECEIVE_WINDOWS_TIMEOUT_IN_SECONDS)
endif

ifdef APPSKEY
CFLAGS += -DAPPSKEY=$(APPSKEY)
endif

ifdef NWKSKEY
CFLAGS += -DNWKSKEY=$(NWKSKEY)
endif

# -----------------------------
# Default Frequency and parameters
# -----------------------------
CFLAGS += -DMULTITECH_ISM2400_PARAMS=1

ifdef RANGETEST
CFLAGS += -DRANGETEST=$(RANGETEST)
endif


# -----------------------------
# Flags
# -----------------------------
# Should be removed for production
#CFLAGS += -Wno-unused-variable
#CFLAGS += -Wno-unused-function
#CFLAGS += -Wno-error=unused-parameter
#CFLAGS += -Wno-sign-compare
#CFLAGS += -Wno-multistatement-macros

include $(RIOTBASE)/Makefile.include
