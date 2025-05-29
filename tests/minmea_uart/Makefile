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

USEMODULE += minmea_utils

# OpenLog https://github.com/CampusIoT/tutorial/blob/master/openlogger/README.md
OPENLOG_BAUDRATE ?= 115200
CFLAGS += -DOPENLOG_BAUDRATE=$(OPENLOG_BAUDRATE)
CFLAGS += -DSTDIO_UART_BAUDRATE=$(OPENLOG_BAUDRATE)


GNSS_UART_DEV ?= 1
GNSS_BAUDRATE ?= 9600
CFLAGS += -DGNSS_UART_DEV=$(GNSS_UART_DEV)
CFLAGS += -DGNSS_BAUDRATE=$(GNSS_BAUDRATE)



# USEPKG += minmea
# USEMODULE += fmt

#USEMODULE += printf_float

#CFLAGS += -DISR_STACKSIZE=2048U

CFLAGS += -Wno-switch

# avoid running Kconfig by default
SHOULD_RUN_KCONFIG ?=

include $(RIOTBASE)/Makefile.include
