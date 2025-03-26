ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Python Resource Manager Helper Library
endef

INCVPATH += $(QNX_TARGET)/usr/include/python3.11
INCVPATH += $(QNX_TARGET)/usr/include/$(CPU)/python3.11
LDFLAGS  += -Wl,--copy-dt-needed-entries

NAME = pyresmgr

include $(MKFILES_ROOT)/qtargets.mk



