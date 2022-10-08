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

#include <libpad.h>
#include <libmc.h>
#include <libcdvd.h>

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
IMPORT_BIN2C(usbd_irx);
#ifdef NO_BDM
IMPORT_BIN2C(usb_mass_irx);
#else
IMPORT_BIN2C(bdm_irx);
IMPORT_BIN2C(bdmfs_fatfs_irx);
IMPORT_BIN2C(usbmass_bd_irx);
#endif

void RunLoaderElf(char *filename, char *party);
void EMERGENCY(void);
void ResetIOP(void);
void SetDefaultSettings(void);
void TimerInit(void);
u64 Timer(void);
void TimerEnd(void);
char* CheckPath(char* path);
static void AlarmCallback(s32 alarm_id, u16 time, void *common);
int dischandler();


typedef struct
{
    int SKIPLOGO;
    char *KEYPATHS[17][3];
} CONFIG;
CONFIG GLOBCFG;

char* EXECPATHS[3];
int PAD = 0;

int main()
{
    int button, x, j, cnf_size, is_PCMCIA = 0;
    static int config_source = SOURCE_INVALID;
    unsigned char *RAM_p=NULL;
    char *CNFBUFF, *name, *value;
    static int num_buttons = 4, pad_button = 0x0100; // first pad button is L2
    SifInitRpc(0);                                   // Initialize SIFCMD & SIFRPC
    ResetIOP();
    // Initialize SIF services for loading modules and files.
    SifInitIopHeap();
    SifLoadFileInit();
    fioInit(); // NO scr_printf BEFORE here
    init_scr();
    DPRINTF("sbv_patch_enable_lmb\n");
    sbv_patch_enable_lmb(); // The old IOP kernel has no support for LoadModuleBuffer. Apply the patch to enable it.

    DPRINTF("sbv_patch_disable_prefix_check\n");
    sbv_patch_disable_prefix_check(); /* disable the MODLOAD module black/white list, allowing executables to be freely loaded from any device. */

    DPRINTF("Loading SIO2MAN.IRX\n");
    SifExecModuleBuffer(sio2man_irx, size_sio2man_irx, 0, NULL, NULL); /*  Load SDK modules to avoid different behavior across different models*/
    DPRINTF("Loading MCMAN.IRX\n");
    SifExecModuleBuffer(mcman_irx, size_mcman_irx, 0, NULL, NULL);
    DPRINTF("Loading MCSERV.IRX\n");
    SifExecModuleBuffer(mcserv_irx, size_mcserv_irx, 0, NULL, NULL);
    DPRINTF("Loading PADMAN.IRX\n");
    SifExecModuleBuffer(padman_irx, size_padman_irx, 0, NULL, NULL);
    mcInit(MC_TYPE_XMC);
    DPRINTF("Loading USBD.IRX\n");
    SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, NULL);
    delay(3);
#ifdef NO_BDM
    DPRINTF("Loading USBHDFSD.IRX\n");
    SifExecModuleBuffer(usb_mass_irx, size_usb_mass_irx, 0, NULL, NULL);
#else
    DPRINTF("Loading BDM.IRX\n");
    SifExecModuleBuffer(bdm_irx, size_bdm_irx, 0, NULL, NULL);
    DPRINTF("Loading BDMFS_FATFS.IRX\n");
    SifExecModuleBuffer(bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, NULL);
    DPRINTF("Loading USBMASS_BD.IRX\n");
    SifExecModuleBuffer(usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, NULL);
