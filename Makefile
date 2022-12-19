.SILENT:
define HEADER
__________  _________________   ____________________.____     
\______   \/   _____/\_____  \  \______   \______   \    |    
 |     ___/\_____  \  /  ____/   |    |  _/|    |  _/    |    
 |    |    /        \/       \   |    |   \|    |   \    |___ 
 |____|   /_______  /\_______ \  |______  /|______  /_______ \\
                  \/         \/         \/        \/        \/
		PlayStation2 Basic BootLoader - By El_isra
endef
export HEADER


# ---{BUILD CFG}--- #

PSX ?= 0 # PSX DESR support
PROHBIT_DVD_0100 ?= 0 # prohibit the DVD Players v1.00 and v1.01 from being booted.
XCDVD_READKEY ?= 0 # Enable the newer sceCdReadKey checks, which are only supported by a newer CDVDMAN module.

# Just one print should be enabled
SCR_PRINT ?= 0
EE_SIO ?= 0
PCSX2 ?= 1

# Related to binary size reduction
KERNEL_NOPATCH ?= 1 
NEWLIB_NANO ?= 1
DUMMY_TIMEZONE ?= 0
DUMMY_LIBC_INIT ?= 1

# ---{ VERSIONING }--- #

VERSION = 1
SUBVERSION = 0
PATCHLEVEL = 0
STATUS = Beta

# ---{ EXECUTABLES }--- #

BINDIR ?= bin/
BASENAME ?= PS2BBL
EE_BIN = $(BINDIR)$(BASENAME).ELF
EE_BIN_STRIPPED = $(BINDIR)stripped_$(BASENAME).ELF
EE_BIN_PACKED = $(BINDIR)COMPRESSED_$(BASENAME).ELF
EE_BIN_ENCRYPTED = $(BINDIR)$(BASENAME).KELF

# ---{ OBJECTS & STUFF }--- #

EE_OBJS_DIR = obj/
EE_SRC_DIR = src/
EE_ASM_DIR = asm/

EE_OBJS = main.o \
          pad.o util.o elf.o timer.o ps2.o ps1.o dvdplayer.o \
          modelname.o libcdvd_add.o OSDHistory.o OSDInit.o OSDConfig.o \
          $(EMBEDDED_STUFF)

EMBEDDED_STUFF = icon_sys_A.o icon_sys_J.o icon_sys_C.o

EE_CFLAGS = -Wall
EE_CFLAGS += -fdata-sections -ffunction-sections
EE_LDFLAGS += -L$(PS2SDK)/ports/lib
EE_LDFLAGS += -Wl,--gc-sections -Wno-sign-compare
EE_LIBS += -ldebug -lmc -lps2_drivers -lpatches
EE_INCS += -Iinclude -I$(PS2SDK)/ports/include
EE_CFLAGS += -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DPATCHLEVEL=\"$(PATCHLEVEL)\" -DSTATUS=\"$(STATUS)\"

# ---{ CONDITIONS }--- #

ifeq ($(PSX),1)
  BASENAME = PSXBBL
  EE_CFLAGS += -DPSX=1
  EE_OBJS += scmd_add.o ioprp.o
  EE_LIBS += -lxcdvd -liopreboot
else
  EE_LIBS += -lcdvd
endif

ifeq ($(DEBUG), 1)
  EE_CFLAGS += -DDEBUG -O0 -g
  EE_LIBS += -lelf-loader
else 
  EE_CFLAGS += -Os
  EE_LDFLAGS += -s
  EE_LIBS += -lelf-loader-nocolour
  SCR_PRINT = 0 # not a debug build? Make sure DPRINTF is disabled 
  EE_SIO = 0
  PCSX2 = 0
endif

ifeq ($(DUMMY_TIMEZONE), 1)
  EE_CFLAGS += -DDUMMY_TIMEZONE
endif

ifdef COMMIT_HASH
  EE_CFLAGS += -DCOMMIT_HASH=\"$(COMMIT_HASH)\"
else
  EE_CFLAGS += -DCOMMIT_HASH=\"UNKNOWN\"
endif

ifeq ($(DUMMY_LIBC_INIT), 1)
  EE_CFLAGS += -DDUMMY_LIBC_INIT
endif

ifeq ($(KERNEL_NOPATCH), 1)
  EE_CFLAGS += -DKERNEL_NOPATCH
endif

ifeq ($(SCR_PRINT), 1)
  EE_CFLAGS += -DSCR_PRINT
endif

ifeq ($(EE_SIO), 1)
  EE_CFLAGS += -DEE_SIO
endif

ifeq ($(PCSX2), 1)
  EE_CFLAGS += -DPCSX2
endif

ifeq ($(XCDVD_READKEY),1)
  EE_CFLAGS += -DXCDVD_READKEY=1
endif

ifeq ($(PROHBIT_DVD_0100),1)
  EE_CFLAGS += -DPROHBIT_DVD_0100=1
endif

# ---{ RECIPES }--- #
all:
	$(MAKE) $(EE_BIN)

greeting:
	@echo built PS2BBL PSX=$(PSX)
	@echo PROHBIT_DVD_0100=$(PROHBIT_DVD_0100), XCDVD_READKEY=$(XCDVD_READKEY)
	@echo KERNEL_NOPATCH=$(KERNEL_NOPATCH), NEWLIB_NANO=$(NEWLIB_NANO)
	@echo binaries dispatched to $(BINDIR)
ifeq ($(PCSX2), 1)
	@echo with printf()
endif
ifeq ($(EE_SIO), 1)
	@echo with EE SIO
endif
ifeq ($(SCR_PRINT), 1)
	@echo with scr_printf()
endif

release: clean
	$(MAKE) $(EE_BIN_PACKED)
	$(MAKE) greeting
	@echo "$$HEADER"

clean:
	@echo cleaning...
	@echo - Objects
	@rm -rf $(EE_OBJS)
	@echo - Objects folders 
	@rm -rf $(EE_OBJS_DIR) $(EE_ASM_DIR) $(BINDIR)
	@echo  "\n\n\n"

$(EE_BIN_STRIPPED): $(EE_BIN)
	@echo " -- Stripping"
	$(EE_STRIP) -o $@ $<

$(EE_BIN_PACKED): $(EE_BIN_STRIPPED)
	@echo " -- Compressing"
	ps2-packer $< $@ > /dev/null

$(EE_BIN_ENCRYPTED): $(EE_BIN_PACKED)
	@echo " -- Encrypting..."
	thirdparty/kelftool encrypt fmcb $< $@ 

# move OBJ to folder and search source on src/, borrowed from OPL makefile

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%) # remap all EE_OBJ to obj subdir

$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_ASM_DIR):
	@mkdir -p $@

$(BINDIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	@echo "  - $@"
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.s | $(EE_OBJS_DIR)
	@echo "  - $@"
	$(EE_AS) $(EE_ASFLAGS) $< -o $@
#

celan: clean # repetitive typo that i have when quicktyping
kelf: $(EE_BIN_ENCRYPTED) # alias of KELF creation
# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
include embed.make
