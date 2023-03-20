#
# Copyright (C) 2021-2023 Université Grenoble Alpes
#

APPLICATION=test_driver_sx1302

BOARD ?= nucleo-f446re

BOARD_WHITELIST := thingsat-f4 nucleo-f411re nucleo-f446re nucleo-f446ze nucleo-f429zi nucleo-f767zi nucleo-f722ze

ifeq ($(BOARD),nucleo-f446re)
CFLAGS += -DCORECELL_ON_NUCLEO=1
# USEMODULE += stts751
endif


ifeq ($(BOARD),nucleo-f429zi)
CFLAGS += -DCORECELL_ON_NUCLEO=1
# USEMODULE += stts751
endif

# https://www.st.com/resource/en/data_brief/x-nucleo-iks01a3.pdf
ifeq ($(IKS01A3),yes)
# a3g4250d gyro and temperature
USEMODULE += l3gxxxx
USEMODULE += a3g4250d

# HTS221 humidity and temperature sensor
USEMODULE += hts221

# LPS22HH pressure sensor
USEMODULE += lpsxxx
USEMODULE += lps22hh

# LSM6DSO 3-axis accelerometer + 3-axis gyroscope

# LIS2MDL 3-axis magnetometer

# LIS2DW12 3-axis accelerometer

# STTS751 temperature sensor

endif 


# Gateway is started after reboot and listens LoRa messages
ifeq ($(NO_SHELL),1)
CFLAGS += -DNO_SHELL=1
endif

# include and auto-initialize all available sensors
USEMODULE += saul_default

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
