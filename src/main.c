#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <debug.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sbv_patches.h>
#include <ps2sdkapi.h>
#include <usbhdfsd-common.h>

#include <osd_config.h>

#include <libpad.h>
#include <libmc.h>
#include <libcdvd.h>

#include "debugprintf.h"
#include "pad.h"
#include "util.h"
#include "common.h"

#include "libcdvd_add.h"
#include "dvdplayer.h"
#include "OSDInit.h"
#include "OSDConfig.h"
#include "OSDHistory.h"
#include "ps1.h"
#include "ps2.h"
#include "modelname.h"
#include "banner.h"

#ifdef PSX
#include <iopcontrol_special.h>
#include "psx/plibcdvd_add.h"
#endif

// For avoiding define NEWLIB_AWARE
void fioInit();

#define RBG2INT(R,G,B) ((0<<24) + (R<<16) + (G<<8) + B)
#define IMPORT_BIN2C(_n)       \
    extern unsigned char _n[]; \
    extern unsigned int size_##_n

// --------------- IRX/IOPRP extern --------------- //
#ifdef PSX
IMPORT_BIN2C(psx_ioprp);
#endif
IMPORT_BIN2C(sio2man_irx);
IMPORT_BIN2C(mcman_irx);
IMPORT_BIN2C(mcserv_irx);
IMPORT_BIN2C(padman_irx);

#ifdef HAS_EMBEDDED_IRX
IMPORT_BIN2C(usbd_irx);
#ifdef NO_BDM
IMPORT_BIN2C(usb_mass_irx);
#else
IMPORT_BIN2C(bdm_irx);
IMPORT_BIN2C(bdmfs_fatfs_irx);
IMPORT_BIN2C(usbmass_bd_irx);
#endif // NO_BDM
#endif // HAS_EMBEDDED_IRX
// --------------- func defs --------------- //
void RunLoaderElf(char *filename, char *party);
void EMERGENCY(void);
void ResetIOP(void);
void SetDefaultSettings(void);
void TimerInit(void);
u64 Timer(void);
void TimerEnd(void);
char *CheckPath(char *path);
static void AlarmCallback(s32 alarm_id, u16 time, void *common);
int dischandler();
void CDVDBootCertify(u8 romver[16]);
void credits(void);
void CleanUp(void);
void LoadUSBIRX(void);
#ifdef PSX
static void InitPSX();
#endif


// --------------- glob stuff --------------- //
typedef struct
{
    int SKIPLOGO;
    char *KEYPATHS[17][3];
    int DELAY;
    int OSDHISTORY_READ;
    int TRAYEJECT;
} CONFIG;
CONFIG GLOBCFG;

char *EXECPATHS[3];
u8 ROMVER[16];
int PAD = 0;


