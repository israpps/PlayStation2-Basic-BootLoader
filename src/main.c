#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include <unistd.h>
#include <malloc.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <sbv_patches.h>
#include <ps2sdkapi.h>
#include <usbhdfsd-common.h>

#include <osd_config.h>

#include <libpad.h>
#include <libmc.h>
#include <libcdvd.h>
#include <libcdvd-common.h>

#include "debugprintf.h"
#include "pad.h"
#include "util.h"
#include "common.h"
PS2_DISABLE_AUTOSTART_PTHREAD();

#include "libcdvd_add.h"
#include "dvdplayer.h"
#include "OSDInit.h"
#include "OSDConfig.h"
#include "OSDHistory.h"
#include "ps1.h"
#include "ps2.h"
#include "modelname.h"

#define IMPORT_BIN2C(_n)       \
    extern unsigned char _n[]; \
    extern unsigned int size_##_n

#ifdef PSX
IMPORT_BIN2C(psx_ioprp);
#include <iopcontrol_special.h>
#include "psx/plibcdvd_add.h"
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
#endif //NO_BDM
#endif //HAS_EMBEDDED_IRX

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
int loadIRXFile(char *path, u32 arg_len, const char *args, int *mod_res);
void LoadUSBIRX();
void CDVDBootCertify(u8 romver[16]);
void credits(void);
static void InitPSX();

typedef struct
{
    int SKIPLOGO;
    char *KEYPATHS[17][3];
    int DELAY;
} CONFIG;
CONFIG GLOBCFG;

char *EXECPATHS[3];
u8 ROMVER[16];
int PAD = 0;

