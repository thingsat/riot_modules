#
# Copyright (C) 2021-2024 Université Grenoble Alpes
#
RIOTBASE ?= ${HOME}/github/RIOT-OS/RIOT

APPLICATION=test_driver_sx1302

BOARD ?= thingsat-up4-v2
EXTERNAL_BOARD_DIRS ?= ../../boards

BOARD_WHITELIST := esp32-nano thingsat-up4-v2 thingsat-up4 thingsat-up1-f4 rpi-pico-w rpi-pico esp32-wroom-32 nucleo-l432kc nucleo-l432kc-inisat nucleo-l476rg nucleo-f411re nucleo-f446re nucleo-f446ze nucleo-f429zi nucleo-f746zg nucleo-f767zi nucleo-f722ze stm32f746g-disco

EXTERNAL_MODULE_DIRS += ../../modules

ifeq ($(BOARD),thingsat-up4-v2)
EXTERNAL_MODULE_DIRS += ../../modules
USEMODULE += stts751
# STTS751_CORECELL_I2C_ADDR is defined in board.h
endif

ifeq ($(BOARD),thingsat-up4)
EXTERNAL_MODULE_DIRS += ../../modules
USEMODULE += stts751
# STTS751_CORECELL_I2C_ADDR is defined in board.h
endif

ifeq ($(BOARD),nucleo-f446re)
CORECELL_ON_NUCLEO=yes
endif

ifeq ($(BOARD),nucleo-f429zi)
CORECELL_ON_NUCLEO=yes
endif

ifeq ($(BOARD),nucleo-f446ze)
CORECELL_ON_NUCLEO=yes
endif

ifeq ($(BOARD),nucleo-f746zg)
CORECELL_ON_NUCLEO=yes
endif

ifeq ($(BOARD),nucleo-l476rg)
RAK5146_ON_NUCLEO=yes
endif

ifeq ($(BOARD),nucleo-l432kc)
RAK5146_ON_NUCLEO32=yes
endif

ifeq ($(BOARD),nucleo-l432kc-inisat)
# MCU board for OBC Thingsat board designed for INISAT
RAK5146_ON_INISAT=yes
endif

ifeq ($(BOARD),esp32-wroom-32)
RAK5146_ON_NUCLEO=yes
endif

ifeq ($(BOARD),esp32-nano)
# WARNING! The following modules conflict with 'ztimer_periph_rtt': 'rtt_rtc'
# To disable ztimer periph_rtt auto-inclusion add 'ztimer_no_periph_rtt' to 'USEMODULE'
USEMODULE += ztimer_no_periph_rtt
RAK5146_ON_NUCLEO=yes
endif

ifeq ($(BOARD),rpi-pico)
RAK5146_ON_PICO=yes
endif

ifeq ($(BOARD),rpi-pico-w)
RAK5146_ON_PICO=yes
endif

ifeq ($(CORECELL_ON_NUCLEO),yes)
CFLAGS += -DCORECELL_ON_NUCLEO=1
# Builtin Temperature Sensor on Corecell
USEMODULE += stts751
CFLAGS += -DSTTS751_CORECELL_I2C_ADDR=0x39
endif

ifeq ($(RAK5146_ON_NUCLEO),yes)
CFLAGS += -DRAK5146_ON_NUCLEO=1
# No temperature Sensor on RAK5146
endif

ifeq ($(RAK5146_ON_NUCLEO32),yes)
CFLAGS += -DRAK5146_ON_NUCLEO32=1
# No temperature Sensor on RAK5146
endif

ifeq ($(RAK5146_ON_PICO),yes)
CFLAGS += -DRAK5146_ON_PICO=1
endif


ifeq ($(RAK5146_ON_INISAT),yes)
CFLAGS += -DRAK5146_ON_INISAT=1
endif


USEMODULE += saul_default
#USEMODULE += senml_saul
USEMODULE += senml_phydat
USEMODULE += printf_float

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


endif 

# OpenLog https://github.com/CampusIoT/tutorial/blob/master/openlogger/README.md
OPENLOG_BAUDRATE ?= 115200
CFLAGS += -DSTDIO_UART_BAUDRATE=$(OPENLOG_BAUDRATE)


#USEMODULE += periph_rtc

ENDPOINT_DEVADDR ?= 0xfc001234
CFLAGS += -DENDPOINT_DEVADDR=$(ENDPOINT_DEVADDR)


CFLAGS_DBG = 


# Gateway is started after reboot and listens LoRa messages
ifeq ($(NO_SHELL),1)
CFLAGS += -DNO_SHELL=1
endif

# include and auto-initialize all available sensors
#USEMODULE += saul_default

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



# -----------------------------
# SX1302 Lib 
# -----------------------------

# Default is 2.1.0
SX1302_LIB_VERSION ?= 2_1_0
#SX1302_LIB_VERSION ?= 1_0_4

ifeq ($(SX1302_LIB_VERSION),1_0_4)
USEMODULE += sx1302_1_0_4
endif

ifeq ($(SX1302_LIB_VERSION),2_1_0)
USEMODULE += sx1302_2_1_0
USEMODULE += ztimer
USEMODULE += ztimer_sec
#USEMODULE += ztimer_msec
endif

EXTERNAL_MODULE_DIRS += ../../public_modules

#FEATURES_REQUIRED += periph_rtc
FEATURES_OPTIONAL += periph_rtc
FEATURES_OPTIONAL += periph_rtc_ms
FEATURES_OPTIONAL += periph_rtc_mem

# DISABLE_MODULE += periph_init_rtc

#32768U 16384U
#ifndef STTS751
#CFLAGS += -DTHREAD_STACKSIZE_MAIN=8192U
1CFLAGS += -DLGW_LISTEN_STACKSIZE=8192U

CFLAGS += -DTHREAD_STACKSIZE_MAIN=8192U
ifeq ($(BOARD),nucleo-l432kc)
CFLAGS += -DLGW_LISTEN_STACKSIZE=10240U
else
CFLAGS += -DLGW_LISTEN_STACKSIZE=16384U
endif

# -----------------------------
# For Repeat callback
# -----------------------------

CFLAGS += -DINVOKE_CALLBACKS=1

# -----------------------------
# Sx1302 Module footprint (TODO)
# -----------------------------

CFLAGS += -DLGW_PA_GAIN=0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1
CFLAGS += -DLGW_PWR_IDX=15,16,17,19,20,22,1,2,3,4,5,6,7,9,11,14

LGW_SX1302_LUT_DIG_GAIN ?= 0

CFLAGS += -DLGW_SX1302_LUT_DIG_GAIN=$(LGW_SX1302_LUT_DIG_GAIN)
# For SX1257 only
CFLAGS += -DLGW_SX1257_LUT_DAC_GAIN=3
CFLAGS += -DLGW_SX1257_LUT_MIX_GAIN=10



ifeq ($(ENABLE_GPS),1)
include Makefile.gnss.mk
endif

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
