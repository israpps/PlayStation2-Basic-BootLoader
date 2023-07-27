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
DEBUG ?= 0
PSX ?= 0 # PSX DESR support
HDD ?= 0 #wether to add internal HDD support
MX4SIO ?= 0
PROHBIT_DVD_0100 ?= 0 # prohibit the DVD Players v1.00 and v1.01 from being booted.
XCDVD_READKEY ?= 0 # Enable the newer sceCdReadKey checks, which are only supported by a newer CDVDMAN module.

PRINTF ?= NONE

HOMEBREW_IRX ?= 0
FILEXIO_NEED ?= 0 #if we need filexio and imanx loaded for other features (HDD, mx4sio, etc)
HAS_LOCAL_IRX ?= 0 # whether to embed or not non vital IRX (wich will be loaded from memcard files)

# Related to binary size reduction
KERNEL_NOPATCH = 1 
NEWLIB_NANO = 1
DUMMY_TIMEZONE = 1
DUMMY_LIBC_INIT = 1

# ---{ VERSIONING }--- #

VERSION = 1
SUBVERSION = 2
PATCHLEVEL = 0
STATUS = Beta

# ---{ EXECUTABLES }--- #

BINDIR ?= bin/
BASENAME ?= PS2BBL
EE_BIN = $(BINDIR)$(BASENAME).ELF
EE_BIN_STRIPPED = $(BINDIR)stripped_$(BASENAME).ELF
EE_BIN_PACKED = $(BINDIR)COMPRESSED_$(BASENAME).ELF
KELFTYPE ?= MC
EE_BIN_ENCRYPTED = $(BINDIR)$(BASENAME)_$(KELFTYPE).KELF

# ---{ OBJECTS & STUFF }--- #

EE_OBJS_DIR = obj/
EE_SRC_DIR = src/
EE_ASM_DIR = asm/

EE_OBJS = main.o \
          util.o elf.o timer.o ps2.o ps1.o dvdplayer.o \
          modelname.o libcdvd_add.o OSDHistory.o OSDInit.o OSDConfig.o \
          $(EMBEDDED_STUFF) \
		      $(IOP_OBJS)

EMBEDDED_STUFF = icon_sys_A.o icon_sys_J.o icon_sys_C.o

EE_CFLAGS = -Wall
EE_CFLAGS += -fdata-sections -ffunction-sections -DREPORT_FATAL_ERRORS
EE_LDFLAGS += -L$(PS2SDK)/ports/lib
EE_LDFLAGS += -Wl,--gc-sections -Wno-sign-compare
EE_LIBS += -ldebug -lmc -lpatches
EE_INCS += -Iinclude -I$(PS2SDK)/ports/include
EE_CFLAGS += -DVERSION=\"$(VERSION)\" -DSUBVERSION=\"$(SUBVERSION)\" -DPATCHLEVEL=\"$(PATCHLEVEL)\" -DSTATUS=\"$(STATUS)\"

# ---{ CONDITIONS }--- #

ifneq ($(VERBOSE), 1)
   .SILENT:
endif

ifeq ($(MX4SIO), 1)
  HOMEBREW_IRX = 1
  FILEXIO_NEED = 1
  EE_OBJS += mx4sio_bd.o
  EE_CFLAGS += -DMX4SIO
endif

ifneq ($(HOMEBREW_IRX), 0)
   $(info --- enforcing usage of homebrew IRX modules)
   USE_ROM_PADMAN = 0
   USE_ROM_MCMAN = 0
   USE_ROM_SIO2MAN = 0
else
   $(info --- using BOOT-ROM drivers)
   USE_ROM_PADMAN = 1
   USE_ROM_MCMAN = 1
   USE_ROM_SIO2MAN = 1
endif

ifeq ($(PSX), 1)
   $(info --- building with PSX-DESR support)
  BASENAME = PSXBBL
  EE_CFLAGS += -DPSX=1
  EE_OBJS += scmd_add.o ioprp.o
  EE_LIBS += -lxcdvd -liopreboot
else
  EE_LIBS += -lcdvd
endif

ifeq ($(DEBUG), 1)
   $(info --- debugging enabled)
  EE_CFLAGS += -DDEBUG -O0 -g
  EE_LIBS += -lelf-loader
else 
  EE_CFLAGS += -Os
  EE_LDFLAGS += -s
  EE_LIBS += -lelf-loader-nocolour
endif

ifeq ($(USE_ROM_PADMAN), 1)
  EE_CFLAGS += -DUSE_ROM_PADMAN
  EE_LIBS += -lpad
  EE_OBJS += pad.o
else
  EE_OBJS += pad.o padman_irx.o
  EE_LIBS += -lpadx
endif

ifeq ($(USE_ROM_MCMAN), 1)
  EE_CFLAGS += -DUSE_ROM_MCMAN
else
  EE_OBJS += mcman_irx.o mcserv_irx.o
endif

ifeq ($(USE_ROM_SIO2MAN), 1)
  EE_CFLAGS += -DUSE_ROM_SIO2MAN
else
  EE_OBJS += sio2man_irx.o
endif

ifneq ($(HAS_LOCAL_IRX), 1)
  $(info --- USB drivers will be embedded)
  EE_OBJS += usbd_irx.o bdm_irx.o bdmfs_fatfs_irx.o usbmass_bd_irx.o
  EE_CFLAGS += -DHAS_EMBEDDED_IRX
