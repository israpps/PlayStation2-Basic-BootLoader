IOP_BIN = ../sd2psxman.irx

IOP_OBJS = main.o ioplib.o imports.o

all: $(IOP_BIN)

clean:
	rm -f -r $(IOP_OBJS) $(IOP_BIN)

ifeq ($(DEBUG), 1)
  $(info -- SD2PSXMAN debug enabled)
  IOP_CFLAGS += -DDEBUG
endif

include $(PS2SDK)/Defs.make
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.iopglobal