int main(int argc, char *argv[])
{
    u32 STAT;
    u64 tstart;
    int button, x, j, cnf_size, is_PCMCIA = 0, fd, result;
    static int config_source = SOURCE_INVALID, num_buttons = 4, pad_button = 0x0100; // first pad button is L2
    unsigned char *RAM_p = NULL;
    char *CNFBUFF, *name, *value;

    ResetIOP();
    SifInitIopHeap(); // Initialize SIF services for loading modules and files.
    SifLoadFileInit();
    fioInit(); // NO scr_printf BEFORE here
    init_scr();
    scr_setCursor(0);//get rid of that cursor.
    scr_printf(".\n"); // GBS control does not detect image output with scr debug till the first char is printed
    // print a simple dot here to represent the program start, and to allow gbs control to start displaying video before banner and pad timeout begins to run.
    DPRINTF("enabling LoadModuleBuffer\n");
    sbv_patch_enable_lmb(); // The old IOP kernel has no support for LoadModuleBuffer. Apply the patch to enable it.

    DPRINTF("disabling MODLOAD device blacklist/whitelist\n");
    sbv_patch_disable_prefix_check(); /* disable the MODLOAD module black/white list, allowing executables to be freely loaded from any device. */
#ifdef USE_ROM_SIO2MAN
    j = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    DPRINTF(" [SIO2MAN.IRX]: %d\n", j);
#else
    j = SifExecModuleBuffer(sio2man_irx, size_sio2man_irx, 0, NULL, &x);
    DPRINTF(" [SIO2MAN.IRX]: ret=%d, ID=%d\n", j, x);
#endif
#ifdef USE_ROM_MCMAN
    j = SifLoadModule("rom0:MCMAN", 0, NULL);
    DPRINTF(" [MCMAN.IRX]: %d\n", j);
    j = SifLoadModule("rom0:MCSERV", 0, NULL);
    DPRINTF(" [MCSERV.IRX]: %d\n", j);
    mcInit(MC_TYPE_MC);
#else
    j = SifExecModuleBuffer(mcman_irx, size_mcman_irx, 0, NULL, &x);
    DPRINTF(" [MCMAN.IRX]: ret=%d, ID=%d\n", j, x);
    j = SifExecModuleBuffer(mcserv_irx, size_mcserv_irx, 0, NULL, &x);
    DPRINTF(" [MCSERV.IRX]: ret=%d, ID=%d\n", j, x);
    mcInit(MC_TYPE_XMC);
#endif
#ifdef USE_ROM_PADMAN
    j = SifLoadModule("rom0:PADMAN", 0, NULL);
    DPRINTF(" [PADMAN.IRX]: %d\n", j);
#else
    j = SifExecModuleBuffer(padman_irx, size_padman_irx, 0, NULL, &x);
    DPRINTF(" [PADMAN.IRX]: ret=%d, ID=%d\n", j, x);
#endif
    LoadUSBIRX();

    if ((fd = open("rom0:ROMVER", O_RDONLY)) >= 0) {
        read(fd, ROMVER, sizeof(ROMVER));
        close(fd);
    }
    j = SifLoadModule("rom0:ADDDRV", 0, NULL); // Load ADDDRV. The OSD has it listed in rom0:OSDCNF/IOPBTCONF, but it is otherwise not loaded automatically.
    DPRINTF(" [ADDDRV.IRX]: %d\n", j);

    DPRINTF("init OSD system paths\n");
    OSDInitSystemPaths();

    // Initialize libcdvd & supplement functions (which are not part of the ancient libcdvd library we use).
    sceCdInit(SCECdINoD);
    cdInitAdd();

#ifndef PSX
    DPRINTF("Certifying CDVD Boot\n");
    CDVDBootCertify(ROMVER); /* This is not required for the PSX, as its OSDSYS will do it before booting the update. */
#endif

    DPRINTF("init OSD\n");
    InitOsd(); // Initialize OSD so kernel patches can do their magic

    DPRINTF("init ROMVER, model name ps1dvr and dvdplayer ver\n");
    OSDInitROMVER(); // Initialize ROM version (must be done first).
    ModelNameInit(); // Initialize model name
    PS1DRVInit();    // Initialize PlayStation Driver (PS1DRV)
    DVDPlayerInit(); // Initialize ROM DVD player. It is normal for this to fail on consoles that have no DVD ROM chip (i.e. DEX or the SCPH-10000/SCPH-15000).

    if (OSDConfigLoad() != 0) // Load OSD configuration
    {                         // OSD configuration not initialized. Defaults loaded.
        scr_setfontcolor(0x00ffff);
        DPRINTF("OSD Configuration not initialized. Defaults loaded.\n");
        scr_setfontcolor(0xffffff);
    }

    // Applies OSD configuration (saves settings into the EE kernel)
    DPRINTF("Saving OSD configuration to EE Kernel\n");
    OSDConfigApply();

    /*  Try to enable the remote control, if it is enabled.
        Indicate no hardware support for it, if it cannot be enabled. */
    DPRINTF("trying to enable remote control");
    do {
        DPRINTF(".");
        result = sceCdRcBypassCtl(OSDConfigGetRcGameFunction() ^ 1, &STAT);
        if (STAT & 0x100) { // Not supported by the PlayStation 2.
            // Note: it does not seem like the browser updates the NVRAM here to change this status.
            OSDConfigSetRcEnabled(0);
            OSDConfigSetRcSupported(0);
            DPRINTF("\n");
            break;
        }
    } while ((STAT & 0x80) || (result == 0));

    // Remember to set the video output option (RGB or Y Cb/Pb Cr/Pr) accordingly, before SetGsCrt() is called.
    DPRINTF("Setting vmode\n");
    SetGsVParam(OSDConfigGetVideoOutput() == VIDEO_OUTPUT_RGB ? VIDEO_OUTPUT_RGB : VIDEO_OUTPUT_COMPONENT); 
    DPRINTF("Init pads\n");
    PadInitPads();
    DPRINTF("Init timer and wait for rescue mode key\n");
    TimerInit();
    tstart = Timer();
    while (Timer() <= (tstart + 2000)) {
        PAD = ReadCombinedPadStatus();
        if ((PAD & PAD_R1) && (PAD & PAD_START)) // if ONLY R1+START are pressed...
            EMERGENCY();
    }
    TimerEnd();
    DPRINTF("load default settings\n");
    SetDefaultSettings();
    FILE *fp;
    DPRINTF("Reading settings...\n");
    fp = fopen("mass:/PS2BBL/CONFIG.INI", "r");
    if (fp == NULL) {
        DPRINTF("Cant load config from mass\n");
        fp = fopen("mc0:/PS2BBL/CONFIG.INI", "r");
        if (fp == NULL) {
            DPRINTF("Cant load config from mc0\n");
            fp = fopen("mc1:/PS2BBL/CONFIG.INI", "r");
            if (fp == NULL) {
                DPRINTF("Cant load config from mc1\n");
                config_source = SOURCE_INVALID;
            } else {
                config_source = SOURCE_MC1;
            }
        } else {
            config_source = SOURCE_MC0;
        }
    } else {
        config_source = SOURCE_MASS;
    }

    if (config_source != SOURCE_INVALID) {
        DPRINTF("valid config on device %d, reading now\n", config_source);
        pad_button = 0x0001;
        num_buttons = 16;
        fseek(fp, 0, SEEK_END);
        cnf_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        DPRINTF("Allocating %d bytes for RAM_p\n", cnf_size);
        RAM_p = (unsigned char *)malloc(cnf_size + 1);
        if (RAM_p != NULL) {
            CNFBUFF = RAM_p;
            int temp;
            if ((temp = fread(RAM_p, 1, cnf_size, fp)) == cnf_size) {
                DPRINTF("Reading finished... Closing fp*\n");
                fclose(fp);
                CNFBUFF[cnf_size] = '\0';
                int var_cnt = 0;
                char TMP[64];
                for (var_cnt = 0; get_CNF_string(&CNFBUFF, &name, &value); var_cnt++) {
                    // DPRINTF("reading entry %d", var_cnt);
                    if (!strcmp("OSDHISTORY_READ", name)) {
                        GLOBCFG.OSDHISTORY_READ = atoi(value);
                        continue;
                    }
                    if (!strcmp("SKIP_PS2LOGO", name)) {
                        GLOBCFG.SKIPLOGO = atoi(value);
                        continue;
                    }
                    if (!strcmp("KEY_READ_WAIT_TIME", name)) {
                        GLOBCFG.DELAY = atoi(value);
                        continue;
                    }
                    if (!strcmp("EJECT_TRAY", name)) {
                        GLOBCFG.TRAYEJECT = atoi(value);
                        continue;
                    }
                    for (x = 0; x < 17; x++) {
                        for (j = 0; j < 3; j++) {
                            sprintf(TMP, "LK_%s_E%d", KEYS_ID[x], j + 1);
                            if (!strcmp(name, TMP)) {
                                GLOBCFG.KEYPATHS[x][j] = value;
                                break;
                            }
                        }
                    }
                }
                free(RAM_p);
            } else {
                fclose(fp);
                scr_setfontcolor(0x0000ff);
                scr_printf("\tERROR: could not read %d bytes of config file, only %d readed\n", cnf_size, temp);
                scr_setfontcolor(0xffffff);
            }
        } else {
            scr_setbgcolor(0x0000ff);
            scr_clear();
            scr_printf("\tFailed to allocate %d+1 bytes!\n", cnf_size);
            sleep(3);
            scr_setbgcolor(0x000000);
            scr_clear();

        }
    } else {
        scr_printf("Can't find config, loading hardcoded paths\n");
        for (x = 0; x < 5; x++)
            for (j = 0; j < 3; j++)
                GLOBCFG.KEYPATHS[x][j] = CheckPath(DEFPATH[3 * x + j]);
        sleep(1);
    }
    if (RAM_p != NULL)
        free(RAM_p);
    int R = 0x80, G = 0x80, B = 0x80;
    if (GLOBCFG.OSDHISTORY_READ)
    {
        j = 1;
        // Try to load the history file from memory card slot 1
        if (LoadHistoryFile(0) < 0) { // Try memory card slot 2
            if (LoadHistoryFile(1) < 0)
                {
                    DPRINTF("no history files found\n\n");
                    j = 0;
                }
        }
        
        if (j)
        {
            for (j=0; j < MAX_HISTORY_ENTRIES; j++)
            {
				switch (j%3) {
					case 0:
						R += (HistoryEntries[j].LaunchCount*2);
						break;
					case 1:
						G += (HistoryEntries[j].LaunchCount*2);
						break;
					case 2:
						B += (HistoryEntries[j].LaunchCount*2);
						break;
					default:
						B += (HistoryEntries[j].LaunchCount*2);
						
				}
            }
            scr_setfontcolor(RBG2INT(B,G,R));
            DPRINTF("New banner color is: #%8x\n",RBG2INT(B,G,R));
        } else
            DPRINTF("can't find any osd history for banner color\n");
    }
    // Stores last key during DELAY msec
    scr_clear();
    scr_printf("\n\n\n\n%s", BANNER);
    scr_setfontcolor(0xffffff);
    scr_printf(BANNER_FOOTER"\n\n\tModel:\t\t%s\n"
               "\tPlayStation Driver:\t%s\n"
               "\tDVD Player:\t%s\n"
			   "\tConfig source:\t%d\n",
               ModelNameGet(),
               PS1DRVGetVersion(),
               DVDPlayerGetVersion(),
			   config_source);
    DPRINTF("Setting vmode\n");
    TimerInit();
    tstart = Timer();
    while (Timer() <= (tstart + GLOBCFG.DELAY)) {
        // while (1) {
        //  If key was detected
        //  DPRINTF("Trying to read PADs\n");
        PAD = ReadCombinedPadStatus();
        button = pad_button;
        for (x = 0; x < num_buttons; x++) { // check all pad buttons
            if (PAD & button) {
                DPRINTF("PAD detected\n");
                // if button detected , copy path to corresponding index
                for (j = 0; j < 3; j++)
                    EXECPATHS[j] = GLOBCFG.KEYPATHS[x + 1][j];
                for (j = 0; j < 3; j++) {
                    EXECPATHS[j] = CheckPath(EXECPATHS[j]);
                    if (exist(EXECPATHS[j])) {
                        scr_setfontcolor(0x00ff00);
                        scr_printf("\tLoading %s\n", EXECPATHS[j]);
                        if (!is_PCMCIA)
                            PadDeinitPads();
                        RunLoaderElf(EXECPATHS[j], NULL);
                    } else {
                        scr_setfontcolor(0x00ffff);
                        DPRINTF("%s not found\n", EXECPATHS[j]);
                        scr_setfontcolor(0xffffff);
                    }
                }
                break;
            }
            button = button << 1; // sll of 1 cleared bit to move to next pad button
        }
    }

    tstart = Timer();
    if (Timer() <= (tstart + 4000)) {
        scr_clear();
        for (j = 0; j < 3; j++) {
            if (exist(CheckPath(GLOBCFG.KEYPATHS[0][j]))) {
                if (!is_PCMCIA)
                    PadDeinitPads();
                RunLoaderElf(CheckPath(GLOBCFG.KEYPATHS[0][j]), NULL);
            }
        }
    }
    TimerEnd();

    scr_printf("\n\n\tEND OF EXECUTION REACHED\nCould not find any of the default applications\nCheck your config file for the LK_AUTO_E# entries\nOr press a key while logo displays to run the bound application");
    while (1) {
        ;
    }

    return 0;
}

