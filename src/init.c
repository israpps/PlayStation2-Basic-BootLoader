#include "main.h"
extern u8 ROMVER[16];
void init_osd_generic()
{
    int fd, j, result;
    u32 STAT;
    if ((fd = open("rom0:ROMVER", O_RDONLY)) >= 0) {
        read(fd, ROMVER, sizeof(ROMVER));
        close(fd);
    }
    j = SifLoadModule("rom0:ADDDRV", 0, NULL); // Load ADDDRV. The OSD has it listed in rom0:OSDCNF/IOPBTCONF, but it is otherwise not loaded automatically.
    DPRINTF(" [ADDDRV]: %d\n", j);

    // Initialize libcdvd & supplement functions (which are not part of the ancient libcdvd library we use).
    sceCdInit(SCECdINoD);
    cdInitAdd();

    DPRINTF("init OSD system paths\n");
    OSDInitSystemPaths();

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
    DPRINTF("Saving OSD configuration\n");
    OSDConfigApply();

    /*  Try to enable the remote control, if it is enabled.
        Indicate no hardware support for it, if it cannot be enabled. */
    DPRINTF("trying to enable remote control\n");
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
    DPRINTF("Setting vmode\n");
    SetGsVParam(OSDConfigGetVideoOutput() == VIDEO_OUTPUT_RGB ? VIDEO_OUTPUT_RGB : VIDEO_OUTPUT_COMPONENT);
}

void init_iop_patches()
{
    DPRINTF("enabling LoadModuleBuffer\n");
    sbv_patch_enable_lmb(); // The old IOP kernel has no support for LoadModuleBuffer. Apply the patch to enable it.

    DPRINTF("disabling MODLOAD device blacklist/whitelist\n");
    sbv_patch_disable_prefix_check(); /* disable the MODLOAD module black/white list, allowing executables to be freely loaded from any device. */
}

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
#ifdef REPORT_FATAL_ERRORS
    } else {
        scr_setfontcolor(0x0000ff);
        scr_printf("\tERROR: Could not certify CDVD Boot. ROMVER was NULL\n");
        scr_setfontcolor(0xffffff);
#endif
    }

    // This disables DVD Video Disc playback. This functionality is restored by loading a DVD Player KELF.
    /*    Hmm. What should the check for STAT be? In v1.xx, it seems to be a check against 0x08. In v2.20, it checks against 0x80.
          The HDD Browser does not call this function, but I guess it would check against 0x08. */
    /*  do
     {
         sceCdForbidDVDP(&STAT);
     } while (STAT & 0x08); */
}

#ifdef PSX
void InitPSX()
{
    int result, STAT;

    SifInitRpc(0);
    sceCdInit(SCECdINoD);

    // No need to perform boot certification because rom0:OSDSYS does it.
    while (custom_sceCdChgSys(sceDVRTrayModePS2) != sceDVRTrayModePS2) {}; // Switch the drive into PS2 mode.

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

    /// WARNING: untill further investigation, the memory mode should remain on 64mb. changing it to 32 breaks SDK ELF Loader
    /// SetMemoryMode(1);
    ///_InitTLB();

    while (!SifIopSync()) {};
}
#endif

void iop_init_sio2_related()
{ 
    int j, x;
#ifdef USE_ROM_SIO2MAN
    j = SifLoadStartModule("rom0:SIO2MAN", 0, NULL, &x);
    DPRINTF(" [SIO2MAN]: ID=%d, ret=%d\n", j, x);
#else
    j = SifExecModuleBuffer(sio2man_irx, size_sio2man_irx, 0, NULL, &x);
    DPRINTF(" [SIO2MAN]: ID=%d, ret=%d\n", j, x);
#endif
#ifdef USE_ROM_MCMAN
    j = SifLoadStartModule("rom0:MCMAN", 0, NULL, &x);
    DPRINTF(" [MCMAN]: ID=%d, ret=%d\n", j, x);
    j = SifLoadStartModule("rom0:MCSERV", 0, NULL, &x);
    DPRINTF(" [MCSERV]: ID=%d, ret=%d\n", j, x);
    mcInit(MC_TYPE_MC);
#else
    j = SifExecModuleBuffer(mcman_irx, size_mcman_irx, 0, NULL, &x);
    DPRINTF(" [MCMAN]: ID=%d, ret=%d\n", j, x);
    j = SifExecModuleBuffer(mcserv_irx, size_mcserv_irx, 0, NULL, &x);
    DPRINTF(" [MCSERV]: ID=%d, ret=%d\n", j, x);
    mcInit(MC_TYPE_XMC);
#endif
#ifdef USE_ROM_PADMAN
    j = SifLoadStartModule("rom0:PADMAN", 0, NULL, &x);
    DPRINTF(" [PADMAN]: ID=%d, ret=%d\n", j, x);
#else
    j = SifExecModuleBuffer(padman_irx, size_padman_irx, 0, NULL, &x);
    DPRINTF(" [PADMAN]: ID=%d, ret=%d\n", j, x);
#endif
}


