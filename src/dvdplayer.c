#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <kernel.h>
#include <sifcmd.h>
#include <libcdvd.h>
#include <libmc.h>
#include <errno.h>
#include <unistd.h>

#include "dvdplayer.h"
#include "OSDInit.h"
#include "OSDHistory.h"

void CleanUp(void);

#define DVD_PLAYER_VER_LEN 32

struct DVDPlayer
{
    char ver[DVD_PLAYER_VER_LEN];
    int major, minor;
    int region;
};

static struct DVDPlayer ROMDVDPlayer;

static char dvdid[] = "rom1:DVDID?";
static char dvdver[] = "rom1:DVDVER?";

static char eromdrvCMD[] = "-k rom1:EROMDRV?";
static char dvdelfCMD[] = "-x erom0:DVDELF";

static int readMCFile(int fd, void *buffer, int len);
static int ReadFileFromMC(int port, int slot, const char *file, void *buffer, int len);
static int getStatMC(int port, int slot, int *type, int *free, int *format);
static int openMCFile(int port, int slot, const char *path, int mode);
static int closeMCFile(int fd);
static int CheckFileFromMC(int port, int slot, const char *file);

extern char ConsoleROMVER[ROMVER_MAX_LEN];

static int readMCFile(int fd, void *buffer, int len)
{
    int result;

    if (sceMcRead(fd, buffer, len) == 0) {
        return (sceMcSync(0, NULL, &result) == 1 ? result : 0);
    } else
        return -10;
}

static int ReadFileFromMC(int port, int slot, const char *file, void *buffer, int len)
{
    int fd, bytesRead;

    if (getStatMC(port, slot, NULL, NULL, NULL) >= -2) {
        if ((fd = openMCFile(port, slot, file, 1)) >= 0) {
            if ((bytesRead = readMCFile(fd, buffer, len)) < 0)
                return -1;

            return (closeMCFile(fd) >= 0 ? bytesRead : -1);
        } else
            return -1;
    } else
        return -1;
}

static int getStatMC(int port, int slot, int *type, int *free, int *format)
{
    int result;

    if ((result = sceMcGetInfo(port, slot, type, free, format)) == 0) {
        return (sceMcSync(0, NULL, &result) == 1 ? result : -11);
    } else
        return -10;
}

static int openMCFile(int port, int slot, const char *path, int mode)
{
    int result, type, format;

    getStatMC(port, slot, &type, NULL, &format);
    if (type == sceMcTypePS2 && format == 1) {
        if ((result = sceMcOpen(port, slot, path, mode)) == 0) {
            return (sceMcSync(0, NULL, &result) == 1 ? result : -11);
        } else
            return -10;
    } else
        return -12;
}

static int closeMCFile(int fd)
{
    int result;

    if ((result = sceMcClose(fd)) != 0)
        return -10;

    return (sceMcSync(0, NULL, &result) == 1 ? result : -11);
}

static int CheckFileFromMC(int port, int slot, const char *file)
{
    int fd;

    if ((fd = openMCFile(port, slot, file, O_RDONLY)) >= 0) {
        closeMCFile(fd);
        return 0;
    }

    return 1;
}

/*  Note: this is a mesh-up of the two versions of this function.
    Originally, the OSDSYS from ROM v2.20 had two similar versions, one for
    DVDPlayerGetVersion() and the other for DVDPlayerGetUpdateVersion().

    Both had their own versions of the memory-card access functions,
    but the functions for DVDPlayerGetVersion seemed to have some additional code
    (perhaps for updating access status with some other part).

    Both also did different things upon finding a valid update file (see below).    */
