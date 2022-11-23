#include <stdio.h>
#include <osd_config.h>
#include <kernel.h>

#include "OSDInit.h"
#include "OSDConfig.h"

static OSDConfig1_t osdConfigPS1;
static OSDConfig2_t osdConfigPS2;
static u8 ps1drvConfig;
static int timezone = 0x80; // This (as well as related checks) seems to be 0x7F in older versions.

int OSDConfigLoad(void)
{
    int result;
    result = OSDLoadConfigFromNVM(&osdConfigPS1, &osdConfigPS2);
    ps1drvConfig = osdConfigPS1.data[0] & 0x11;

    return result;
}

void OSDConfigSave(u8 invalid)
{
    osdConfigPS1.data[0] = ps1drvConfig & 0x11;
    OSDSaveConfigToNVM(&osdConfigPS1, &osdConfigPS2, invalid);
}

void OSDConfigApply(void)
{
    int language;
    ConfigParam config;
    Config2Param config2;

    config.spdifMode = OSDConfigGetSPDIF();
    config.screenType = OSDConfigGetScreenType();
    config.videoOutput = OSDConfigGetVideoOutput();
    config.japLanguage = 1;
    config.ps1drvConfig = ps1drvConfig;
    config.version = 2;
    language = OSDConfigGetLanguage();
    config.language = (language <= LANGUAGE_PORTUGUESE) ? language : LANGUAGE_ENGLISH;
    config.timezoneOffset = OSDConfigGetTimezoneOffset();

    SetOsdConfigParam(&config);
    GetOsdConfigParam(&config);

    /*  Not supported by unpatched protokernels.
        Older OSD2 browser versions only deal with 2 bytes, and do not set the format, version and language fields.    */
    GetOsdConfigParam2(&config2, 4, 0);
    if (config2.format < 2)
        config2.format = 2;

    config2.daylightSaving = OSDConfigGetDaylightSaving();
    config2.timeFormat = OSDConfigGetTimeFormat();
    config2.dateFormat = OSDConfigGetDateFormat();

    config2.version = 2;
    config2.language = OSDConfigGetLanguage();

    SetOsdConfigParam2(&config2, 4, 0);
}

// Getter/Setter functions
int OSDConfigGetSPDIF(void)
{
    return osdConfigPS2.spdifMode;
}

int OSDConfigSetSPDIF(int mode)
{
    osdConfigPS2.spdifMode = mode;
    return osdConfigPS2.spdifMode;
}

int OSDConfigGetScreenType(void)
{
    return osdConfigPS2.screenType < 3 ? osdConfigPS2.screenType : TV_SCREEN_43;
}

int OSDConfigSetScreenType(int type)
{
    if (osdConfigPS2.screenType >= 3 && type == TV_SCREEN_43)
        return TV_SCREEN_43;

    osdConfigPS2.screenType = type;
    return osdConfigPS2.screenType;
}

int OSDConfigGetVideoOutput(void)
{
    return osdConfigPS2.videoOutput;
}

int OSDConfigSetVideoOutput(int type)
{
    osdConfigPS2.videoOutput = type;
    return osdConfigPS2.videoOutput;
}

// Old system, for version = 1
/* int OSDConfigGetLanguage(void)
{
    if (OSDGetConsoleRegion() == OSD_REGION_JAPAN)
    { // Domestic
        return ((osdConfigPS2.language ^ 1) ? LANGUAGE_JAPANESE : LANGUAGE_ENGLISH);
    }
    else
    { // Export
        if (osdConfigPS2.language == LANGUAGE_JAPANESE)
        { // Export sets cannot have Japanese set as a language
            osdConfigPS2.language = LANGUAGE_ENGLISH;
            return osdConfigPS2.language;
        }

        return ((osdConfigPS2.language > 0 && osdConfigPS2.language < 8) ? osdConfigPS2.language : LANGUAGE_ENGLISH);
    }
} */

