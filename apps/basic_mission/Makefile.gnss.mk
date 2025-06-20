# -----------------------------
# GPS/GNSS Module
# -----------------------------

USEMODULE += gps_uart

GPS_UART_ENABLE ?= 0
GPS_UART_ENABLE_TRACE ?= 0
GPS_UART_ENABLE_TRACE_ERROR ?= 0
GPS_UART_ENABLE_TRACE_BADCHECKSUM ?= 0
GPS_UART_ENABLE_TRACE_UNKNOWN_SENTENCE ?= 0
GPS_UART_ENABLE_ERROR_MAX_BEFORE_RESTART ?= 100

CFLAGS += -DGPS_UART_ENABLE=$(GPS_UART_ENABLE)
CFLAGS += -DGPS_UART_ENABLE_TRACE=$(GPS_UART_ENABLE_TRACE)
CFLAGS += -DGPS_UART_ENABLE_TRACE_ERROR=$(GPS_UART_ENABLE_TRACE_ERROR)
CFLAGS += -DGPS_UART_ENABLE_TRACE_BADCHECKSUM=$(GPS_UART_ENABLE_TRACE_BADCHECKSUM)
CFLAGS += -DGPS_UART_ENABLE_TRACE_UNKNOWN_SENTENCE=$(GPS_UART_ENABLE_TRACE_UNKNOWN_SENTENCE)
CFLAGS += -DGPS_UART_ENABLE_ERROR_MAX_BEFORE_RESTART=$(GPS_UART_ENABLE_ERROR_MAX_BEFORE_RESTART)

FEATURES_REQUIRED += periph_uart
FEATURES_OPTIONAL += periph_lpuart  # STM32 L0 and L4 provides lpuart support
FEATURES_OPTIONAL += periph_uart_modecfg
FEATURES_OPTIONAL += periph_uart_rxstart_irq

ifeq ($(BOARD),thingsat-up4)
# Set UART board for GNSS
GPS_UART_DEV = 1
GPS_UART_BAUDRATE ?= 9600
# Disable GPS in SX1302 driver
ENABLE_GPS = 0
endif

ifeq ($(BOARD),thingsat-up1-f4)
# Set UART board for GNSS
GPS_UART_DEV = 1
GPS_UART_BAUDRATE ?= 9600
# Disable GPS in SX1302 driver
ENABLE_GPS = 0
endif

ifeq ($(RAK5146_ON_NUCLEO),yes)
CFLAGS += -DRAK5146_ON_NUCLEO=1
# No temperature Sensor on RAK5146
# No PPS pin

# Builtin GNSS on RAK5146
#GPS_MODEL = "uBlox ZOE-M8Q-GPS"
GPS_BAUDRATE = 9600
GPS_UART_DEV = 1
GPS_RESET_PIN = ARDUINO_PIN_6
GPS_UART_ENABLE_TRACE = 0
# Enable GPS in SX1302 driver
ENABLE_GPS = 1
endif


ifeq ($(BOARD),p-nucleo-wb55)
# https://os.mbed.com/platforms/ST-Nucleo-WB55RG/
CFLAGS += -DRAK5146_ON_NUCLEO=1
# No temperature Sensor on RAK5146
# No PPS pin

# Builtin GNSS on RAK5146
#GPS_MODEL = "uBlox ZOE-M8Q-GPS"
GPS_BAUDRATE = 9600
GPS_UART_DEV = 1
#        .rx_pin     = GPIO_PIN(PORT_A, 3), ARDUINO_D0
#        .tx_pin     = GPIO_PIN(PORT_A, 2), ARDUINO_D1
GPS_RESET_PIN = GPIO_PIN\(PORT_A,8\)
GPS_STANDBY_PIN = GPIO_PIN\(PORT_A,15\)
GPS_UART_ENABLE_TRACE = 0
# Enable GPS in SX1302 driver
ENABLE_GPS = 1
endif

ifeq ($(BOARD),nucleo-f446re)
# https://os.mbed.com/platforms/ST-Nucleo-F446RE/
CFLAGS += -DRAK5146_ON_NUCLEO=1
# No temperature Sensor on RAK5146
# No PPS pin

# Builtin GNSS on RAK5146
#GPS_MODEL = "uBlox ZOE-M8Q-GPS"
GPS_BAUDRATE = 9600
GPS_UART_DEV = 1
#        .rx_pin     = GPIO_PIN(PORT_A, 10), ARDUINO_D3
#        .tx_pin     = GPIO_PIN(PORT_A, 9), ARDUINO_D9
GPS_RESET_PIN = GPIO_PIN\(PORT_B,10\)
GPS_STANDBY_PIN = GPIO_PIN\(PORT_B,4\)
GPS_UART_ENABLE_TRACE = 0
# Enable GPS in SX1302 driver
ENABLE_GPS = 1
endif


ifeq ($(BOARD),nucleo-f401re)
# https://os.mbed.com/platforms/ST-Nucleo-F446RE/
CFLAGS += -DRAK5146_ON_NUCLEO=1
# No temperature Sensor on RAK5146
# No PPS pin

# Builtin GNSS on RAK5146
#GPS_MODEL = "uBlox ZOE-M8Q-GPS"
GPS_BAUDRATE = 9600
GPS_UART_DEV = 1
#        .rx_pin     = GPIO_PIN(PORT_A, 10), ARDUINO_D3
#        .tx_pin     = GPIO_PIN(PORT_A, 9), ARDUINO_D9
GPS_RESET_PIN = GPIO_PIN\(PORT_B,10\)
GPS_STANDBY_PIN = GPIO_PIN\(PORT_B,4\)
# Enable GPS in SX1302 driver
ENABLE_GPS = 1
endif


ifeq ($(BOARD),nucleo-l432kc-inisat)
GPS_UART_DEV = 1
GPS_BAUDRATE = 9600
# Enable GPS in SX1302 driver
ENABLE_GPS = 1
endif

ifeq ($(RAK5146_ON_NUCLEO32),yes)
CFLAGS += -DRAK5146_ON_NUCLEO32=1
# Enable GPS in SX1302 driver
ENABLE_GPS = 1

# No temperature Sensor on RAK5146
# No PPS pin

# Builtin GNSS on RAK5146
#GPS_MODEL = "uBlox ZOE-M8Q-GPS"
GPS_UART_DEV = 1
GPS_BAUDRATE = 9600
GPS_RESET_PIN = ARDUINO_PIN_6
endif

ifdef GPS_UART_DEV
CFLAGS += -DGPS_UART_DEV=$(GPS_UART_DEV)
CFLAGS += -DGPS_UART_ENABLE_TRACE=$(GPS_UART_ENABLE_TRACE)
endif

ifdef ENABLE_GPS
# Enable GPS in SX1302 driver
CFLAGS += -DENABLE_GPS=$(ENABLE_GPS)
endif

ifdef GPS_MODEL
CFLAGS += -DGPS_MODEL=$(GPS_MODEL)
endif

ifdef GPS_BAUDRATE
CFLAGS += -DGPS_BAUDRATE=$(GPS_BAUDRATE)
endif

ifdef GPS_RESET_PIN
CFLAGS += -DGPS_RESET_PIN=$(GPS_RESET_PIN)
endif

ifdef GPS_STANDBY_PIN
CFLAGS += -DGPS_STANDBY_PIN=$(GPS_STANDBY_PIN)
endif

ifdef GPS_PPS_INT_PIN
CFLAGS += -DGPS_PPS_INT_PIN=$(GPS_PPS_INT_PIN)
endif
