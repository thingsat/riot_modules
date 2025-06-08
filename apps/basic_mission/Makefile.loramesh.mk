
# Meshtastic Messages
MESHTASTIC ?= 0

ifeq ($(MESHTASTIC),1)
CFLAGS += -DMESHTASTIC=$(MESHTASTIC)
USEMODULE += meshtastic_utils
endif