static int DVDPlayerUpdateCheck(int *pPort, int *pSlot, char *pVersion)
{
    char dvdplayer_id[64], dvdplayer_e_ver[64], version[32], id[32], region;
    const char *pChar;
    int port, slot, result, major, minor;

    sprintf(dvdplayer_id, "%s/%s", OSDGetDVDPLExecFolder(), "dvdplayer.id");
    // In OSD v1.0x, the browser used to check either "dvdplayer-j.ver" or "dvdplayer-e.ver", depending on the language setting.
    sprintf(dvdplayer_e_ver, "%s/%s", OSDGetDVDPLExecFolder(), "dvdplayer-e.ver");

    for (port = 0; port < 2; port++) {
        for (slot = 0; slot < 1; slot++) {
            result = ReadFileFromMC(port, slot, dvdplayer_id, id, sizeof(id));
            if (result >= 0) {
                if (result < sizeof(id)) // NULL-terminate, but truncate if too long
                    id[result] = '\0';
                else
                    id[sizeof(id) - 1] = '\0';

                result = ReadFileFromMC(port, slot, dvdplayer_e_ver, version, sizeof(version));
                if (result >= 0) {
                    if (result < sizeof(version)) // NULL-terminate, but truncate if too long
                        version[result] = '\0';
                    else
                        version[sizeof(version) - 1] = '\0';

                    major = atoi(id);

                    for (pChar = id; *pChar >= '0' && *pChar <= '9'; pChar++) {
                    };

                    if (*pChar == '.') {
                        minor = atoi(pChar + 1);

                        for (; *pChar >= '0' && *pChar <= '9'; pChar++) {
                        };

                        result = 0;
                        region = *pChar;
                    } else { // Missing dot
                        major = 0;
                        minor = 0;
                        region = 0;
                        result = -1;
                    }

                    if (result == 0) {
                        result = -1;
                        if ((ROMDVDPlayer.region == -2) || (ROMDVDPlayer.region == region)) {
                            result = (major > ROMDVDPlayer.major) || ((major == ROMDVDPlayer.major) && (minor > ROMDVDPlayer.minor)) ? 0 : -1;
                        }

                        if (result == 0) { // DVD Player update of the right region exists and is newer.
#ifdef PROHBIT_DVD_0100
                                           // Ensure that the banned DVD Player is not being booted.
                            if (strcmp(version, "1.00\n") != 0 && strcmp(version, "1.01\n") != 0) {
#endif
                                // This is done for DVDPlayerGetUpdateVersion().
                                if (pPort != NULL && pSlot != NULL) {
                                    *pPort = port;
                                    *pSlot = slot;
                                }

                                // This is done for DVDPlayerGetVersion(). The check for the pointer also existed, even though it checks against a buffer's address.
                                if (pVersion != NULL) {
                                    strncpy(pVersion, version, DVD_PLAYER_VER_LEN - 1);
                                    pVersion[DVD_PLAYER_VER_LEN - 1] = '\0';
                                }

                                return 0;
#ifdef PROHBIT_DVD_0100
                            }
#endif
                        }
                    }
                }
            }
        }
    }

    return -1;
}

static int DVDPlayerGetUpdateVersion(const char *file, int *pPort, int *pSlot)
{
    if (DVDPlayerUpdateCheck(pPort, pSlot, NULL) == 0) { // DVD Player update of the right region exists and is newer.
        return (CheckFileFromMC(*pPort, *pSlot, file) == 0 ? 0 : -1);
    }

    return -1;
}

static int CheckDVDPlayerUpdate(void)
{
    /// ISRA:original code has a 1024bytes buffer for path. wich is ridiculous for a string that will ALWAYS have 31 chars ("B?EXEC-DVDPLAYER/dvdplayer.elf")
    char path[33];
    int port, slot, fd;

    sprintf(path, "%s/%s", OSDGetDVDPLExecFolder(), "dvdplayer.elf");
    if (DVDPlayerGetUpdateVersion(path, &port, &slot) == 0) {
        return port;
    } else {
        if (OSDGetDVDPlayerRegion(&dvdid[10]) == 0 || (fd = open(dvdid, O_RDONLY)) < 0)
            fd = open("rom1:DVDID", O_RDONLY);

        if (fd < 0)
            return -1;
        close(fd);

        return 2;
    }
}

