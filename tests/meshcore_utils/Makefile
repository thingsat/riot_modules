#
# Copyright (C) 2021-2025 Université Grenoble Alpes
#
RIOTBASE ?= ${HOME}/github/RIOT-OS/RIOT

APPLICATION=test_meshcore_utils

EXTERNAL_BOARD_DIRS ?= ../../boards

BOARD ?= native

BOARD_WHITELIST := native native64 esp32-nano thingsat-up4-v2 thingsat-up4 thingsat-up1-f4 rpi-pico-w rpi-pico esp32-wroom-32 nucleo-l432kc nucleo-l432kc-inisat nucleo-l476rg nucleo-f411re nucleo-f446re nucleo-f446ze nucleo-f429zi nucleo-f746zg nucleo-f767zi stm32f746g-disco

EXTERNAL_MODULE_DIRS += ../../modules

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
USEMODULE += shell_cmds_default

USEMODULE += ps

USEMODULE += fmt
USEMODULE += meshcore_utils

# -----------------------------
# Flags
# -----------------------------

CFLAGS += -Wno-unused-function

include $(RIOTBASE)/Makefile.include
