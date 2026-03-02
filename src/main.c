#include "main.h"
#include "common.h"
// --------------- glob stuff --------------- //
CONFIG GLOBCFG;
static int config_source = SOURCE_INVALID;
unsigned char *config_buf = NULL; // pointer to allocated config file

char *EXECPATHS[3];
u8 ROMVER[16];

int PAD = 0;

int main(int argc, char *argv[])
{
    u64 tstart;
    int button, x, j, cnf_size;
    static int num_buttons = 4, pad_button = 0x0100; // first pad button is L2
    char *CNFBUFF, *name, *value;

    ResetIOP();
    SifInitIopHeap(); // Initialize SIF services for loading modules and files.
    SifLoadFileInit();
    fioInit(); // NO scr_printf BEFORE here
    init_scr();
    scr_setCursor(0); // get rid of annoying that cursor.
    DPRINTF_INIT()
#ifndef NO_DPRINTF
    DPRINTF("PS2BBL: starting with %d argumments:\n", argc);
    for (x = 0; x < argc; x++)
        DPRINTF("\targv[%d] = [%s]\n", x, argv[x]);
#endif
    scr_putchar(1, 1, 0x303030, '.'); // GBS control does not detect image output with scr debug till the first char is printed
    // print a simple dot to allow gbs control to start displaying video before banner and pad timeout begins to run. othersiwe, users with timeout lower than 4000 will have issues to respond in time

    init_iop_patches();

    UDPTTY_STARTUP();
#ifdef DISC_STOP_AT_BOOT
    sceCdStop();
    sceCdSync(0);
#endif


    SIO2_MC_PAD_STARTUP();

    FILEXIO_STARTUP();

    BDM_USB_STARTUP();


#ifdef MMCE
    j = SifExecModuleBuffer(mmceman_irx, size_mmceman_irx, 0, NULL, &x);
    DPRINTF(" [MMCEMAN]: ID=%d, ret=%d\n", j, x);
#endif

#ifdef MX4SIO
    j = SifExecModuleBuffer(mx4sio_bd_irx, size_mx4sio_bd_irx, 0, NULL, &x);
    DPRINTF(" [MX4SIO_BD]: ID=%d, ret=%d\n", j, x);
#endif

#ifdef HDD
    if (LoadHDDIRX() < 0) // only load HDD crap if filexio and iomanx are up and running
    {
        scr_setbgcolor(0x0000ff);
        scr_clear();
        sleep(4);
    }
#endif
    init_osd_generic();

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
    for (x = SOURCE_CWD; x >= SOURCE_MC0; x--) {
        char *T = CheckPath(CONFIG_PATHS[x]);
        fp = fopen(T, "r");
        if (fp != NULL) {
            config_source = x;
            break;
        }
    }

    if (config_source != SOURCE_INVALID) {
        DPRINTF("valid config on device '%s', reading now\n", SOURCES[config_source]);
        pad_button = 0x0001; // on valid config, change the value of `pad_button` so the pad detection loop iterates all the buttons instead of only those configured on default paths
        num_buttons = 16;
        fseek(fp, 0, SEEK_END);
        cnf_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        DPRINTF("Allocating %d bytes for config\n", cnf_size);
        config_buf = (unsigned char *)malloc(cnf_size + 1);
        if (config_buf != NULL) {
            CNFBUFF = config_buf;
            int temp;
            if ((temp = fread(config_buf, 1, cnf_size, fp)) == cnf_size) {
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
                    if (!strncmp("LOAD_IRX_E", name, 10)) {
                        j = SifLoadStartModule(CheckPath(value), 0, NULL, &x);
                        DPRINTF("# Loaded IRX from config entry [%s] -> [%s]: ID=%d, ret=%d\n", name, value, j, x);
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
                    if (!strcmp("LOGO_DISPLAY", name)) {
                        GLOBCFG.LOGO_DISP = atoi(value);
                        continue;
                    }
                    if (!strncmp("LK_", name, 3)) {
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
                }
            } else {
                fclose(fp);
                DPRINTF("\tERROR: could not read %d bytes of config file, only %d readed\n", cnf_size, temp);
#ifdef REPORT_FATAL_ERRORS
                scr_setfontcolor(0x0000ff);
                scr_printf("\tERROR: could not read %d bytes of config file, only %d readed\n", cnf_size, temp);
                scr_setfontcolor(0xffffff);
#endif
            }
        } else {
            DPRINTF("\tFailed to allocate %d+1 bytes!\n", cnf_size);
#ifdef REPORT_FATAL_ERRORS
            scr_setbgcolor(0x0000ff);
            scr_clear();
            scr_printf("\tFailed to allocate %d+1 bytes!\n", cnf_size);
            sleep(3);
            scr_setbgcolor(0x000000);
            scr_clear();
#endif
        }
#ifdef HDD
        if (config_source == SOURCE_HDD) {

            if (fileXioUmount("pfs0:") < 0)
                DPRINTF("ERROR: Could not unmount 'pfs0:'\n");
        }
#endif
    } else {
        scr_printf("Can't find config, loading hardcoded paths\n");
        for (x = 0; x < 5; x++)
            for (j = 0; j < 3; j++)
                GLOBCFG.KEYPATHS[x][j] = CheckPath(DEFPATH[3 * x + j]);
        sleep(1);
    }

    int R = 0x80, G = 0x80, B = 0x80;
    if (GLOBCFG.OSDHISTORY_READ && (GLOBCFG.LOGO_DISP > 1)) {
        j = 1;
        // Try to load the history file from memory card slot 1
        if (LoadHistoryFile(0) < 0) { // Try memory card slot 2
            if (LoadHistoryFile(1) < 0) {
                DPRINTF("no history files found\n\n");
                j = 0;
            }
        }

        if (j) {
            for (j = 0; j < MAX_HISTORY_ENTRIES; j++) {
                switch (j % 3) {
                    case 0:
                        R += (HistoryEntries[j].LaunchCount * 2);
                        break;
                    case 1:
                        G += (HistoryEntries[j].LaunchCount * 2);
                        break;
                    case 2:
                        B += (HistoryEntries[j].LaunchCount * 2);
                        break;
                    default:
                        B += (HistoryEntries[j].LaunchCount * 2);
                }
            }
            scr_setfontcolor(RBG2INT(B, G, R));
            DPRINTF("New banner color is: #%8x\n", RBG2INT(B, G, R));
        } else {
            DPRINTF("can't find any osd history for banner color\n");
        }
    }
    // Stores last key during DELAY msec
    scr_clear();
    if (GLOBCFG.LOGO_DISP > 1)
        scr_printf("\n\n\n\n%s", BANNER);
    scr_setfontcolor(0xffffff);
    if (GLOBCFG.LOGO_DISP > 1)
        scr_printf(BANNER_FOOTER);
    if (GLOBCFG.LOGO_DISP > 0) {
        scr_printf("\n\n\tModel:\t\t%s\n"
                   "\tPlayStation Driver:\t%s\n"
                   "\tDVD Player:\t%s\n"
                   "\tConfig source:\t%s\n",
                   ModelNameGet(),
                   PS1DRVGetVersion(),
                   DVDPlayerGetVersion(),
                   SOURCES[config_source]);
#ifndef NO_TEMP_DISP
        PrintTemperature();
#endif
    }
    DPRINTF("Timer starts!\n");
    TimerInit();
    tstart = Timer();
    while (Timer() <= (tstart + GLOBCFG.DELAY)) {
        button = pad_button; // reset the value so we can iterate (bit-shift) again
        PAD = ReadCombinedPadStatus_raw();
        for (x = 0; x < num_buttons; x++) { // check all pad buttons
            if (PAD & button) {
                DPRINTF("PAD detected\n");
                // if button detected, copy path to corresponding index
                for (j = 0; j < 3; j++) {
                    EXECPATHS[j] = CheckPath(GLOBCFG.KEYPATHS[x + 1][j]);
                    if (exist(EXECPATHS[j])) {
                        scr_setfontcolor(0x00ff00);
                        scr_printf("\tLoading %s\n", EXECPATHS[j]);
                        CleanUp();
                        RunLoaderElf(EXECPATHS[j], MPART);
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
    DPRINTF("Wait time consummed. running AUTO entry\n");
    TimerEnd();
    for (j = 0; j < 3; j++) {
        EXECPATHS[j] = CheckPath(GLOBCFG.KEYPATHS[0][j]);
        if (exist(EXECPATHS[j])) {
            scr_setfontcolor(0x00ff00);
            scr_printf("\tLoading %s\n", EXECPATHS[j]);
            CleanUp();
            RunLoaderElf(EXECPATHS[j], MPART);
        } else {
            scr_printf("%s %-15s\r", EXECPATHS[j], "not found");
        }
    }

    scr_clear();
    scr_setfontcolor(0x00ffff);
    scr_printf("\n\n\tEND OF EXECUTION REACHED\nCould not find any of the default applications\nCheck your config file for the LK_AUTO_E# entries\nOr press a key while logo displays to run the bound application\npress R1+START to enter emergency mode");
    scr_setfontcolor(0xffffff);
    while (1) {
        sleep(1);
        PAD = ReadCombinedPadStatus_raw();
        if ((PAD & PAD_R1) && (PAD & PAD_START)) // if ONLY R1+START are pressed...
            EMERGENCY();
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
            CleanUp();
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
#ifdef HDD
        if (!strcmp("$HDDCHECKER", path))
            HDDChecker();
#endif
        if (!strcmp("$CREDITS", path))
            credits();
        if (!strcmp("$OSDSYS", path))
            runOSDNoUpdate();
        if (!strncmp("$RUNKELF:", path, strlen("$RUNKELF:"))) {
            runKELF(CheckPath(path + strlen("$RUNKELF:"))); // pass to runKELF the path without the command token, digested again by CheckPath()
        }
    }
    if (!strncmp("mc?", path, 3)) {
        path[2] = (config_source == SOURCE_MC1) ? '1' : '0';
        if (exist(path)) {
            return path;
        } else {
            path[2] = (config_source == SOURCE_MC1) ? '0' : '1';
            if (exist(path))
                return path;
        }
#ifdef MMCE
    } else if (!strncmp("mmce?", path, 5)) {
        path[4] = (config_source == SOURCE_MMCE1) ? '1' : '0';
        if (exist(path)) {
            return path;
        } else {
            path[4] = (config_source == SOURCE_MMCE1) ? '0' : '1';
            if (exist(path))
                return path;
        }
#endif
#ifdef HDD
    } else if (!strncmp("hdd", path, 3)) {
        if (MountParty(path) < 0) {
            DPRINTF("-{%s}-\n", path);
            return path;
        } else {
            DPRINTF("--{%s}--{%s}\n", path, strstr(path, "pfs:"));
            return strstr(path, "pfs:");
        } // leave path as pfs:/blabla
        if (!MountParty(path))
            return strstr(path, "pfs:");
#endif
#ifdef MX4SIO
    } else if (!strncmp("massX:", path, 6)) {
        int x = LookForBDMDevice();
        if (x >= 0)
            path[4] = '0' + x;
#endif
    }
    return path;
}

void SetDefaultSettings(void)
{
    int i, j;
    for (i = 0; i < 17; i++)
        for (j = 0; j < 3; j++)
            GLOBCFG.KEYPATHS[i][j] = "isra:/";
    GLOBCFG.SKIPLOGO = 0;
    GLOBCFG.OSDHISTORY_READ = 1;
    GLOBCFG.DELAY = DEFDELAY;
    GLOBCFG.TRAYEJECT = 0;
    GLOBCFG.LOGO_DISP = 2;
}


#ifdef MX4SIO
int LookForBDMDevice(void)
{
    static char mass_path[] = "massX:";
    static char DEVID[5];
    int dd;
    int x = 0;
    for (x = 0; x < 5; x++) {
        mass_path[4] = '0' + x;
        if ((dd = fileXioDopen(mass_path)) >= 0) {
            int *intptr_ctl = (int *)DEVID;
            *intptr_ctl = fileXioIoctl(dd, USBMASS_IOCTL_GET_DRIVERNAME, "");
            close(dd);
            if (!strncmp(DEVID, "sdc", 3)) {
                DPRINTF("%s: Found MX4SIO device at mass%d:/\n", __func__, x);
                return x;
            }
        }
    }
    return -1;
}
#endif



#ifdef DEV9
int loadDEV9(void)
{
    if (!dev9_loaded) {
        int ID, RET;
        ID = SifExecModuleBuffer(&ps2dev9_irx, size_ps2dev9_irx, 0, NULL, &RET);
        DPRINTF("[DEV9]: ret=%d, ID=%d\n", RET, ID);
        if (ID < 0 && RET == 1) // ID smaller than 0: issue reported from modload | RET == 1: driver returned no resident end
            return 0;
        dev9_loaded = 1;
    }
    return 1;
}
#endif


#ifdef HDD
static int CheckHDD(void)
{
    int ret = fileXioDevctl("hdd0:", HDIOC_STATUS, NULL, 0, NULL, 0);
    /* 0 = HDD connected and formatted, 1 = not formatted, 2 = HDD not usable, 3 = HDD not connected. */
    DPRINTF("%s: HDD status is %d\n", __func__, ret);
    if ((ret >= 3) || (ret < 0))
        return -1;
    return ret;
}

int LoadHDDIRX(void)
{
    int ID, RET, HDDSTAT;
    static const char hddarg[] = "-o"
                                 "\0"
                                 "4"
                                 "\0"
                                 "-n"
                                 "\0"
                                 "20";
    //static const char pfsarg[] = "-n\0" "24\0" "-o\0" "8";

    if (!loadDEV9())
        return -1;

    ID = SifExecModuleBuffer(&poweroff_irx, size_poweroff_irx, 0, NULL, &RET);
    DPRINTF(" [POWEROFF]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -2;

    poweroffInit();
    poweroffSetCallback(&poweroffCallback, NULL);
    DPRINTF("PowerOFF Callback installed...\n");

    ID = SifExecModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL, &RET);
    DPRINTF(" [ATAD]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -3;

    ID = SifExecModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg, &RET);
    DPRINTF(" [PS2HDD]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -4;

    HDDSTAT = CheckHDD();
    HDD_USABLE = !(HDDSTAT < 0);

    /* PS2FS.IRX */
    if (HDD_USABLE) {
        ID = SifExecModuleBuffer(&ps2fs_irx, size_ps2fs_irx, 0, NULL, &RET);
        DPRINTF(" [PS2FS]: ret=%d, ID=%d\n", RET, ID);
        if (ID < 0 || RET == 1)
            return -5;
    }

    return 0;
}

int MountParty(const char *path)
{
    int ret = -1;
    DPRINTF("%s: %s\n", __func__, path);
    char *BUF = NULL;
    BUF = strdup(path); //use strdup, otherwise, path will become `hdd0:`
    char MountPoint[40];
    if (getMountInfo(BUF, NULL, MountPoint, NULL)) {
        mnt(MountPoint);
        if (BUF != NULL)
            free(BUF);
        strcpy(PART, MountPoint);
        strcat(PART, ":");
        return 0;
    } else {
        DPRINTF("ERROR: could not process path '%s'\n", path);
        PART[0] = '\0';
    }
    if (BUF != NULL)
        free(BUF);
    return ret;
}

int mnt(const char *path)
{
    DPRINTF("Mounting '%s'\n", path);
    if (fileXioMount("pfs0:", path, FIO_MT_RDONLY) < 0) // mount
    {
        DPRINTF("Mount failed. unmounting pfs0 and trying again...\n");
        if (fileXioUmount("pfs0:") < 0) //try to unmount then mount again in case it got mounted by something else
        {
            DPRINTF("Unmount failed!!!\n");
        }
        if (fileXioMount("pfs0:", path, FIO_MT_RDONLY) < 0) {
            DPRINTF("mount failed again!\n");
            return -4;
        } else {
            DPRINTF("Second mount succed!\n");
        }
    } else
        DPRINTF("mount successfull on first attemp\n");
    return 0;
}

void HDDChecker()
{
    char ErrorPartName[64];
    const char *HEADING = "HDD Diagnosis routine";
    int ret = -1;
    scr_clear();
    scr_printf("\n\n%*s%s\n", ((80 - strlen(HEADING)) / 2), "", HEADING);
    scr_setfontcolor(0x0000FF);
    ret = fileXioDevctl("hdd0:", HDIOC_STATUS, NULL, 0, NULL, 0);
    if (ret == 0 || ret == 1)
        scr_setfontcolor(0x00FF00);
    if (ret != 3) {
        scr_printf("\t\t - HDD CONNECTION STATUS: %d\n", ret);
        /* Check ATA device S.M.A.R.T. status. */
        ret = fileXioDevctl("hdd0:", HDIOC_SMARTSTAT, NULL, 0, NULL, 0);
        if (ret != 0)
            scr_setfontcolor(0x0000ff);
        else
            scr_setfontcolor(0x00FF00);
        scr_printf("\t\t - S.M.A.R.T STATUS: %d\n", ret);
        /* Check for unrecoverable I/O errors on sectors. */
        ret = fileXioDevctl("hdd0:", HDIOC_GETSECTORERROR, NULL, 0, NULL, 0);
        if (ret != 0)
            scr_setfontcolor(0x0000ff);
        else
            scr_setfontcolor(0x00FF00);
        scr_printf("\t\t - SECTOR ERRORS: %d\n", ret);
        /* Check for partitions that have errors. */
        ret = fileXioDevctl("hdd0:", HDIOC_GETERRORPARTNAME, NULL, 0, ErrorPartName, sizeof(ErrorPartName));
        if (ret != 0)
            scr_setfontcolor(0x0000ff);
        else
            scr_setfontcolor(0x00FF00);
        scr_printf("\t\t - CORRUPTED PARTITIONS: %d\n", ret);
        if (ret != 0) {
            scr_printf("\t\tpartition: %s\n", ErrorPartName);
        }
    } else
        scr_setfontcolor(0x00FFFF), scr_printf("Skipping test, HDD is not connected\n");
    scr_setfontcolor(0xFFFFFF);
    scr_printf("\t\tWaiting for 10 seconds...\n");
    sleep(10);
}
/// @brief poweroff callback function
/// @note only expansion bay models will properly make use of this. the other models will run the callback but will poweroff themselves before reaching function end...
void poweroffCallback(void *arg)
{
    fileXioDevctl("pfs:", PDIOC_CLOSEALL, NULL, 0, NULL, 0);
    while (fileXioDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0) {};
    // As required by some (typically 2.5") HDDs, issue the SCSI STOP UNIT command to avoid causing an emergency park.
    fileXioDevctl("mass:", USBMASS_DEVCTL_STOP_ALL, NULL, 0, NULL, 0);
    /* Power-off the PlayStation 2. */
    poweroffShutdown();
}

#endif
int dischandler()
{
    int OldDiscType, DiscType, ValidDiscInserted, result, first_run = 1;
    u32 STAT;

    scr_clear();
    scr_printf("\n\t%s: Activated\n", __func__);

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
                    if (first_run) {
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
                    scr_printf("Unknown (%d)\n", DiscType);
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
    while (!SifIopReset("", 0)) {};
#else
    /* sp193: We need some of the PSX's CDVDMAN facilities, but we do not want to use its (too-)new FILEIO module.
       This special IOPRP image contains a IOPBTCONF list that lists PCDVDMAN instead of CDVDMAN.
       PCDVDMAN is the board-specific CDVDMAN module on all PSX, which can be used to switch the CD/DVD drive operating mode.
       Usually, I would discourage people from using board-specific modules, but I do not have a proper replacement for this. */
    while (!SifIopRebootBuffer(psx_ioprp, size_psx_ioprp)) {};
#endif
    while (!SifIopSync()) {};

#ifdef PSX
    InitPSX();
#endif
}

static void AlarmCallback(s32 alarm_id, u16 time, void *common)
{
    iWakeupThread((int)common);
}

void CleanUp(void)
{
    sceCdInit(SCECdEXIT);
    if (config_buf) {
        free(config_buf);
        config_buf = NULL;
    }
    PadDeinitPads();
}

void credits(void)
{
    scr_clear();
    scr_printf("\n\n");
    scr_printf("%s%s", BANNER, BANNER_FOOTER);
    scr_printf("\n"
               "\n"
               "\tBased on SP193 OSD Init samples.\n"
               "\t\tall credits go to him\n"
               "\tThanks to: fjtrujy, uyjulian, asmblur and AKuHAK\n"
               "\tbuild hash [" COMMIT_HASH "]\n"
               "\t\tcompiled on "__DATE__
               " "__TIME__
               "\n"
#ifdef MX4SIO
               " MX4SIO"
#endif
#ifdef HDD
               " HDD "
#endif

    );
    while (1) {};
}

void runOSDNoUpdate(void)
{
    char *args[3] = {"SkipHdd", "BootBrowser", "SkipMc"};
    CleanUp();
    SifExitCmd();
    ExecOSD(3, args);
}

#ifndef NO_TEMP_DISP
void PrintTemperature()
{
    // Based on PS2Ident libxcdvd from SP193
    unsigned char in_buffer[1], out_buffer[16];
    int stat = 0;

    memset(&out_buffer, 0, 16);

    in_buffer[0] = 0xEF;
    if (sceCdApplySCmd(0x03, in_buffer, sizeof(in_buffer), out_buffer /*, sizeof(out_buffer)*/) != 0) {
        stat = out_buffer[0];
    }

    if (!stat) {
        unsigned short temp = out_buffer[1] * 256 + out_buffer[2];
        scr_printf("\tTemp: %02d.%02dC\n", (temp - (temp % 128)) / 128, (temp % 128));
    } else {
        DPRINTF("Failed 0x03 0xEF command. stat=%x \n", stat);
    }
}
#endif

/* BELOW THIS POINT ALL MACROS and MISC STUFF MADE TO REDUCE BINARY SIZE WILL BE PLACED */

#if defined(DUMMY_TIMEZONE)
void _libcglue_timezone_update()
{
}
#endif

#if defined(KERNEL_NOPATCH)
DISABLE_PATCHED_FUNCTIONS();
#endif

DISABLE_EXTRA_TIMERS_FUNCTIONS();
PS2_DISABLE_AUTOSTART_PTHREAD();