else
  $(info --- USB drivers will be loaded of memory card)
endif

ifeq ($(HDD), 1)
  $(info --- compiling with HDD support)
  EE_LIBS += -lpoweroff
  EE_OBJS += ps2fs_irx.o ps2hdd_irx.o ps2atad_irx.o ps2dev9_irx.o poweroff_irx.o
  EE_CFLAGS += -DHDD
  FILEXIO_NEED = 1
  KELFTYPE = HDD
endif

ifeq ($(FILEXIO_NEED), 1)
  $(info --- FILEXIO will be included)
  EE_CFLAGS += -DFILEXIO
  EE_LIBS += -lfileXio
  EE_OBJS += filexio_irx.o iomanx_irx.o
endif

ifeq ($(DUMMY_TIMEZONE), 1)
  EE_CFLAGS += -DDUMMY_TIMEZONE
endif

ifdef COMMIT_HASH
  EE_CFLAGS += -DCOMMIT_HASH=\"$(COMMIT_HASH)\"
else
  EE_CFLAGS += -DCOMMIT_HASH=\"$(shell git rev-parse --short HEAD)\"
endif

ifeq ($(DUMMY_LIBC_INIT), 1)
  EE_CFLAGS += -DDUMMY_LIBC_INIT
endif

ifeq ($(KERNEL_NOPATCH), 1)
  EE_CFLAGS += -DKERNEL_NOPATCH
endif

ifeq ($(PRINTF), NONE)
else ifeq ($(PRINTF), SCR)
  $(info --- SCR Printf enabled)
  EE_CFLAGS += -DSCR_PRINT
else ifeq ($(PRINTF), EE_SIO)
  $(info --- EESIO Printf enabled)
  EE_CFLAGS += -DEE_SIO_DEBUG
  EE_LIBS += -lsiocookie
  EE_OBJS += sioprintf.o
else ifeq ($(PRINTF), PRINTF)
  $(info --- Common Printf enabled)
  EE_CFLAGS += -DCOMMON_PRINTF
else ifneq ($(PRINTF),)
  $(warning UNKNOWN PRINTF REQUESTED: '$(PRINTF)')
endif

ifeq ($(XCDVD_READKEY),1)
  EE_CFLAGS += -DXCDVD_READKEY=1
endif

ifeq ($(PROHBIT_DVD_0100),1)
  EE_CFLAGS += -DPROHBIT_DVD_0100=1
endif

# ---{ RECIPES }--- #
.PHONY: greeting debug all clean kelf packed release

all: $(EE_BIN)
ifeq (DEBUG, 1)
	$(MAKE) greeting
endif
rebuild: clean packed

packed: $(EE_BIN_PACKED)

greeting:
	@echo built PS2BBL PSX=$(PSX), LOCAL_IRX=$(HAS_EMBED_IRX), DEBUG=$(DEBUG)
	@echo PROHBIT_DVD_0100=$(PROHBIT_DVD_0100), XCDVD_READKEY=$(XCDVD_READKEY)
	@echo KERNEL_NOPATCH=$(KERNEL_NOPATCH), NEWLIB_NANO=$(NEWLIB_NANO)
	@echo binaries dispatched to $(BINDIR)
	@echo printf=$(PRINTF)
	@echo $(EE_OBJS)

release: clean $(EE_BIN_PACKED)
	@rm -f $(EE_BIN_STRIPPED)
	@echo "$$HEADER"

clean:
	@echo cleaning...
	@echo - Executables
	@rm -rf $(EE_BIN) $(EE_BIN_STRIPPED) $(EE_BIN_ENCRYPTED) $(EE_BIN_PACKED)
	@echo - Object folders 
	@rm -rf $(EE_OBJS_DIR) $(EE_ASM_DIR)
	@echo  "\n"

$(EE_BIN_STRIPPED): $(EE_BIN)
	@echo " -- Stripping"
	$(EE_STRIP) -o $@ $<

$(EE_BIN_PACKED): $(EE_BIN_STRIPPED)
	@echo " -- Compressing"
ifneq ($(DEBUG),1)
	ps2-packer $< $@
else
	ps2-packer -v $< $@
endif

$(EE_BIN_ENCRYPTED): $(EE_BIN_PACKED)
	@echo " -- Encrypting ($(KELFTYPE))"
ifeq ($(KELFTYPE), MC)
	thirdparty/kelftool_dnasload.exe encrypt dnasload $< $@
else ifeq ($(KELFTYPE), HDD)
	thirdparty/kelftool_dnasload.exe encrypt fhdb $< $@
else
	$(error UNKNOWN KELF TYPE: '$(KELFTYPE)')
endif
# move OBJ to folder and search source on src/, borrowed from OPL makefile

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%) # remap all EE_OBJ to obj subdir

$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_ASM_DIR):
	@mkdir -p $@

$(BINDIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
ifneq ($(VERBOSE),1)
	@echo "  - $@"
endif
	$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.s | $(EE_OBJS_DIR)
ifneq ($(VERBOSE),1)
	@echo "  - $@"
endif
	$(EE_AS) $(EE_ASFLAGS) $< -o $@
#
analize:
	$(MAKE) rebuild DEBUG=1
	python3 thirdparty/elf-size-analize.py $(EE_BIN) -R -t mips64r5900el-ps2-elf-

celan: clean # a repetitive typo when quicktyping
kelf: $(EE_BIN_ENCRYPTED) # alias of KELF creation
# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
include embed.make
