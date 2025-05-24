#
# Copyright (C) 2021-2024 Université Grenoble Alpes
#
RIOTBASE ?= ${HOME}/github/RIOT-OS/RIOT

APPLICATION=test_lora_mesh

EXTERNAL_BOARD_DIRS ?= ../../boards

BOARD_WHITELIST := esp32-nano thingsat-up4-v2 thingsat-up4 thingsat-up1-f4 rpi-pico-w rpi-pico esp32-wroom-32 nucleo-l432kc nucleo-l432kc-inisat nucleo-l476rg nucleo-f411re nucleo-f446re nucleo-f446ze nucleo-f429zi nucleo-f746zg nucleo-f767zi nucleo-f722ze stm32f746g-disco

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
USEMODULE += printf_float
USEMODULE += lora_mesh


# -----------------------------
# Flags 
# -----------------------------


# Should be removed for production
# CFLAGS += -Wno-unused-variable -Wno-unused-function
# CFLAGS += -Wno-sign-compare
# CFLAGS += -Wno-multistatement-macros
CFLAGS += -Wno-unused-function
#CFLAGS += -Wno-cpp

include $(RIOTBASE)/Makefile.include