int main()
{
    u32 STAT;
    int button, x, j, cnf_size, is_PCMCIA = 0, fd, result;
    static int config_source = SOURCE_INVALID;
    unsigned char *RAM_p = NULL;
    char *CNFBUFF, *name, *value;
    static int num_buttons = 4, pad_button = 0x0100; // first pad button is L2
    GLOBCFG.DELAY = DEFDELAY;

    SifInitRpc(0); // Initialize SIFCMD & SIFRPC
    ResetIOP();
    SifInitIopHeap(); // Initialize SIF services for loading modules and files.
    SifLoadFileInit();
    fioInit(); // NO scr_printf BEFORE here
    init_scr();
    DPRINTF("enabling LoadModuleBuffer\n");
    sbv_patch_enable_lmb(); // The old IOP kernel has no support for LoadModuleBuffer. Apply the patch to enable it.

    DPRINTF("disabling MODLOAD device blacklist/whitelist\n");
    sbv_patch_disable_prefix_check(); /* disable the MODLOAD module black/white list, allowing executables to be freely loaded from any device. */
    DPRINTF("Loading IRX:\n");
    j = SifExecModuleBuffer(sio2man_irx, size_sio2man_irx, 0, NULL, &x); /*  Load SDK modules to avoid different behavior across different models*/
    DPRINTF(" [SIO2MAN.IRX]: %d\n", j);
    j = SifExecModuleBuffer(mcman_irx, size_mcman_irx, 0, NULL, &x);
    DPRINTF(" [MCMAN.IRX]: %d\n", j);
    j = SifExecModuleBuffer(mcserv_irx, size_mcserv_irx, 0, NULL, &x);
    DPRINTF(" [MCSERV.IRX]: %d\n", j);
    mcInit(MC_TYPE_XMC);
    j = SifExecModuleBuffer(padman_irx, size_padman_irx, 0, NULL, &x);
    DPRINTF(" [PADMAN.IRX]: %d\n", j);

    LoadUSBIRX();
    if ((fd = open("rom0:ROMVER", O_RDONLY)) >= 0) {
        read(fd, ROMVER, sizeof(ROMVER));
        close(fd);
    }
    j = SifLoadModule("rom0:ADDDRV", 0, NULL); // Load ADDDRV. The OSD has it listed in rom0:OSDCNF/IOPBTCONF, but it is otherwise not loaded automatically.
    DPRINTF(" [ADDDRV.IRX]: %d\n", j);
	
    DPRINTF("init OSD\n");
    InitOsd(); // Initialize OSD so kernel patches can do their magic
    DPRINTF("init OSD system paths\n");
    OSDInitSystemPaths();

#ifndef PSX
    DPRINTF("Certify CDVD Boot\n");
    CDVDBootCertify(ROMVER); /* This is not required for the PSX, as its OSDSYS will do it before booting the update. */
#endif

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
    OSDConfigApply();

    /*  Try to enable the remote control, if it is enabled.
        Indicate no hardware support for it, if it cannot be enabled. */
    DPRINTF("trying to enable remote control...\n");
    do {
        result = sceCdRcBypassCtl(OSDConfigGetRcGameFunction() ^ 1, &STAT);
        if (STAT & 0x100) { // Not supported by the PlayStation 2.
            // Note: it does not seem like the browser updates the NVRAM here to change this status.
            OSDConfigSetRcEnabled(0);
            OSDConfigSetRcSupported(0);
            break;
        }
    } while ((STAT & 0x80) || (result == 0));

    // Remember to set the video output option (RGB or Y Cb/Pb Cr/Pr) accordingly, before SetGsCrt() is called.
    SetGsVParam(OSDConfigGetVideoOutput() == VIDEO_OUTPUT_RGB ? 0 : 1);
    scr_printf("\nModel:\t\t%s\n"
               "PlayStation Driver:\t%s\n"
               "DVD Player:\t%s\n",
               ModelNameGet(),
               PS1DRVGetVersion(),
               DVDPlayerGetVersion());
    PadInitPads();

    for (x = 0; x < 3; x++, sleep(1)) { //wait for him 3 secs
        PAD = ReadCombinedPadStatus();
        if ((PAD & PAD_R1) && (PAD & PAD_START)) // if ONLY R1+START are pressed...
            EMERGENCY();
    }

    SetDefaultSettings();
    FILE *fp;
    DPRINTF("Reading settings...\n");
    fp = fopen("mc0:/PS2BBL/CONFIG.INI", "r");
    if (fp == NULL) {
        DPRINTF("Cant load config from mc0\n");
        fp = fopen("mc1:/PS2BBL/CONFIG.INI", "r");
        if (fp == NULL) {
            DPRINTF("Cant load config from mc1\n");
            fp = fopen("mass:/PS2BBL/CONFIG.INI", "r");
            if (fp == NULL) {
                DPRINTF("Cant load config from mass\n");
                config_source = SOURCE_INVALID;
            } else {
                config_source = SOURCE_MASS;
            }
        } else {
            config_source = SOURCE_MC1;
        }
    } else {
        config_source = SOURCE_MC0;
    }

    if (config_source != SOURCE_INVALID) {
        DPRINTF("valid config, reading now\n");
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
                    if (!strcmp("SKIP_PS2LOGO", name)) {
                        GLOBCFG.SKIPLOGO = atoi(value);
                        continue;
                    }
                    if (!strcmp("KEY_READ_WAIT_TIME", name)) {
                        GLOBCFG.DELAY = atoi(value);
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
                DPRINTF("ERROR: could not read %d bytes of config file, only %d readed\n", cnf_size, temp);
                scr_setfontcolor(0xffffff);
            }
        } else {
            scr_setbgcolor(0x0000ff);
            scr_setfontcolor(0xffffff);
            DPRINTF("Failed to allocate %d+1 bytes!\n", cnf_size);
        }
    } else {
        DPRINTF("Invalid config, loading hardcoded shit\n");
        for (x = 0; x < 5; x++)
            for (j = 0; j < 3; j++)
                GLOBCFG.KEYPATHS[x][j] = CheckPath(DEFPATH[3 * x + j]);
    }
    if (RAM_p != NULL)
        free(RAM_p);

    // Stores last key during DELAY msec
    u64 tstart;
    TimerInit();
    tstart = Timer();
    scr_clear();
    DPRINTF("Reading PADs\n");
    while (Timer() <= (tstart + GLOBCFG.DELAY)) {
    //while (1) {
        // If key was detected
        // DPRINTF("Trying to read PADs\n");
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
                        scr_printf("Loading %s\n", EXECPATHS[j]);
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
        for (j = 0; j < 3; j++) {
            if (exist(CheckPath(GLOBCFG.KEYPATHS[0][j]))) {
                if (!is_PCMCIA)
                    PadDeinitPads();
                RunLoaderElf(CheckPath(GLOBCFG.KEYPATHS[0][j]), NULL);
            }
        }
    }
    TimerEnd();

    scr_printf("END OF EXECUTION REACHED\n");
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

char *CheckPath(char *path)
{
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
    if (!strcmp("$CDVD", path))
        dischandler();
    if (!strcmp("$CDVD_NO_PS2LOGO", path)) {
        GLOBCFG.SKIPLOGO = 1;
        dischandler();
    }
    if (!strcmp("$CREDITS", path))
        credits();
    return path;
}

