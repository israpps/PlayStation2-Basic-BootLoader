#include <stdio.h>
#include <fcntl.h>
#include <kernel.h>
#include <string.h>
#include <libcdvd.h>
#include "libcdvd_add.h"
#include <osd_config.h>
#include "OSDInit.h"
#include <unistd.h>

/*  Parsing of values from the EEPROM and setting them into the EE kernel
    was done in different ways, across different browser versions.

    The early browsers of ROM v1.00 and v1.01 (SCPH-10000/SCPH-15000)
    parsed the values within the EEPROM into global variables,
    which are used to set the individual fields in the OSD configuration.

    The newer browsers parsed the values into a bitfield structure,
    which does not have the same layout as the OSD configuration structure.

    Both designs had the parsing and the preparation of the OSD
    configuration data separated, presumably for clarity of code and
    to achieve low-coupling (perhaps they belonged to different modules).    */

static int ConsoleRegion = -1, ConsoleOSDRegion = -1, ConsoleOSDLanguage = -1;
static int ConsoleOSDRegionInitStatus = 0, ConsoleRegionParamInitStatus = 0; // 0 = Not init. 1 = Init complete. <0 = Init failed.
static u8 ConsoleRegionData[16] = {0, 0, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Perhaps it once used to read more configuration blocks (original capacity was 7 blocks).
static u8 OSDConfigBuffer[CONFIG_BLOCK_SIZE * 2];

// Local function prototypes
static int InitMGRegion(void);
static int ConsoleInitRegion(void);
static int ConsoleRegionParamsInitPS1DRV(const char *romver);
static int GetConsoleRegion(void);
static int CdReadOSDRegionParams(char *OSDVer);
static int GetOSDRegion(void);
static void InitOSDDefaultLanguage(int region, const char *language);
static int ReadOSDConfigPS2(OSDConfig2_t *config, const OSDConfigStore_t *OSDConfigBuffer);
static void ReadOSDConfigPS1(OSDConfig1_t *config, const OSDConfigStore_t *OSDConfigBuffer);
static void WriteOSDConfigPS1(OSDConfigStore_t *OSDConfigBuffer, const OSDConfig1_t *config);
static void WriteOSDConfigPS2(OSDConfigStore_t *OSDConfigBuffer, const OSDConfig2_t *config, u8 invalid);
static void ReadConfigFromNVM(u8 *buffer);
static void WriteConfigToNVM(const u8 *buffer);

// Directory names
static char SystemDataFolder[] = "BRDATA-SYSTEM";
static char SystemExecFolder[] = "BREXEC-SYSTEM";
static char DVDPLExecFolder[] = "BREXEC-DVDPLAYER";

char ConsoleROMVER[ROMVER_MAX_LEN];

static int InitMGRegion(void)
{
    u32 stat;

    if (ConsoleRegionParamInitStatus == 0) {
        int result;
        do {
            if ((result = custom_sceCdReadRegionParams(ConsoleRegionData, &stat)) == 0) { // Failed.
                ConsoleRegionParamInitStatus = 1;
            } else {
                if (stat & 0x100) {
                    // MECHACON does not support this function.
                    ConsoleRegionParamInitStatus = -1;
                    break;
                } else { // Status OK, but the result yielded an error.
                    ConsoleRegionParamInitStatus = 1;
                }
            }
        } while ((result == 0) || (stat & 0x80));
    }

    return ConsoleRegionParamInitStatus;
}

void OSDInitSystemPaths(void)
{
    int region;
    const char regions[CONSOLE_REGION_COUNT] = {'I', 'A', 'E', 'C'};

    region = OSDGetConsoleRegion();
    if (region >= 0 && region < CONSOLE_REGION_COUNT) {
        SystemDataFolder[1] = regions[region];
        SystemExecFolder[1] = regions[region];
        DVDPLExecFolder[1] = regions[region];
    }
}

int OSDGetMGRegion(void)
{
    return ((InitMGRegion() >= 0) ? ConsoleRegionData[8] : 0);
}

static int ConsoleInitRegion(void)
{
    GetOSDRegion();
    return InitMGRegion();
}

static int ConsoleRegionParamsInitPS1DRV(const char *romver)
{
    int result;

    if (ConsoleInitRegion() >= 0) {
        ConsoleRegionData[2] = romver[4];
        result = 1;
    } else
        result = 0;

    return result;
}

/* Present here, but not sure what it is (unused) */
/* int OSDGetROMRegion(char *out)
{
    int result;

    if (ConsoleInitRegion() >= 0)
    {
        out[4] = ConsoleRegionData[2];
        result = 1;
    }
    else
        result = 0;

    return result;
} */

int OSDGetPS1DRVRegion(char *region)
{
    int result;

    if (ConsoleInitRegion() >= 0) {
        *region = ConsoleRegionData[2];
        result = 1;
    } else
        result = 0;

    return result;
}

int OSDGetDVDPlayerRegion(char *region)
{
    int result;

    if (ConsoleInitRegion() >= 0) {
        *region = ConsoleRegionData[8];
        result = 1;
    } else
        result = 0;

    return result;
}

static int GetConsoleRegion(void)
{
    int result;

    if ((result = ConsoleRegion) < 0) {
        int fd;
        if ((fd = open("rom0:ROMVER", O_RDONLY)) >= 0) {
            char romver[16];
            read(fd, romver, sizeof(romver));
            close(fd);
            ConsoleRegionParamsInitPS1DRV(romver);

            switch (romver[4]) {
                case 'C':
                    ConsoleRegion = CONSOLE_REGION_CHINA;
                    break;
                case 'E':
                    ConsoleRegion = CONSOLE_REGION_EUROPE;
                    break;
                case 'H':
                case 'A':
                    ConsoleRegion = CONSOLE_REGION_USA;
                    break;
                case 'J':
                    ConsoleRegion = CONSOLE_REGION_JAPAN;
            }

            result = ConsoleRegion;
        } else
            result = -1;
    }

    return result;
}

static int CdReadOSDRegionParams(char *OSDVer)
{
    int result;

    if (OSDVer[4] == '?') {
        if (InitMGRegion() >= 0) {
            result = 1;
            OSDVer[4] = ConsoleRegionData[3];
            OSDVer[5] = ConsoleRegionData[4];
            OSDVer[6] = ConsoleRegionData[5];
            OSDVer[7] = ConsoleRegionData[6];
        } else
            result = 0;
    } else {
        ConsoleRegionParamInitStatus = -256;
        result = 0;
    }

    return result;
}

static int GetOSDRegion(void)
{
    char OSDVer[16];

    if (ConsoleOSDRegionInitStatus == 0 || ConsoleOSDRegion == -1) {
        int fd;
        ConsoleOSDRegionInitStatus = 1;
        if ((fd = open("rom0:OSDVER", O_RDONLY)) >= 0) {
            read(fd, OSDVer, sizeof(OSDVer));
            close(fd);
            CdReadOSDRegionParams(OSDVer);
            switch (OSDVer[4]) {
                case 'A':
                    ConsoleOSDRegion = OSD_REGION_USA;
                    break;
                case 'C':
                    ConsoleOSDRegion = OSD_REGION_CHINA;
                    break;
                case 'E':
                    ConsoleOSDRegion = OSD_REGION_EUROPE;
                    break;
                case 'H':
                    ConsoleOSDRegion = OSD_REGION_ASIA;
                    break;
                case 'J':
                    ConsoleOSDRegion = OSD_REGION_JAPAN;
                    break;
                case 'K':
                    ConsoleOSDRegion = OSD_REGION_KOREA;
                    break;
                case 'R':
                    ConsoleOSDRegion = OSD_REGION_RUSSIA;
                    break;
                default:
                    ConsoleOSDRegion = OSD_REGION_INVALID;
            }

            if (ConsoleOSDRegion != OSD_REGION_INVALID)
                InitOSDDefaultLanguage(ConsoleOSDRegion, &OSDVer[5]);
        } else
            ConsoleOSDRegion = OSD_REGION_INVALID;
    }

    return ConsoleOSDRegion;
}

static void InitOSDDefaultLanguage(int region, const char *language)
{
    if (ConsoleOSDLanguage == -1) {
        int DefaultLang = -1;
        if (language != NULL) {
            if (strncmp(language, "jpn", 3) == 0)
                DefaultLang = LANGUAGE_JAPANESE;
            else if (strncmp(language, "eng", 3) == 0)
                DefaultLang = LANGUAGE_ENGLISH;
            else if (strncmp(language, "fre", 3) == 0)
                DefaultLang = LANGUAGE_FRENCH;
            else if (strncmp(language, "spa", 3) == 0)
                DefaultLang = LANGUAGE_SPANISH;
            else if (strncmp(language, "ger", 3) == 0)
                DefaultLang = LANGUAGE_GERMAN;
            else if (strncmp(language, "ita", 3) == 0)
                DefaultLang = LANGUAGE_ITALIAN;
            else if (strncmp(language, "dut", 3) == 0)
                DefaultLang = LANGUAGE_DUTCH;
            else if (strncmp(language, "por", 3) == 0)
                DefaultLang = LANGUAGE_PORTUGUESE;
            else if (strncmp(language, "rus", 3) == 0)
                DefaultLang = LANGUAGE_RUSSIAN;
            else if (strncmp(language, "kor", 3) == 0)
                DefaultLang = LANGUAGE_KOREAN;
            else if (strncmp(language, "tch", 3) == 0)
                DefaultLang = LANGUAGE_TRAD_CHINESE;
            else if (strncmp(language, "sch", 3) == 0)
                DefaultLang = LANGUAGE_SIMPL_CHINESE;
            else
                DefaultLang = -1;
        }

        // Check if the specified language is valid for the region
        if (!OSDIsLanguageValid(region, DefaultLang)) {
            switch (region) {
                case OSD_REGION_JAPAN:
                    DefaultLang = LANGUAGE_JAPANESE;
                    break;
                case OSD_REGION_CHINA:
                    DefaultLang = LANGUAGE_SIMPL_CHINESE;
                    break;
                case OSD_REGION_RUSSIA:
                    DefaultLang = LANGUAGE_RUSSIAN;
                    break;
                case OSD_REGION_KOREA:
                    DefaultLang = LANGUAGE_KOREAN;
                    break;
                case OSD_REGION_USA:
                case OSD_REGION_EUROPE:
                case OSD_REGION_ASIA:
                default:
                    DefaultLang = LANGUAGE_ENGLISH;
            }
        }

        ConsoleOSDLanguage = DefaultLang;
    }
}

int OSDIsLanguageValid(int region, int language)
{
    switch (region) {
        case OSD_REGION_JAPAN:
            return (language == LANGUAGE_JAPANESE || language == LANGUAGE_ENGLISH) ? language : -1;
        case OSD_REGION_CHINA:
            return (language == LANGUAGE_ENGLISH || language == LANGUAGE_SIMPL_CHINESE) ? language : -1;
        case OSD_REGION_RUSSIA:
            return (language == LANGUAGE_ENGLISH || language == LANGUAGE_RUSSIAN) ? language : -1;
        case OSD_REGION_KOREA:
            return (language == LANGUAGE_ENGLISH || language == LANGUAGE_KOREAN) ? language : -1;
        case OSD_REGION_ASIA:
            return (language == LANGUAGE_ENGLISH || language == LANGUAGE_TRAD_CHINESE) ? language : -1;
        case OSD_REGION_USA:
        case OSD_REGION_EUROPE:
        default:
            return (language <= LANGUAGE_PORTUGUESE && region > OSD_REGION_JAPAN) ? language : -1;
    }
}

int OSDGetConsoleRegion(void)
{ // Default to Japan, if the region cannot be obtained.
    int result;

    result = GetConsoleRegion();

    return (result < 0 ? 0 : result);
}

int OSDGetVideoMode(void)
{
    return (GetConsoleRegion() == CONSOLE_REGION_EUROPE);
}

int OSDGetRegion(void)
{
    int region;

    if ((region = GetOSDRegion()) < 0) {
        region = OSDGetConsoleRegion();
        InitOSDDefaultLanguage(region, NULL);
    }

    return region;
}

int OSDGetDefaultLanguage(void)
{
    if (ConsoleOSDLanguage == -1)
        OSDGetRegion();

    return ConsoleOSDLanguage;
}

/*    Notes:
        Version = 0 (Protokernel consoles only) NTSC-J, 1 = ROM versions up to v1.70, 2 = v1.80 and later. 2 = support for extended languages (Osd2 bytes 3 and 4)
                    In the homebrew PS2SDK, this was previously known as the "region".
        japLanguage = 0 (Japanese, protokernel consoles only), 1 = non-Japanese (Protokernel consoles only). Newer browsers have this set always to 1.
*/
static int ReadOSDConfigPS2(OSDConfig2_t *config, const OSDConfigStore_t *OSDConfigBuffer)
{
    config->spdifMode = OSDConfigBuffer->PS2.spdifMode;
    config->screenType = OSDConfigBuffer->PS2.screenType;
    config->videoOutput = OSDConfigBuffer->PS2.videoOutput;

    if (OSDConfigBuffer->PS2.extendedLanguage) // Extended/Basic language set
    {                                          // One of the 8 standard languages
        config->language = OSDConfigBuffer->PS2.language;
    } else { // Japanese/English
        config->language = OSDConfigBuffer->PS2.japLanguage;
    }

    config->daylightSaving = OSDConfigBuffer->PS2.daylightSaving;
    config->timeFormat = OSDConfigBuffer->PS2.timeFormat;
    config->dateFormat = OSDConfigBuffer->PS2.dateFormat;
    config->timezoneOffset = OSDConfigBuffer->PS2.timezoneOffsetLo | ((u32)OSDConfigBuffer->PS2.timezoneOffsetHi) << 8;
    config->timezone = OSDConfigBuffer->PS2.timezoneLo | (((u32)OSDConfigBuffer->PS2.timezoneHi) << 8);
    config->rcEnabled = OSDConfigBuffer->PS2.rcEnabled;
    config->rcGameFunction = OSDConfigBuffer->PS2.rcGameFunction;
    config->rcSupported = OSDConfigBuffer->PS2.rcSupported;
    config->dvdpProgressive = OSDConfigBuffer->PS2.dvdpProgressive;

    return (OSDConfigBuffer->PS2.osdInit ^ 1);
}

static void ReadOSDConfigPS1(OSDConfig1_t *config, const OSDConfigStore_t *OSDConfigBuffer)
{
    int i;

    for (i = 0; i < CONFIG_BLOCK_SIZE; i++)
        config->data[i] = OSDConfigBuffer->PS1.bytes[i];
}

static void WriteOSDConfigPS1(OSDConfigStore_t *OSDConfigBuffer, const OSDConfig1_t *config)
{
    int i;

    for (i = 0; i < CONFIG_BLOCK_SIZE; i++)
        OSDConfigBuffer->PS1.bytes[i] = config->data[i];
}

static void WriteOSDConfigPS2(OSDConfigStore_t *OSDConfigBuffer, const OSDConfig2_t *config, u8 invalid)
{
    int japLanguage, version, osdInitValue;

    osdInitValue = invalid ^ 1;
    version = OSDConfigBuffer->PS2.extendedLanguage == 0 ? 1 : OSDConfigBuffer->PS2.extendedLanguage;

    if (config->language <= LANGUAGE_ENGLISH)
        japLanguage = config->language;
    else // Do not update the legacy language option if the language was changed to something unsupported.
        japLanguage = OSDConfigBuffer->PS2.japLanguage;

    // 0x0F
    OSDConfigBuffer->PS2.videoOutput = config->videoOutput;
    OSDConfigBuffer->PS2.japLanguage = japLanguage;
    OSDConfigBuffer->PS2.extendedLanguage = 1;
    OSDConfigBuffer->PS2.spdifMode = config->spdifMode;
    OSDConfigBuffer->PS2.screenType = config->screenType;

    // 0x10
    OSDConfigBuffer->PS2.language = config->language;
    OSDConfigBuffer->PS2.version = version;

    // 0x11
    OSDConfigBuffer->PS2.timezoneOffsetHi = config->timezoneOffset >> 8;
    OSDConfigBuffer->PS2.dateFormat = config->dateFormat;
    OSDConfigBuffer->PS2.timeFormat = config->timeFormat;
    OSDConfigBuffer->PS2.daylightSaving = config->daylightSaving;
    OSDConfigBuffer->PS2.osdInit = osdInitValue;

    // 0x12
    OSDConfigBuffer->PS2.timezoneOffsetLo = config->timezoneOffset;

    // 0x13
    OSDConfigBuffer->PS2.timezoneHi = config->timezone >> 8;
    /// ISRA: the following line is here because this is done on sony code. commented here cause it's senseless
    //OSDConfigBuffer->PS2.unknownB13_01 = OSDConfigBuffer->PS2.unknownB13_01; // Carry over
    OSDConfigBuffer->PS2.rcEnabled = config->rcEnabled;
    OSDConfigBuffer->PS2.rcGameFunction = config->rcGameFunction;
    OSDConfigBuffer->PS2.rcSupported = config->rcSupported;
    OSDConfigBuffer->PS2.dvdpProgressive = config->dvdpProgressive;

    // 0x14
    OSDConfigBuffer->PS2.timezoneLo = config->timezone;
}

static void ReadConfigFromNVM(u8 *buffer)
{ /*  Hmm. What should the check for stat be? In v1.xx, it seems to be a check against 0x9. In v2.20, it checks against 0x81.
      In the HDD Browser, reading checks against 0x81, while writing checks against 0x9.
      But because we are targeting all consoles, it would be probably safer to follow the HDD Browser. */
    int result;
    u32 stat;

    do {
        sceCdOpenConfig(1, 0, 2, &stat);
    } while (stat & 0x81);

    do {
        result = sceCdReadConfig(buffer, &stat);
    } while ((stat & 0x81) || (result == 0));

    do {
        result = sceCdCloseConfig(&stat);
    } while ((stat & 0x81) || (result == 0));
}

static void WriteConfigToNVM(const u8 *buffer)
{ // Read the comment in ReadConfigFromNVM() about the error status bits.
    u32 stat;
    int result;

    do {
        sceCdOpenConfig(1, 1, 2, &stat);
    } while (stat & 0x09);

    do {
        result = sceCdWriteConfig(buffer, &stat);
    } while ((stat & 0x09) || (result == 0));

    do {
        result = sceCdCloseConfig(&stat);
    } while ((stat & 9) || (result == 0));
}

int OSDLoadConfigFromNVM(OSDConfig1_t *osdConfigPS1, OSDConfig2_t *osdConfigPS2)
{
    int result;

    ReadConfigFromNVM(OSDConfigBuffer);
    result = ReadOSDConfigPS2(osdConfigPS2, (const OSDConfigStore_t *)OSDConfigBuffer);
    ReadOSDConfigPS1(osdConfigPS1, (const OSDConfigStore_t *)OSDConfigBuffer);

    return result;
}

int OSDSaveConfigToNVM(const OSDConfig1_t *osdConfigPS1, const OSDConfig2_t *osdConfigPS2, u8 invalid)
{
    WriteOSDConfigPS1((OSDConfigStore_t *)OSDConfigBuffer, osdConfigPS1);
    WriteOSDConfigPS2((OSDConfigStore_t *)OSDConfigBuffer, osdConfigPS2, invalid);
    WriteConfigToNVM(OSDConfigBuffer);

    return 0;
}

// Directory management
const char *OSDGetHistoryDataFolder(void)
{
    return SystemDataFolder;
}

const char *OSDGetSystemDataFolder(void)
{
    return SystemDataFolder;
}

const char *OSDGetSystemExecFolder(void)
{
    return SystemExecFolder;
}

const char *OSDGetDVDPLExecFolder(void)
{
    return DVDPLExecFolder;
}

int OSDInitROMVER(void)
{
    int fd;

    memset(ConsoleROMVER, 0, ROMVER_MAX_LEN);
    if ((fd = open("rom0:ROMVER", O_RDONLY)) >= 0) {
        read(fd, ConsoleROMVER, ROMVER_MAX_LEN);
        close(fd);
        return 1;
    }

    return 0;
}
