
# Meshtastic
MESHTASTIC_ENABLE ?= 0

ifeq ($(MESHTASTIC_ENABLE),1)
include Makefile.meshtastic.mk
endif

# Chirpstack Mesh
CHIRPSTACK_MESH_ENABLE ?= 0

ifeq ($(CHIRPSTACK_MESH_ENABLE),1)
include Makefile.chirpstackmesh.mk
endif