void SetDefaultSettings(void)
{
    int i, j;
    for (i = 0; i < 17; i++)
        for (j = 0; j < 3; j++)
            GLOBCFG.KEYPATHS[i][j] = NULL;
    GLOBCFG.SKIPLOGO = 0;
}

int dischandler()
{
    int OldDiscType, DiscType, ValidDiscInserted, result;
    u32 STAT;

    scr_clear();
    scr_printf("%s: Activated\n", __func__);
    sceCdInit(SCECdINoD);
    cdInitAdd();

    scr_printf("\tEnabling Diagnosis...\n");
    do { // 0 = enable, 1 = disable.
        result = sceCdAutoAdjustCtrl(0, &STAT);
    } while ((STAT & 0x08) || (result == 0));

    // For this demo, wait for a valid disc to be inserted.
    scr_printf("Waiting for disc to be inserted...\n");

    ValidDiscInserted = 0;
    OldDiscType = -1;
    while (!ValidDiscInserted) {
        DiscType = sceCdGetDiskType();
        if (DiscType != OldDiscType) {
            scr_printf("New Disc:\t");
            OldDiscType = DiscType;

            switch (DiscType) {
                case SCECdNODISC:
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

void LoadUSBIRX(void)
{
    int TMP, x;
#ifdef HAS_EMBEDDED_IRX
    TMP = SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, &x);
#else
    TMP = loadIRXFile("mc?:/PS2BBL/USBD.IRX", 0, NULL, &x);
#endif
    delay(3);
    DPRINTF(" [USBD.IRX]: %d\n", TMP);
#ifdef HAS_EMBEDDED_IRX
    TMP = SifExecModuleBuffer(bdm_irx, size_bdm_irx, 0, NULL, &x);
#else
    TMP = loadIRXFile("mc?:/PS2BBL/BDM.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [BDM.IRX]: %d\n", TMP);
#ifdef HAS_EMBEDDED_IRX
    TMP = SifExecModuleBuffer(bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, &x);
#else
    TMP = loadIRXFile("mc?:/PS2BBL/BDMFS_FATFS.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [BDMFS_FATFS.IRX]: %d\n", TMP);
#ifdef HAS_EMBEDDED_IRX
    TMP = SifExecModuleBuffer(usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, &x);
#else
    TMP = loadIRXFile("mc?:/PS2BBL/USBMASS_BD.IRX", 0, NULL, &x);
#endif
    DPRINTF(" [USBMASS_BD.IRX]: %d\n", TMP);
    delay(3);
}

void ResetIOP(void)
{
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
    while (sceCdChgSys(2) != 2) {}; // Switch the drive into PS2 mode.

    // Signal the start of a game, so that the user can use the "quit" game button.
    do {
        result = sceCdNoticeGameStart(1, &STAT);
    } while ((result == 0) || (STAT & 0x80));

    // Reset the IOP again to get the standard PS2 default modules.
    while (!SifIopReset("", 0)) {};

    /*    Set the EE kernel into 32MB mode. Let's do this, while the IOP is being reboot.
        The memory will be limited with the TLB. The remap can be triggered by calling the _InitTLB syscall
        or with ExecPS2().
        WARNING! If the stack pointer resides above the 32MB offset at the point of remap, a TLB exception will occur.
        This example has the stack pointer configured to be within the 32MB limit. */
#ifndef PSX_SKIP_32MB_RAM_MODE
    SetMemoryMode(1);
    _InitTLB();
#endif

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
        scr_printf("ERROR: Could not certify CDVD Boot. ROMVER was NULL\n");
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
    scr_printf("PLayStation2 Basic Bootloader\n"
                "Made by Matias Israelson (AKA: El_isra)\n"
                "This project is heavily based on SP193 OSD initialization libraries. all credits go to him\n"
                "this build corresponds to the hash ["COMMIT_HASH"]\n"
                "compiled on "__DATE__" "__TIME__"\n"
        );
    while(1){};
}

#if defined(DUMMY_TIMEZONE)
void _ps2sdk_timezone_update()
{
}
#endif

#if defined(DUMMY_LIBC_INIT)
void _ps2sdk_libc_init()
{
}
void _ps2sdk_libc_deinit() {}
void _ps2sdk_args_parse(int argc, char **argv) {}
#endif