void EMERGENCY(void)
{
    scr_clear();
    scr_printf("\n\n\n\tEmergency mode\n\n\t doing infinite attempts to boot\n\t\tmass:/RESCUE.ELF\n");
    scr_setfontcolor(0xffffff);
    while (1) {
        scr_printf(".");
        sleep(1);
        if (exist("mass:/RESCUE.ELF")) {
            PadDeinitPads();
            RunLoaderElf("mass:/RESCUE.ELF", NULL);
        }
    }
}

void runKELF(const char *kelfpath)
{
    char arg3[64];
    char *args[4] = {"-m rom0:SIO2MAN", "-m rom0:MCMAN", "-m rom0:MCSERV", arg3};
    sprintf(arg3, "-x %s", kelfpath);

    PadDeinitPads();
    LoadExecPS2("moduleload", 4, args);
}

char *CheckPath(char *path)
{
    if (path[0] == '$') // we found a program command
    {
        if (!strcmp("$CDVD", path))
            dischandler();
        if (!strcmp("$CDVD_NO_PS2LOGO", path)) {
            GLOBCFG.SKIPLOGO = 1;
            dischandler();
        }
        if (!strcmp("$CREDITS", path))
            credits();
        if (!strncmp("$RUNKELF:", path, strlen("$RUNKELF:"))) {
            runKELF(CheckPath(path + strlen("$RUNKELF:"))); // pass to runKELF the path without the command token, digested again by CheckPath()
        }
    }
    if (!strncmp("mc?", path, 3)) {
        path[2] = '0';
        if (exist(path)) {
            return path;
        } else {
            path[2] = '1';
            if (exist(path))
                return path;
        }
    }
    return path;
}

