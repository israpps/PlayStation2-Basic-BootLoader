$(EE_ASM_DIR)sio2man_irx.s: $(PS2SDK)/iop/irx/sio2man.irx | $(EE_ASM_DIR)
	@bin2s $< $@ sio2man_irx

$(EE_ASM_DIR)mcman_irx.s: $(PS2SDK)/iop/irx/mcman.irx | $(EE_ASM_DIR)
	@bin2s $< $@ mcman_irx

$(EE_ASM_DIR)mcserv_irx.s: $(PS2SDK)/iop/irx/mcserv.irx | $(EE_ASM_DIR)
	@bin2s $< $@ mcserv_irx

$(EE_ASM_DIR)padman_irx.s: $(PS2SDK)/iop/irx/freepad.irx | $(EE_ASM_DIR)
	@bin2s $< $@ padman_irx

$(EE_ASM_DIR)usbd_irx.s: $(PS2SDK)/iop/irx/usbd.irx | $(EE_ASM_DIR)
	@bin2s $< $@ usbd_irx

ifeq ($(NO_BDM),1)

$(EE_ASM_DIR)usbhdfsd_irx.s: $(PS2SDK)/iop/irx/usbhdfsd.irx | $(EE_ASM_DIR)
	@bin2s $< $@ usb_mass_irx

else

$(EE_ASM_DIR)bdm_irx.s: $(PS2SDK)/iop/irx/bdm.irx | $(EE_ASM_DIR)
	@bin2s $< $@ bdm_irx

$(EE_ASM_DIR)bdmfs_fatfs_irx.s: $(PS2SDK)/iop/irx/bdmfs_fatfs.irx | $(EE_ASM_DIR)
	@bin2s $< $@ bdmfs_fatfs_irx

$(EE_ASM_DIR)usbmass_bd_irx.s: $(PS2SDK)/iop/irx/usbmass_bd.irx | $(EE_ASM_DIR)
	@bin2s $< $@ usbmass_bd_irx

endif

$(EE_ASM_DIR)loader_elf.s: modules/ELF_LOADER/loader.elf | $(EE_ASM_DIR)
	@bin2s $< $@ loader_elf

$(EE_ASM_DIR)ioprp.s: embed/ioprp.img | $(EE_ASM_DIR)
	@bin2s $< $@ psx_ioprp

$(EE_ASM_DIR)icon_sys_A.s : embed/icons/icon_A.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_A

$(EE_ASM_DIR)icon_sys_J.s : embed/icons/icon_J.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_J

$(EE_ASM_DIR)icon_sys_C.s : embed/icons/icon_C.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_C