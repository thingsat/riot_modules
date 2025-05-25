#
# Copyright (C) 2021-2024 Université Grenoble Alpes
#
RIOTBASE ?= ${HOME}/github/RIOT-OS/RIOT

APPLICATION=test_minmea_uart

BOARD ?= nucleo-l432kc-inisat

EXTERNAL_MODULE_DIRS += ../../modules

EXTERNAL_BOARD_DIRS ?= ../../boards

FEATURES_REQUIRED += periph_uart
FEATURES_OPTIONAL += periph_lpuart  # STM32 L0 and L4 provides lpuart support
FEATURES_OPTIONAL += periph_uart_modecfg
FEATURES_OPTIONAL += periph_uart_rxstart_irq

USEMODULE += shell
USEMODULE += xtimer

USEPKG += minmea
USEMODULE += fmt
#USEMODULE += printf_float

#CFLAGS += -DISR_STACKSIZE=2048U

CFLAGS += -Wno-switch

CFLAGS += -DSTDIO_UART_BAUDRATE=115200

# avoid running Kconfig by default
SHOULD_RUN_KCONFIG ?=

include $(RIOTBASE)/Makefile.include
