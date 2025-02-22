BIN2S = @bin2c
vpath %.irx embed/iop/
vpath %.irx $(PS2SDK)/iop/irx/
IRXTAG = $(notdir $(addsuffix _irx, $(basename $<)))

# ---{ IOP BINARIES }--- #
$(EE_ASM_DIR)ioprp.c: embed/ioprp.img | $(EE_ASM_DIR)
	$(BIN2S) $< $@ psx_ioprp

$(EE_ASM_DIR)padman_irx.c: freepad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ padman_irx

$(EE_ASM_DIR)iomanx_irx.c: iomanX.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ iomanX_irx

$(EE_ASM_DIR)filexio_irx.c: fileXio.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ fileXio_irx

$(EE_ASM_DIR)ps2hdd_irx.c: ps2hdd-osd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2hdd_irx

$(EE_ASM_DIR)ps2ip_irx.c: ps2ip-nm.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2ip_irx

$(EE_ASM_DIR)%_irx.c: %.irx
	$(DIR_GUARD)
	$(BIN2S) $< $@ $(IRXTAG)

embed/iop/mmceman.irx:
	$(info - - Downloading MMCEMAN Driver)
	wget -q https://github.com/israpps/wLaunchELF_ISR/raw/refs/heads/master/iop/__precompiled/mmceman.irx -O $@

# ---{ EMBEDDED RESOURCES }--- #
$(EE_ASM_DIR)icon_sys_A.c: embed/icons/icon_A.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_A

$(EE_ASM_DIR)icon_sys_J.c: embed/icons/icon_J.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_J

$(EE_ASM_DIR)icon_sys_C.c: embed/icons/icon_C.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_C
