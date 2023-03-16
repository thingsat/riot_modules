#
# Copyright (C) 2021-2022 Université Grenoble Alpes
#

APPLICATION=test_sx1302

BOARD ?= nucleo-f446re

BOARD_WHITELIST := thingsat-f4 nucleo-f411re nucleo-f446re nucleo-f446ze nucleo-f767zi nucleo-f722ze

ifeq ($(BOARD),nucleo-f446re)
CFLAGS += -DCORECELL_ON_NUCLEO=1
endif

# -----------------------------
# Debug
# -----------------------------

# Set this to 1 to enable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

#CFLAGS += -DENABLE_DEBUG=1
CFLAGS += -DDEBUG_ASSERT_VERBOSE=1

# -----------------------------
# Modules 
# -----------------------------

USEMODULE += shell
# USEMODULE += shell_commands
USEMODULE += ps

USEMODULE += fmt
USEMODULE += printf_float

USEMODULE += sx1302
EXTERNAL_MODULE_DIRS += ../../modules

FEATURES_REQUIRED += periph_rtc
FEATURES_OPTIONAL += periph_rtc_ms
FEATURES_OPTIONAL += periph_rtc_mem

# DISABLE_MODULE += periph_init_rtc

#32768U 16384U
CFLAGS += -DTHREAD_STACKSIZE_MAIN=8192U
CFLAGS += -DLGW_LISTEN_STACKSIZE=8192U


# -----------------------------
# Sx1302 Module footprint (TODO)
# -----------------------------
CFLAGS += -DENABLE_STTS751=0
CFLAGS += -DENABLE_MOD_FSK=0
# when CFLAGS += -DENABLE_SX125X=0 --> save 8KB
CFLAGS += -DENABLE_SX125X=0
CFLAGS += -DENABLE_SX1250=1

# -----------------------------
# Flags 
# -----------------------------

CFLAGS += -DENABLE_REGTEST=1

# Should be removed for production
# CFLAGS += -Wno-unused-variable -Wno-unused-function
# CFLAGS += -Wno-sign-compare
# CFLAGS += -Wno-multistatement-macros
CFLAGS += -Wno-unused-function

include $(RIOTBASE)/Makefile.include