#endif
    delay(3);
    InitOsd(); // Initialize OSD so kernel patches can do their magic
    PadInitPads();
    for (x = 0; x < 3; x++, sleep(1)) {
        PAD = ReadCombinedPadStatus();
        if ((PAD & PAD_R1) && (PAD & PAD_START)) // if ONLY R1+START are pressed...
            EMERGENCY();
    }
    SetDefaultSettings();
    FILE *fp;
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
        DPRINTF("Config nvalid, reading now\n");
        pad_button = 0x0001;
        num_buttons = 16;
        DPRINTF("Check CNF size\n");
        fseek(fp, 0, SEEK_END);
        cnf_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        DPRINTF("Allocating %d bytes for RAM_p\n", cnf_size);
        RAM_p = (unsigned char *)malloc(cnf_size + 1);
        if (RAM_p != NULL) {
            CNFBUFF = RAM_p;
            DPRINTF("Read data INTO the buffer\n");
            int temp;
            if ((temp = fread(RAM_p, 1, cnf_size, fp)) == cnf_size) {
                DPRINTF("Reading finished... Closing fp*\n");
                fclose(fp);
                DPRINTF("NULL Terminate buffer\n");
                CNFBUFF[cnf_size] = '\0';
                int var_cnt = 0;
                char TMP[64];
                for (var_cnt = 0; get_CNF_string(&CNFBUFF, &name, &value); var_cnt++) {
                    //DPRINTF("reading entry %d", var_cnt);
                    if (!strcmp("SKIP_PS2LOGO", name)) {
                        GLOBCFG.SKIPLOGO = atoi(value);
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
                DPRINTF("ERROR: could not read %d bytes of config file, only %d readed\n", cnf_size, temp);
            }
        } else {
            scr_setbgcolor(0x0000ff);
            DPRINTF("Failed to allocate %d+1 bytes!\n", cnf_size);
        }
    }
    else
    {
        DPRINTF("Invalid config, loading hardcoded shit\n");
		for (x = 0; x < 5; x++)
			for (j = 0; j < 3; j++)
				GLOBCFG.KEYPATHS[x][j] = CheckPath(DEFPATH[3 * x + j]);
    }
    if (RAM_p != NULL)
        free(RAM_p);

	//Stores last key during DELAY msec
    u64 tstart;
    TimerInit();
	tstart = Timer();
    DPRINTF("Reading PADs\n");
	//while (Timer() <= (tstart + DELAY))
    while(1)
	{
		//If key was detected
        //DPRINTF("Trying to read PADs\n");
	    PAD = ReadCombinedPadStatus();
		button = pad_button;
		for (x = 0; x < num_buttons; x++) {  // check all pad buttons
			if (PAD & button) {
                DPRINTF("PAD detected\n");
				// if button detected , copy path to corresponding index
				for (j = 0; j < 3; j++)
					EXECPATHS[j] = GLOBCFG.KEYPATHS[x + 1][j];
                for (j = 0; j < 3; j++)
                {
				    EXECPATHS[j] = CheckPath(EXECPATHS[j]);
                    if (exist(EXECPATHS[j]))
                    {
                        scr_setfontcolor(0x00ff00);
                        scr_printf("Loading %s\n", EXECPATHS[j]);
                        if (!is_PCMCIA)
                            PadDeinitPads();
                        RunLoaderElf(EXECPATHS[j], NULL);
                    } else {DPRINTF("%s not found\n", EXECPATHS[j]);}
                }
                break;
			}
			button = button << 1;  // sll of 1 cleared bit to move to next pad button
		}
        if (Timer() <= (tstart + 4000))
        {
            for (j = 0; j < 3; j++)
            {
                if (exist(CheckPath(GLOBCFG.KEYPATHS[0][j])))
                {
                if (!is_PCMCIA)
                    PadDeinitPads();
                RunLoaderElf(CheckPath(GLOBCFG.KEYPATHS[0][j]), NULL);
                }

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

char* CheckPath(char* path)
{
    if (!strncmp("mc?", path, 3))
    {
        path[2] = '0';
        if (exist(path))
        {
            return path;
        }
        else
        {
            path[2] = '1';
            if (exist(path))
                return path;
        }
    }
    if (!strcmp("$CDVD", path))
        dischandler();
    if (!strcmp("$CDVD_NO_PS2LOGO", path))
    {
        GLOBCFG.SKIPLOGO = 1;
        dischandler();
    }
    return path;
}

void SetDefaultSettings(void)
{
    int i, j;
    for (i = 0; i < 17; i++)
        for (j = 0; j < 3; j++)
            GLOBCFG.KEYPATHS[i][j] = NULL;
    GLOBCFG.SKIPLOGO = 1;
}

int dischandler()
{
    scr_clear();
    scr_printf("%s: Activated\n",__func__);
    int OldDiscType, DiscType, ValidDiscInserted, result;
    u32 stat;
    scr_printf("\tEnabling Diagnosis...\n");
    do
    { // 0 = enable, 1 = disable.
        result = sceCdAutoAdjustCtrl(0, &stat);
    } while ((stat & 0x08) || (result == 0));

    // For this demo, wait for a valid disc to be inserted.
    scr_printf("Waiting for disc to be inserted...\n");

    ValidDiscInserted = 0;
    OldDiscType       = -1;
    while (!ValidDiscInserted)
    {
        DiscType = sceCdGetDiskType();
        if (DiscType != OldDiscType)
        {
            scr_printf("New Disc:\t");
            OldDiscType = DiscType;

            switch (DiscType)
            {
                case SCECdNODISC:
                    scr_printf("No Disc\n");
                    break;

                case SCECdDETCT:
                case SCECdDETCTCD:
                case SCECdDETCTDVDS:
                case SCECdDETCTDVDD:
                    scr_printf("Reading Disc...\n");
                    break;

                case SCECdPSCD:
                case SCECdPSCDDA:
                    scr_printf("PlayStation\n");
                    ValidDiscInserted = 1;
                    break;

                case SCECdPS2CD:
                case SCECdPS2CDDA:
                case SCECdPS2DVD:
                    scr_printf("PlayStation 2\n");
                    ValidDiscInserted = 1;
                    break;

                case SCECdCDDA:
                    scr_printf("Audio Disc (not supported by this program)\n");
                    break;

                case SCECdDVDV:
                    scr_printf("DVD Video\n");
                    ValidDiscInserted = 1;
                    break;
                default:
                    scr_printf("Unknown\n");
            }
        }

        // Avoid spamming the IOP with sceCdGetDiskType(), or there may be a deadlock.
        // The NTSC/PAL H-sync is approximately 16kHz. Hence approximately 16 ticks will pass every millisecond.
        SetAlarm(1000 * 16, &AlarmCallback, (void *)GetThreadId());
        SleepThread();
    }

    // Now that a valid disc is inserted, do something.
    // CleanUp() will be called, to deinitialize RPCs. SIFRPC will be deinitialized by the respective disc-handlers.
    switch (DiscType)
    {
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
    //InitPSX();
#endif
}

static void InitPSX()
{
    int result, stat;

    SifInitRpc(0);
    sceCdInit(SCECdINoD);

    // No need to perform boot certification because rom0:OSDSYS does it.
    while (sceCdChgSys(2) != 2) {}; // Switch the drive into PS2 mode.

    // Signal the start of a game, so that the user can use the "quit" game button.
    do
    {
        result = sceCdNoticeGameStart(1, &stat);
    } while ((result == 0) || (stat & 0x80));

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

static void AlarmCallback(s32 alarm_id, u16 time, void *common)
{
    iWakeupThread((int)common);
}

void CleanUp(void)
{
    sceCdInit(SCECdEXIT);
    PadDeinitPads();
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