void SetDefaultSettings(void)
{
    int i, j;
    for (i = 0; i < 17; i++)
        for (j = 0; j < 3; j++)
            GLOBCFG.KEYPATHS[i][j] = NULL;
    GLOBCFG.SKIPLOGO = 0;
    GLOBCFG.OSDHISTORY_READ = 1;
    GLOBCFG.DELAY = DEFDELAY;
    GLOBCFG.TRAYEJECT = 0;
}

void LoadUSBIRX(void)
{

#ifndef NO_DPRINTF
    int TMP;
    #define ATMP TMP = 
#else
    #define ATMP
#endif
    int x;

// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ATMP SifExecModuleBuffer(bdm_irx, size_bdm_irx, 0, NULL, &x);
#else
    ATMP loadIRXFile("mc?:/PS2BBL/BDM.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [BDM.IRX]: ret=%d, ID=%d\n", TMP, x);
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ATMP SifExecModuleBuffer(bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, &x);
#else
    ATMP loadIRXFile("mc?:/PS2BBL/BDMFS_FATFS.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [BDMFS_FATFS.IRX]: ret=%d, ID=%d\n", TMP, x);
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ATMP SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, &x);
#else
    ATMP loadIRXFile("mc?:/PS2BBL/USBD.IRX", 0, NULL, &x);
