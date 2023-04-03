#
# Copyright (C) 2020-2023 Université Grenoble Alpes
#

# -----------------------------

APPLICATION=bridge_uart

# -----------------------------
# Boards
# -----------------------------

# If no BOARD is found in the environment, use this default:
BOARD ?= thingsat-f4

BOARD_WHITELIST := thingsat-f4 nucleo-f446re nucleo-f446ze nucleo-f767zi nucleo-f722ze lora-e5-dev

FEATURES_REQUIRED += periph_uart
FEATURES_OPTIONAL += periph_lpuart  # STM32 L0 and L4 provides lpuart support
# FEATURES_OPTIONAL += periph_uart_modecfg

# -----------------------------
# Modules
# -----------------------------

USEMODULE += xtimer


CFLAGS += -Werror=unused-function

include $(RIOTBASE)/Makefile.include
