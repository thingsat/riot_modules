# name of your application
APPLICATION = test_camf
# If no BOARD is found in the environment, use this default:
BOARD ?= nucleo-f446re

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

#USEMODULE += printf_float

USEMODULE += shell
# USEMODULE += shell_commands
USEMODULE += shell_cmds_default


USEMODULE += printf_float

USEMODULE += xtimer
FEATURES_REQUIRED += periph_rtc

USEMODULE += camf
EXTERNAL_MODULE_DIRS += ../../modules/

include $(RIOTBASE)/Makefile.include