#ifdef UDPTTY
void loadUDPTTY()
{
    int ID, RET;
    ID = SifExecModuleBuffer(&netman_irx, size_netman_irx, 0, NULL, &RET);
    DPRINTF(" [NETMAN]: ret=%d, ID=%d\n", RET, ID);
    ID = SifExecModuleBuffer(&smap_irx, size_smap_irx, 0, NULL, &RET);
    DPRINTF(" [SMAP]: ret=%d, ID=%d\n", RET, ID);
    ID = SifExecModuleBuffer(&ps2ip_irx, size_ps2ip_irx, 0, NULL, &RET);
    DPRINTF(" [PS2IP]: ret=%d, ID=%d\n", RET, ID);
    ID = SifExecModuleBuffer(&udptty_irx, size_udptty_irx, 0, NULL, &RET);
    DPRINTF(" [UDPTTY]: ret=%d, ID=%d\n", RET, ID);
    sleep(3);
}

void udptty_start()
{
    if (loadDEV9())
        loadUDPTTY();
}
#endif


int LoadUSBIRX(void)
{
    int ID, RET;

// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ID = SifExecModuleBuffer(bdm_irx, size_bdm_irx, 0, NULL, &RET);
#else
    ID = SifLoadStartModule(CheckPath("mc?:/PS2BBL/BDM.IRX"), 0, NULL, &RET);
#endif
    DPRINTF(" [BDM]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -1;
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ID = SifExecModuleBuffer(bdmfs_fatfs_irx, size_bdmfs_fatfs_irx, 0, NULL, &RET);
#else
    ID = SifLoadStartModule(CheckPath("mc?:/PS2BBL/BDMFS_FATFS.IRX"), 0, NULL, &RET);
#endif
    DPRINTF(" [BDMFS_FATFS]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -2;
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ID = SifExecModuleBuffer(usbd_irx, size_usbd_irx, 0, NULL, &RET);
#else
    ID = SifLoadStartModule(CheckPath("mc?:/PS2BBL/USBD.IRX"), 0, NULL, &RET);
#endif
    delay(3);
    DPRINTF(" [USBD]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -3;
// ------------------------------------------------------------------------------------ //
#ifdef HAS_EMBEDDED_IRX
    ID = SifExecModuleBuffer(usbmass_bd_irx, size_usbmass_bd_irx, 0, NULL, &RET);
#else
    ID = SifLoadStartModule(CheckPath("mc?:/PS2BBL/USBMASS_BD.IRX"), 0, NULL, &RET);
#endif
    DPRINTF(" [USBMASS_BD]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -4;
    // ------------------------------------------------------------------------------------ //
    struct stat buffer;
    int ret = -1;
    int retries = 50;

    while (ret != 0 && retries > 0) {
        ret = stat("mass:/", &buffer);
        /* Wait until the device is ready */
        nopdelay();

        retries--;
    }
    return 0;
}


#ifdef FILEXIO
int LoadIOX(void)
{
    int ID, RET;
    ID = SifExecModuleBuffer(&iomanX_irx, size_iomanX_irx, 0, NULL, &RET);
    DPRINTF(" [IOMANX]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -1;

    /* FILEXIO.IRX */
    ID = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, &RET);
    DPRINTF(" [FILEXIO]: ret=%d, ID=%d\n", RET, ID);
    if (ID < 0 || RET == 1)
        return -2;

    RET = fileXioInit();
    DPRINTF("fileXioInit: %d\n", RET);
    return 0;
}

void load_filexio()
{
    if (LoadIOX() < 0) {
        GS_BGCOLOUR(BGR_BLUE);
        sleep(4);
    }
}
#endif

void bdm_usb()
{
    int j = LoadUSBIRX();
    if (j != 0) {
        scr_setfontcolor(0x0000ff);
        scr_printf("ERROR: could not load USB modules (%d)\n", j);
        scr_setfontcolor(0xffffff);
#ifdef HAS_EMBEDDED_IRX //we have embedded IRX... something bad is going on if this condition executes. add a wait time for user to know something is wrong
        sleep(1);
#endif
    }
}

#ifdef DEV9
int dev9_loaded = 0;
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
#include <hdd-ioctl.h>
#include <io_common.h>
#include <assert.h>
#include <libpwroff.h>
char PART[128] = "\0";
int HDD_USABLE = 0;

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