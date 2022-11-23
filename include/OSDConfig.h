
/**
 * @brief Loads the OSD configuration from EEPROM
 * @returns 0 on success. Non-zero = configuration not initialized
 */
int OSDConfigLoad(void);

/**
 * @brief Saves the OSD configuration to EEPROM
 * @param invalid Set invalid=0 to indicate that the OSD settings are initialized (not invalid). Tells the browser to not display first boot screen.
 * @note calling this function does not automatically apply the settings (call OSDConfigApply).
 */
void OSDConfigSave(u8 invalid);

/**
 * @brief Applies OSD configuration (saves settings into the EE kernel)
 */
void OSDConfigApply(void);

// Getter/Setter functions
int OSDConfigGetSPDIF(void);
int OSDConfigSetSPDIF(int mode);
int OSDConfigGetScreenType(void);
int OSDConfigSetScreenType(int type);
int OSDConfigGetVideoOutput(void);
int OSDConfigSetVideoOutput(int type);
/*    Note: PlayStation 2 consoles officially have a list of supported languages.
        Use OSDGetConsoleRegion() to get the console's OSD region, which can determine what it was officially meant to offer.
            OSD_REGION_JAPAN:                   Japanese, English
            OSD_REGION_USA, OSD_REGION_EUROPE:  English, French, Spanish, German, Italian, Dutch, Portuguese
            OSD_REGION_CHINA:                   English, Simplified Chinese*
            OSD_REGION_RUSSIA:                  English, Russian*
            OSD_REGION_KOREA:                   English, Korean*
            OSD_REGION_ASIA:                    English, Traditional Chinese*

        *These languages were added later on.
        Even if the user selects these language, English will be saved as the language setting (refer to OSDConfigApply), which is obtained by games with GetOsdConfigParam.
        The true language selected is saved with SetOsdConfigParam2, but no library has been found to support this field. */
int OSDConfigGetLanguage(void);
int OSDConfigSetLanguage(int language);
int OSDConfigGetLanguageRaw(void);
int OSDConfigSetLanguageRaw(int language);

/**
 * @returns Whether the Remote Control Game Function is enabled or not
 */
int OSDConfigGetRcGameFunction(void);

/**
 * @brief change the remote control game value
 * @param value 1 = enabled, 0 = disabled
 */
int OSDConfigSetRcGameFunction(int value);

/*  I am not entirely sure what this is.
    In ROM v2.20, it seems to have a relationship to a menu that is labeled "Console" and has the text "Remote Control,Off,On".
    However, I could not find the necessary conditions for accessing this menu.

    In the HDD Browser, the "Remote Control,Off,On" menu text exists, but does not seem to be used.

    On my SCPH-77006, this setting is disabled, although the other two remote control-related settings are enabled.    */
int OSDConfigGetRcEnabled(void);
int OSDConfigSetRcEnabled(int value);

/*  This should not be adjusted, other than by the code shown in main().
    This field indicates whether the PlayStation 2 supports the remote control.
    If enabled, allow the user to enable/disable the remote control game function. */
/**
 * @brief check if the console supports remote control
 * @returns 1: true, 0: false
*/
int OSDConfigGetRcSupported(void);

/**
 * @brief manipulate the osdconfig filed wich defines if the remote control is supported
 * @param value the value to be assigned
 * @returns the valueof the osdconfig field that this function manipulates
*/
int OSDConfigSetRcSupported(int value);

/*  If the user chose to disable the DVD Player progressive scan setting,
    it is actually disabled when a DVD Video disc is played because Sony probably wanted the setting to only bind if the user played a DVD.
    The browser only allowed this setting to be disabled, by only showing the menu option for it if it was enabled by the DVD Player. */
int OSDConfigGetDVDPProgressive(void);      // Whether the DVD Player's progressive-scan option is enabled.
int OSDConfigSetDVDPProgressive(int value); // 0 = progressive disabled (default), 1 = progressive enabled

int OSDConfigGetTimezoneOffset(void);
int OSDConfigSetTimezoneOffset(int offset);

int OSDConfigGetTimezone(void);      // It appears to be related to the timezone offset field (I guess, the timezone itself).
int OSDConfigSetTimezone(int value); // 0x80 seems to be for "Unknown", hence valid values should be from 0x00-0x7F. The browser seems to have code that checks this against the timezone offset.

int OSDConfigGetDaylightSaving(void);
int OSDConfigSetDaylightSaving(int daylightSaving);
int OSDConfigGetTimeFormat(void);
int OSDConfigSetTimeFormat(int timeFormat);
int OSDConfigGetDateFormat(void);
int OSDConfigSetDateFormat(int dateFormat);
/*  Unofficially added, for uniformity.
    Only two bits are defined (0x11).

    One is for texture (standard = 0, smooth = 1),
    while the other is for disc speed (standard = 0, fast = 1)    */
int OSDConfigGetPSConfig(void);
int OSDConfigSetPSConfig(int config);
