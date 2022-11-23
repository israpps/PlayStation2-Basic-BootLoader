#define ROMVER_MAX_LEN 16

int OSDInitROMVER(void);

#define CONFIG_BLOCK_SIZE 15

enum CONSOLE_REGION
{
    CONSOLE_REGION_INVALID = -1,
    CONSOLE_REGION_JAPAN = 0,
    CONSOLE_REGION_USA, // USA and Asia
    CONSOLE_REGION_EUROPE,
    CONSOLE_REGION_CHINA,

    CONSOLE_REGION_COUNT
};

enum OSD_REGION
{
    OSD_REGION_INVALID = -1,
    OSD_REGION_JAPAN = 0,
    OSD_REGION_USA,
    OSD_REGION_EUROPE,
    OSD_REGION_CHINA,
    OSD_REGION_RUSSIA,
    OSD_REGION_KOREA,
    OSD_REGION_ASIA,

    OSD_REGION_COUNT
};

// Used to store the values, as obtained from the EEPROM/NVM
typedef struct
{
    /** 0=enabled, 1=disabled */
    /*00*/ u32 spdifMode       : 1;
    /** 0=4:3, 1=fullscreen, 2=16:9 */
    /*01*/ u32 screenType      : 2;
    /** 0=rgb(scart), 1=component */
    /*03*/ u32 videoOutput     : 1;
    /** LANGUAGE_??? value */
    /*04*/ u32 language        : 5;
    /** Timezone minutes offset from GMT */
    /*09*/ u32 timezoneOffset  : 11;
    /** Timezone ID */
    /*20*/ u32 timezone        : 9;
    /** 0=standard(winter), 1=daylight savings(summer) */
    /*29*/ u32 daylightSaving  : 1;
    /** 0=24 hour, 1=12 hour */
    /*30*/ u32 timeFormat      : 1;

    /** 0=YYYYMMDD, 1=MMDDYYYY, 2=DDMMYYYY */
    /*00*/ u32 dateFormat      : 2;
    /** Remote Control On/Off option */
    /*02*/ u32 rcEnabled       : 1;
    /** Remote Control Game Function On/Off */
    /*03*/ u32 rcGameFunction  : 1;
    /** Whether the Remote Control is supported by the PlayStation 2. */
    /*04*/ u32 rcSupported     : 1;
    /** Whether the DVD player should have progressive scanning enabled. */
    /*05*/ u32 dvdpProgressive : 1;
} OSDConfig2_t;

typedef struct
{
    u8 data[CONFIG_BLOCK_SIZE];
    u8 padding;
} OSDConfig1_t;

// Structure of OSD Configuration block within EEPROM
typedef struct
{
    union
    {
        // 0x00-0x0E
        struct
        {
            u8 ps1drv;
            u8 unused[14];
        };
        u8 bytes[CONFIG_BLOCK_SIZE];
    } PS1;

    union
    {
        struct
        {
            // 0x0F
            u8 spdifMode        : 1;
            u8 screenType       : 2;
            u8 videoOutput      : 1;
            u8 japLanguage      : 1;
            u8 extendedLanguage : 1;
            u8 unused1          : 2; // Neither set nor read anywhere.

            // 0x10
            u8 language         : 5;
            u8 version          : 3;

            // 0x11
            u8 timezoneOffsetHi : 3;
            u8 daylightSaving   : 1;
            u8 timeFormat       : 1;
            u8 dateFormat       : 2;
            u8 osdInit          : 1;

            // 0x12
            u8 timezoneOffsetLo;

            // 0x13
            u8 timezoneHi      : 1;
            u8 unknownB13_01   : 3; // Value is carried over
            u8 dvdpProgressive : 1;
            u8 rcSupported     : 1;
            u8 rcGameFunction  : 1;
            u8 rcEnabled       : 1;

            // 0x14
            u8 timezoneLo;

            // 0x15-0x1E
            u8 unusedBytes[9];
        };

        u8 bytes[CONFIG_BLOCK_SIZE];
    } PS2;
} OSDConfigStore_t;

int OSDIsLanguageValid(int region, int language); // Returns >= 0 if language is valid for use in the specified region.
int OSDGetConsoleRegion(void);                    // Initializes and returns the console's region (CONSOLE_REGION).
void OSDInitSystemPaths(void);                    // Initializes system directory names
int OSDGetDefaultLanguage(void);                  // Returns the default language for the console
int OSDGetRegion(void);                           // Initializes and returns the OSD region (OSD_REGION).
int OSDGetVideoMode(void);                        // 0 = NTSC, 1 = PAL

// MagicGate-related functions that are applicable to ROM v2.20 and later
int OSDGetPS1DRVRegion(char *region);    // Returns the MagicGate region letter for the PlayStation Driver (returns 0 on error)
int OSDGetDVDPlayerRegion(char *region); // Returns the MagicGate region letter for the DVD Player (returns 0 on error)
int OSDGetMGRegion(void);                // Returns MagicGate region letter (returns '\0' on error)

// Low-level OSD configuration-management functions (Please use the functions in OSDConfig instead)
int OSDLoadConfigFromNVM(OSDConfig1_t *osdConfigPS1, OSDConfig2_t *osdConfigPS2);                       // Load OSD configuration from NVRAM. Returns 0 on success.
int OSDSaveConfigToNVM(const OSDConfig1_t *osdConfigPS1, const OSDConfig2_t *osdConfigPS2, u8 osdInit); // Save OSD configuration to NVRAM. Returns 0 on success.

// For retrieving various folder names
const char *OSDGetSystemExecFolder(void);
const char *OSDGetSystemDataFolder(void);
const char *OSDGetDVDPLExecFolder(void);
const char *OSDGetHistoryDataFolder(void);

// For compatibility with the homebrew SDK
#define sceMcInit        mcInit
#define sceMcGetInfo     mcGetInfo
#define sceMcOpen        mcOpen
#define sceMcClose       mcClose
#define sceMcSeek        mcSeek
#define sceMcRead        mcRead
#define sceMcWrite       mcWrite
#define sceMcGetDir      mcGetDir
#define sceMcMkDir       mcMkDir
#define sceMcSetFileInfo mcSetFileInfo
#define sceMcDelete      mcDelete
#define sceMcSync        mcSync