#endif
    delay(3);
    DPRINTF(" [USBD.IRX]: ret=%d, ID=%d\n", TMP, x);
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ATMP SifExecModuleBuffer(usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, &x);
#else
    ATMP loadIRXFile("mc?:/PS2BBL/USBMASS_BD.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [USBMASS_BD.IRX]: ret=%d, ID=%d\n", TMP, x);
// ------------------------------------------------------------------------------------ //
    struct stat buffer;
    int ret = -1;
    int retries = 50;

    while(ret != 0 && retries > 0)
    {
        ret = stat("mass:/", &buffer);
        /* Wait until the device is ready */
        nopdelay();

        retries--;
    }
}


int dischandler()
{
    int OldDiscType, DiscType, ValidDiscInserted, result, first_run = 1;
    u32 STAT;

    scr_clear();
    scr_printf("\t%s: Activated\n", __func__);

    scr_printf("\t\tEnabling Diagnosis...\n");
    do { // 0 = enable, 1 = disable.
        result = sceCdAutoAdjustCtrl(0, &STAT);
    } while ((STAT & 0x08) || (result == 0));

    // For this demo, wait for a valid disc to be inserted.
    scr_printf("\tWaiting for disc to be inserted...\n\n");

    ValidDiscInserted = 0;
    OldDiscType = -1;
    while (!ValidDiscInserted) {
        DiscType = sceCdGetDiskType();
        if (DiscType != OldDiscType) {
            scr_printf("\tNew Disc:\t");
            OldDiscType = DiscType;

            switch (DiscType) {
                case SCECdNODISC:
                    if (first_run)
                    {
                        if (GLOBCFG.TRAYEJECT) // if tray eject is allowed on empty tray...
                            sceCdTrayReq(0, NULL);
                        first_run = 0;
                    }
                    scr_setfontcolor(0x0000ff);
                    scr_printf("No Disc\n");
                    scr_setfontcolor(0xffffff);
                    break;

                case SCECdDETCT:
                case SCECdDETCTCD:
                case SCECdDETCTDVDS:
                case SCECdDETCTDVDD:
                    scr_printf("Reading...\n");
                    break;

                case SCECdPSCD:
                case SCECdPSCDDA:
                    scr_setfontcolor(0x00ff00);
                    scr_printf("PlayStation\n");
                    scr_setfontcolor(0xffffff);
                    ValidDiscInserted = 1;
                    break;

                case SCECdPS2CD:
                case SCECdPS2CDDA:
                case SCECdPS2DVD:
                    scr_setfontcolor(0x00ff00);
                    scr_printf("PlayStation 2\n");
                    scr_setfontcolor(0xffffff);
                    ValidDiscInserted = 1;
                    break;

                case SCECdCDDA:
                    scr_setfontcolor(0xffff00);
                    scr_printf("Audio Disc (not supported by this program)\n");
                    scr_setfontcolor(0xffffff);
                    break;

                case SCECdDVDV:
                    scr_setfontcolor(0x00ff00);
                    scr_printf("DVD Video\n");
                    scr_setfontcolor(0xffffff);
                    ValidDiscInserted = 1;
                    break;
                default:
                    scr_setfontcolor(0x0000ff);
                    scr_printf("Unknown\n");
                    scr_setfontcolor(0xffffff);
            }
        }

        // Avoid spamming the IOP with sceCdGetDiskType(), or there may be a deadlock.
        // The NTSC/PAL H-sync is approximately 16kHz. Hence approximately 16 ticks will pass every millisecond.
        SetAlarm(1000 * 16, &AlarmCallback, (void *)GetThreadId());
        SleepThread();
    }

    // Now that a valid disc is inserted, do something.
    // CleanUp() will be called, to deinitialize RPCs. SIFRPC will be deinitialized by the respective disc-handlers.
    switch (DiscType) {
        case SCECdPSCD:
        case SCECdPSCDDA:
            // Boot PlayStation disc
            PS1DRVBoot();
            break;

        case SCECdPS2CD:
        case SCECdPS2CDDA:
        case SCECdPS2DVD:
            // Boot PlayStation 2 disc
            PS2DiscBoot(GLOBCFG.SKIPLOGO);
            break;

        case SCECdDVDV:
            /*  If the user chose to disable the DVD Player progressive scan setting,
                it is disabled here because Sony probably wanted the setting to only bind if the user played a DVD.
                The original did the updating of the EEPROM in the background, but I want to keep this demo simple.
                The browser only allowed this setting to be disabled, by only showing the menu option for it if it was enabled by the DVD Player. */
            /* OSDConfigSetDVDPProgressive(0);
            OSDConfigApply(); */

            /*  Boot DVD Player. If one is stored on the memory card and is newer, it is booted instead of the one from ROM.
                Play history is automatically updated. */
            DVDPlayerBoot();
            break;
    }
    return 0;
}