int DVDPlayerBoot(void)
{
    char *args[4];
    int port;
    /// ISRA: this buffer used to be 1024 bytes. reduced it because it was too much for a simple 38 chars string ("-x mc?:B?EXEC-DVDPLAYER/dvdplayer.elf")
    char dvdplayerCMD[48]; // This was originally static/global.

    UpdatePlayHistory("DVDVIDEO");
    port = CheckDVDPlayerUpdate();
    if (port >= 0) {
        CleanUp(); // Deinit RPCs
        SifExitCmd();

        if (port >= 2) { // ROM DVD Player

            args[0] = eromdrvCMD;
            args[1] = "-m erom0:UDFIO";
            args[2] = dvdelfCMD;

            // Handle region-specific DVD Player of newer consoles.
            if (OSDGetDVDPlayerRegion(&dvdelfCMD[14]) != 0 && dvdelfCMD[14] != '\0') {
                eromdrvCMD[15] = dvdelfCMD[14];
                dvdelfCMD[12] = 'P';   // Change 'E' to 'P': DVDELF -> DVDPLx
            } else {                   // Restore the 'E' and 'F'
                eromdrvCMD[15] = '\0'; // Replace '?' with a NULL.
                dvdelfCMD[12] = 'E';
                dvdelfCMD[14] = 'F';
            }

            LoadExecPS2("moduleload2 rom1:UDNL rom1:DVDCNF", 3, args);
        } else { // DVD Player Update
            args[0] = "-m rom0:SIO2MAN";
            args[1] = "-m rom0:MCMAN";
            args[2] = "-m rom0:MCSERV";

            sprintf(dvdplayerCMD, "-x mc%d:%s/%s", port, OSDGetDVDPLExecFolder(), "dvdplayer.elf");
            args[3] = dvdplayerCMD;

            LoadExecPS2("moduleload", 4, args);
        }

        return 0;
    } else
        return EIO;
}

int DVDPlayerInit(void)
{
    const char *pChar;
    char id[32];
    int fd, result;

    ROMDVDPlayer.ver[0] = '\0';
    ROMDVDPlayer.minor = -1;
    ROMDVDPlayer.major = -1;
    ROMDVDPlayer.region = -1;

    if (OSDGetDVDPlayerRegion(&dvdid[10]) == 0 || (fd = open(dvdid, O_RDONLY)) < 0)
        fd = open("rom1:DVDID", O_RDONLY);

    if (fd < 0) { // Not having a DVD player is not an error.

        // 0100J and 0101J has no ROM DVD Player. This rule is copied from the HDD Browser, as it is absent in the normal browser.
        if (ConsoleROMVER[0] == '0' && ConsoleROMVER[1] == '1' && ConsoleROMVER[2] == '0' && (ConsoleROMVER[3] == '0' || ConsoleROMVER[3] == '1') && ConsoleROMVER[4] == 'J')
            ROMDVDPlayer.region = 'J';
        // I guess, this would be another possibility for such a case, but not used in practice:
        // ROMDVDPlayer.region = -2;

        return 0;
    }

    read(fd, id, sizeof(id));
    close(fd);

    ROMDVDPlayer.major = atoi(id);

    for (pChar = id; *pChar >= '0' && *pChar <= '9'; pChar++) {
    };

    if (*pChar == '.') {
        ROMDVDPlayer.minor = atoi(pChar + 1);

        for (; *pChar >= '0' && *pChar <= '9'; pChar++) {
        };

        result = 0;
        ROMDVDPlayer.region = *pChar;
    } else { // Missing dot
        ROMDVDPlayer.major = 0;
        ROMDVDPlayer.minor = 0;
        ROMDVDPlayer.region = 0;
        result = -1;
    }

    if (result == 0) {
        if (OSDGetDVDPlayerRegion(&dvdver[11]) == 0 || (fd = open(dvdver, O_RDONLY)) < 0)
            fd = open("rom1:DVDVER", O_RDONLY);

        if (fd >= 0) {
            result = read(fd, ROMDVDPlayer.ver, sizeof(ROMDVDPlayer.ver));
            close(fd);

            // NULL-terminate, only if non-error
            ROMDVDPlayer.ver[result >= 0 ? result : 0] = '\0';
        }
    }

    // The original always returned -1. But we'll return something more meaningful.
    return (result >= 0 ? 0 : -1);
}

const char *DVDPlayerGetVersion(void)
{
    static char DVDPlayerUpdateVer[DVD_PLAYER_VER_LEN] = "";

    if (DVDPlayerUpdateCheck(NULL, NULL, DVDPlayerUpdateVer) == 0) { // DVD Player update of the right region exists and is newer.
        return DVDPlayerUpdateVer;
    }

    // DVD Player Update does not exist. Use DVD ROM version.
    return ROMDVDPlayer.ver;
}
