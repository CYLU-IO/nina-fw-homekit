PROJECT_NAME := cordblock-nina

EXTRA_COMPONENT_DIRS := $(PWD)/arduino
EXTRA_COMPONENT_DIRS += $(PWD)/arduino/libraries/ #essential
EXTRA_COMPONENT_DIRS += $(PWD)/arduino/libraries/Homekit

CPPFLAGS += -DARDUINO

ifeq ($(RELEASE),1)
CFLAGS += -DNDEBUG -DCONFIG_FREERTOS_ASSERT_DISABLE -Os -DLOG_LOCAL_LEVEL=0
CPPFLAGS += -DNDEBUG -Os
endif

include $(IDF_PATH)/make/project.mk

#firmware: all
#	python combine.py

#.PHONY: firmware
