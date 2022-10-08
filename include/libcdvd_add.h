int cdInitAdd(void);
int custom_sceCdReadRegionParams(u8 *data, u32 *stat);
int custom_sceCdReadRegionParams(u8 *data, u32 *stat);
int sceCdAltBootCertify(const u8 *data);
int sceCdAltRM(char *ModelName, u32 *stat);
int sceCdAltRcBypassCtl(int bypass, u32 *stat); // TODO: Not implemented.
int custom_sceCdReadPS1BootParam(char *param, u32 *stat);