int OSDConfigGetLanguage(void)
{
    int region, DefaultLanguage;

    region = OSDGetRegion();
    DefaultLanguage = OSDGetDefaultLanguage();
    if (region != OSD_REGION_JAPAN) { // Export sets cannot have Japanese set as a language
        if (osdConfigPS2.language == LANGUAGE_JAPANESE) {
            osdConfigPS2.language = DefaultLanguage;
            return osdConfigPS2.language;
        }
    }

    return (OSDIsLanguageValid(region, osdConfigPS2.language) >= 0 ? osdConfigPS2.language : DefaultLanguage);
}

int OSDConfigSetLanguage(int language)
{
    if (OSDIsLanguageValid(OSDGetRegion(), osdConfigPS2.language) < 0) {
        if (OSDGetDefaultLanguage() == language)
            return language;
    }

    osdConfigPS2.language = language;
    return osdConfigPS2.language;
}

int OSDConfigGetLanguageRaw(void)
{
    return osdConfigPS2.language;
}

int OSDConfigSetLanguageRaw(int language)
{
    osdConfigPS2.language = language;
    return osdConfigPS2.language;
}

int OSDConfigGetRcGameFunction(void)
{
    return osdConfigPS2.rcGameFunction;
}

int OSDConfigSetRcGameFunction(int value)
{
    osdConfigPS2.rcGameFunction = value;
    return osdConfigPS2.rcGameFunction;
}

int OSDConfigGetRcEnabled(void)
{
    return osdConfigPS2.rcEnabled;
}

int OSDConfigSetRcEnabled(int value)
{
    osdConfigPS2.rcEnabled = value;
    return osdConfigPS2.rcEnabled;
}

int OSDConfigGetRcSupported(void)
{
    return osdConfigPS2.rcSupported;
}

int OSDConfigSetRcSupported(int value)
{
    osdConfigPS2.rcSupported = value;
    return osdConfigPS2.rcSupported;
}

int OSDConfigGetDVDPProgressive(void)
{
    return osdConfigPS2.dvdpProgressive;
}

int OSDConfigSetDVDPProgressive(int value)
{
    osdConfigPS2.dvdpProgressive = value;
    return osdConfigPS2.dvdpProgressive;
}

int OSDConfigGetTimezoneOffset(void)
{
    return osdConfigPS2.timezoneOffset;
}

int OSDConfigSetTimezoneOffset(int offset)
{
    osdConfigPS2.timezoneOffset = offset;
    return osdConfigPS2.timezoneOffset;
}

int OSDConfigGetTimezone(void)
{
    if (osdConfigPS2.timezone >= 0x80) {
        timezone = osdConfigPS2.timezone;
        return 0x80;
    }

    return osdConfigPS2.timezone;
}

int OSDConfigSetTimezone(int value)
{
    if (value == 0x80) {
        osdConfigPS2.timezone = timezone;
        return 0x80;
    } else {
        osdConfigPS2.timezone = value;
        return osdConfigPS2.timezone;
    }
}

int OSDConfigGetDaylightSaving(void)
{
    return osdConfigPS2.daylightSaving;
}

int OSDConfigSetDaylightSaving(int daylightSaving)
{
    osdConfigPS2.daylightSaving = daylightSaving;
    return osdConfigPS2.daylightSaving;
}

int OSDConfigGetTimeFormat(void)
{
    return osdConfigPS2.timeFormat;
}

int OSDConfigSetTimeFormat(int timeFormat)
{
    osdConfigPS2.timeFormat = timeFormat;
    return osdConfigPS2.timeFormat;
}

int OSDConfigGetDateFormat(void)
{
    return osdConfigPS2.dateFormat;
}

int OSDConfigSetDateFormat(int dateFormat)
{
    osdConfigPS2.dateFormat = dateFormat;
    return osdConfigPS2.dateFormat;
}

// Unofficially added, for uniformity.
int OSDConfigGetPSConfig(void)
{
    return ((int)ps1drvConfig);
}

int OSDConfigSetPSConfig(int config)
{
    ps1drvConfig = (u8)config;
    return ((int)ps1drvConfig);
}
