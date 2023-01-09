define HEADER 
 _/_/_/_/_/_/_/_/_/_/
|.                  |/ 
|    _____  _____   |/ 
|   |      |        |/ 
|   |_____ |_____   |/ 
|   |      |        |/ 
|   |_____ |_____   |/ 
|                   |/ 
|___________________|/ SIOCookie

Coded by El_isra. Idea from uyjulian

endef
export HEADER

EE_LIB = SIOCookie.a

# KERNEL_NOPATCH = 1
# NEWLIB_NANO = 1

EE_OBJS = src/SIOCookie.o
EE_CFLAGS += -fdata-sections -ffunction-sections
EE_LDFLAGS += -Wl,--gc-sections
EE_INCS += -Iinclude

ifeq ($(DEBUG), 1)
  EE_CFLAGS += -DDEBUG -O0 -g
else 
  EE_CFLAGS += -Os
  EE_LDFLAGS += -s
endif

all: $(EE_LIB)
	@echo "$$HEADER"

clean:
	rm -rf $(EE_OBJS) $(EE_LIB)

rebuild: clean all

# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
