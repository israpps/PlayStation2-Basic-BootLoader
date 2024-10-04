// Initialize add-on functions. Currently only retrieves the MECHACON's version to determine what sceCdAltGetRegionParams() should do.
int cdInitAdd(void);

/*
 *  Provides an equivalent of the sceCdGetRegionParams function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
 *  It's supported by only slimline consoles, and returns regional information (e.g. MECHACON version, MG region mask, DVD player region letter etc.).
 */
int custom_sceCdReadRegionParams(u8 *data, u32 *stat);

/*
 *  Provides an equivalent of the sceCdReadPS1BootParam function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
 *  It's supported by only slimline consoles, and returns the boot path for the inserted PlayStation disc.
 */
int custom_sceCdReadPS1BootParam(char *param, u32 *stat);

// Obtain model number from EEPROM via the mechacon SCMD 0x17
int sceCdRM(char *ModelName, u32 *stat);

// Provides an equivalent of the sceCdBootCertify function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
int sceCdBootCertify(const u8 *data);

// TODO: Not implemented.
// int sceCdAltRcBypassCtl(int bypass, u32 *stat);
// int sceCdAltBootCertify(const u8 *data);
// int sceCdAltRM(char *ModelName, u32 *stat);