void ResetIOP(void)
{
    SifInitRpc(0); // Initialize SIFCMD & SIFRPC
#ifndef PSX
    while (!SifIopReset("", 0)) {
    };
#else
    /* sp193: We need some of the PSX's CDVDMAN facilities, but we do not want to use its (too-)new FILEIO module.
       This special IOPRP image contains a IOPBTCONF list that lists PCDVDMAN instead of CDVDMAN.
       PCDVDMAN is the board-specific CDVDMAN module on all PSX, which can be used to switch the CD/DVD drive operating mode.
       Usually, I would discourage people from using board-specific modules, but I do not have a proper replacement for this. */
    while (!SifIopRebootBuffer(psx_ioprp, size_psx_ioprp)) {};
#endif
    while (!SifIopSync()) {
    };

#ifdef PSX
    InitPSX();
#endif
}

#ifdef PSX
static void InitPSX()
{
    int result, STAT;

    SifInitRpc(0);
    sceCdInit(SCECdINoD);

    // No need to perform boot certification because rom0:OSDSYS does it.
    while (custom_sceCdChgSys(2) != 2) {}; // Switch the drive into PS2 mode.

    do {
        result = custom_sceCdNoticeGameStart(1, &STAT);
    } while ((result == 0) || (STAT & 0x80));

    // Reset the IOP again to get the standard PS2 default modules.
    while (!SifIopReset("", 0)) {};

    /*    Set the EE kernel into 32MB mode. Let's do this, while the IOP is being reboot.
        The memory will be limited with the TLB. The remap can be triggered by calling the _InitTLB syscall
        or with ExecPS2().
        WARNING! If the stack pointer resides above the 32MB offset at the point of remap, a TLB exception will occur.
        This example has the stack pointer configured to be within the 32MB limit. */

    SetMemoryMode(1);
    _InitTLB();

    while (!SifIopSync()) {};
}
#endif

