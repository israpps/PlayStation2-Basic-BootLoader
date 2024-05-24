BIN2S = @bin2c
vpath %.irx embed/iop/
vpath %.irx $(PS2SDK)/iop/irx/
IRXTAG = $(notdir $(addsuffix _irx, $(basename $<)))

# ---{ IOP BINARIES }--- #
$(EE_ASM_DIR)ioprp.c: $(IOPRP_SOURCE) | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ioprp

$(EE_ASM_DIR)sio2man_irx.c: sio2man.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ sio2man_irx

$(EE_ASM_DIR)mcman_irx.c: dongleman.irx | $(EE_ASM_DIR)
	echo $(BIN2S) $< $@ mcman_irx
	$(BIN2S) $< $@ mcman_irx

$(EE_ASM_DIR)mcserv_irx.c: mcserv.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mcserv_irx

$(EE_ASM_DIR)padman_irx.c: freepad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ padman_irx

$(EE_ASM_DIR)usbd_irx.c: usbd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usbd_irx

$(EE_ASM_DIR)usbhdfsd_irx.c: usbhdfsd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usb_mass_irx

$(EE_ASM_DIR)bdm_irx.c: bdm.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdm_irx

$(EE_ASM_DIR)bdmfs_fatfs_irx.c: bdmfs_fatfs.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ bdmfs_fatfs_irx

$(EE_ASM_DIR)usbmass_bd_irx.c: usbmass_bd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ usbmass_bd_irx

$(EE_ASM_DIR)mx4sio_bd.c: mx4sio_bd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ mx4sio_bd_irx

#HDD
$(EE_ASM_DIR)poweroff_irx.c: poweroff.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ poweroff_irx

$(EE_ASM_DIR)iomanx_irx.c: iomanX.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ iomanX_irx

$(EE_ASM_DIR)filexio_irx.c: fileXio.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ fileXio_irx

$(EE_ASM_DIR)ps2dev9_irx.c: ps2dev9.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2dev9_irx

$(EE_ASM_DIR)ps2atad_irx.c: ps2atad.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2atad_irx

$(EE_ASM_DIR)ps2hdd_irx.c: ps2hdd-osd.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2hdd_irx

$(EE_ASM_DIR)ps2fs_irx.c: ps2fs.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2fs_irx
#HDD

$(EE_ASM_DIR)ps2ip_irx.c: ps2ip-nm.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ ps2ip_irx

$(EE_ASM_DIR)udptty_irx.c: udptty.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ $(IRXTAG)

$(EE_ASM_DIR)netman_irx.c: netman.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ $(IRXTAG)

$(EE_ASM_DIR)smap_irx.c: smap.irx | $(EE_ASM_DIR)
	$(BIN2S) $< $@ $(IRXTAG)


# ---{ EMBEDDED RESOURCES }--- #
$(EE_ASM_DIR)icon_sys_A.c: embed/icons/icon_A.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_A

$(EE_ASM_DIR)icon_sys_J.c: embed/icons/icon_J.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_J

$(EE_ASM_DIR)icon_sys_C.c: embed/icons/icon_C.sys | $(EE_ASM_DIR)
	$(BIN2S) $< $@ icon_sys_C
