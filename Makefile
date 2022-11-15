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

#BUILD CFG
PSX ?= 0 # PSX DESR support
KERNEL_NOPATCH ?= 1
NEWLIB_NANO ?= 1
HAS_EMBEDDED_IRX ?= 0
PROHBIT_DVD_0100 ?= 0 #Enable to prohibit the DVD Players v1.00 and v1.01  from being booted.
XCDVD_READKEY ?= 0 #Enable to enable the newer sceCdReadKey checks, which are only supported by a newer CDVDMAN module.
#--

EE_BIN = PS2BBL.ELF
EE_BIN_STRIPPED = stripped_$(EE_BIN)
EE_BIN_PACKED = COMPRESSED_$(EE_BIN)

EE_OBJS_DIR = obj/
EE_SRC_DIR = src/
EE_ASM_DIR = asm/


IOP_OBJS = sio2man_irx.o mcman_irx.o mcserv_irx.o padman_irx.o

ifeq ($(HAS_EMBEDDED_IRX),1)
	IOP_OBJS += usbd.o bdm_irx.o bdmfs_fatfs_irx.o usbmass_bd_irx.o
	EE_CFLAGS += -DHAS_EMBEDDED_IRX
endif

EE_OBJS = main.o \
		pad.o util.o elf.o timer.o ps2.o ps1.o dvdplayer.o modelname.o libcdvd_add.o OSDHistory.o OSDInit.o OSDConfig.o \
		icon_sys_A.o icon_sys_J.o icon_sys_C.o \
		loader_elf.o \
		$(IOP_OBJS)

EE_CFLAGS = -Os -DNEWLIB_PORT_AWARE -G0 -Wall
EE_CFLAGS += -fdata-sections -ffunction-sections
# EE_LDFLAGS += -nodefaultlibs -Wl,--start-group -lc_nano -lps2sdkc -lkernel-nopatch -Wl,--end-group
EE_LDFLAGS += -s
EE_LDFLAGS += -Wl,--gc-sections -Wno-sign-compare
EE_LIBS = -ldebug -lc -lmc -lpadx -lpatches -lkernel
EE_INCS += -Iinclude

ifeq ($(PSX),1)
	EE_CFLAGS += -DPSX=1
	EE_OBJS += scmd_add.o ioprp.o
	EE_LIBS += -lxcdvd -liopreboot
else
	EE_LIBS += -lcdvd
endif

ifeq ($(DUMMY_TIMEZONE), 1)
   EE_CFLAGS += -DDUMMY_TIMEZONE
endif

ifeq ($(DUMMY_LIBC_INIT), 1)
   EE_CFLAGS += -DDUMMY_LIBC_INIT
endif

ifeq ($(XCDVD_READKEY),1)
	EE_CFLAGS += -DXCDVD_READKEY=1
endif

ifeq ($(PROHBIT_DVD_0100),1)
	EE_CFLAGS += -DPROHBIT_DVD_0100=1
endif

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

all: greeting 
	$(MAKE) $(EE_BIN)

greeting:
	@echo building PS2BBL PSX=$(PSX), EMBEDDED_IRX= $(HAS_EMBEDDED_IRX)
	@echo KERNEL_NOPATCH=$(KERNEL_NOPATCH), NEWLIB_NANO=$(NEWLIB_NANO)

release: $(EE_BIN_PACKED)
	@echo "$$HEADER"

clean:
	@echo cleaning...
	@echo - Objects
	@rm -rf $(EE_OBJS)
	@echo - Objects folders 
	@rm -rf $(EE_OBJS_DIR) $(EE_ASM_DIR)
	@echo -- ELF loader
	$(MAKE) -C modules/ELF_LOADER/ clean
	@echo - ELF programs
	@rm -rf $(EE_BIN) $(EE_BIN_PACKED) $(EE_BIN_STRIPPED)

celan: clean #repetitive typo that i have when quicktyping

cleaniop:
	@echo cleaning only embedded IOP binaries
	rm -rf $(IOP_OBJS)

$(EE_BIN_STRIPPED): $(EE_BIN)
	@echo " -- Stripping"
	$(EE_STRIP) -o $@ $<

$(EE_BIN_PACKED): $(EE_BIN_STRIPPED)
	@echo " -- Compressing"
	ps2-packer $< $@ > /dev/null

modules/ELF_LOADER/loader.elf: modules/ELF_LOADER/
	@echo -- ELF Loader
	$(MAKE) -C $<

# move OBJ to folder and search source on src/, borrowed from OPL makefile
$(EE_OBJS_DIR):
	@mkdir -p $@

$(EE_ASM_DIR):
	@mkdir -p $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c | $(EE_OBJS_DIR)
	@echo "  - $@"
	@$(EE_CC) $(EE_CFLAGS) $(EE_INCS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.s | $(EE_OBJS_DIR)
	@echo "  - $@"
	@$(EE_AS) $(EE_ASFLAGS) $< -o $@
#

# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
include Makefile.embed