#ifndef PSX
void CDVDBootCertify(u8 romver[16])
{
    u8 RomName[4];
    /*  Perform boot certification to enable the CD/DVD drive.
        This is not required for the PSX, as its OSDSYS will do it before booting the update. */
    if (romver != NULL) {
        // e.g. 0160HC = 1,60,'H','C'
        RomName[0] = (romver[0] - '0') * 10 + (romver[1] - '0');
        RomName[1] = (romver[2] - '0') * 10 + (romver[3] - '0');
        RomName[2] = romver[4];
        RomName[3] = romver[5];

        // Do not check for success/failure. Early consoles do not support (and do not require) boot-certification.
        sceCdBootCertify(RomName);
    } else {
        scr_setfontcolor(0x0000ff);
        scr_printf("\tERROR: Could not certify CDVD Boot. ROMVER was NULL\n");
        scr_setfontcolor(0xffffff);
    }

    // This disables DVD Video Disc playback. This functionality is restored by loading a DVD Player KELF.
    /*    Hmm. What should the check for STAT be? In v1.xx, it seems to be a check against 0x08. In v2.20, it checks against 0x80.
          The HDD Browser does not call this function, but I guess it would check against 0x08. */
    /*  do
     {
         sceCdForbidDVDP(&STAT);
     } while (STAT & 0x08); */
}
#endif

static void AlarmCallback(s32 alarm_id, u16 time, void *common)
{
    iWakeupThread((int)common);
}

void CleanUp(void)
{
    sceCdInit(SCECdEXIT);
    PadDeinitPads();
}

void credits(void)
{
    scr_clear();
    scr_printf("\n\n");
    scr_printf("%s%s",BANNER, BANNER_FOOTER);
    scr_printf("\n"
               "\n"
               "\tThis project is heavily based on SP193 OSD initialization libraries.\n"
               "\t\tall credits go to him\n"
               "\tThanks to: fjtrujy, uyjulian, asmblur and AKuHAK\n"
               "\tthis build corresponds to the hash [" COMMIT_HASH "]\n"
               "\t\tcompiled on "__DATE__" "__TIME__"\n"
               );
    while (1) {};
}

/* BELOW THIS POINT ALL MACROS and MISC STUFF MADE TO REDUCE BINARY SIZE WILL BE PLACED */

#if defined(DUMMY_TIMEZONE)
   void _libcglue_timezone_update() {}
#endif

#if defined(DUMMY_LIBC_INIT)
   void _libcglue_init() {}
   void _libcglue_deinit() {}
   void _libcglue_args_parse() {}
#endif

#if defined(KERNEL_NOPATCH)
    DISABLE_PATCHED_FUNCTIONS();
#endif

PS2_DISABLE_AUTOSTART_PTHREAD();
