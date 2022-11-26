$(EE_ASM_DIR)ioprp.s: embed/ioprp.img | $(EE_ASM_DIR)
	@bin2s $< $@ psx_ioprp

$(EE_ASM_DIR)icon_sys_A.s : embed/icons/icon_A.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_A

$(EE_ASM_DIR)icon_sys_J.s : embed/icons/icon_J.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_J

$(EE_ASM_DIR)icon_sys_C.s : embed/icons/icon_C.sys | $(EE_ASM_DIR)
	@bin2s $< $@ icon_sys_C