#ifndef MAIN_H
#define MAIN_H
#define NEWLIB_PORT_AWARE

#include <tamtypes.h>
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

#ifdef DEV9
static int dev9_loaded = 0;
int loadDEV9(void);
#endif

// For avoiding define NEWLIB_AWARE
void fioInit();
#define RBG2INT(R, G, B) ((0 << 24) + (R << 16) + (G << 8) + B)
#include "irx_import.h"
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
int LoadUSBIRX(void);
void runOSDNoUpdate(void);
#ifdef PSX
static void InitPSX();
#endif
#ifndef NO_TEMP_DISP
void PrintTemperature();
#endif
#ifdef HDD
int LoadHDDIRX(void); // Load HDD IRXes
int LoadFIO(void); // Load FileXio and it´s dependencies
int MountParty(const char* path); ///processes strings in the format `hdd0:/$PARTITION:pfs:$PATH_TO_FILE/` to mount partition
int mnt(const char* path); ///mount partition specified on path
#endif

#ifdef UDPTTY
void loadUDPTTY();
#endif

#ifdef HDD
#include <hdd-ioctl.h>
#include <io_common.h>
#include <assert.h>
#include <libpwroff.h>
char PART[128] = "\0";
int HDD_USABLE = 0;
#define MPART PART
int LoadHDDIRX(void); // Load HDD IRXes
int MountParty(const char* path); ///processes strings in the format `hdd0:/$PARTITION:pfs:$PATH_TO_FILE/` to mount partition
int mnt(const char* path); ///mount partition specified on path
void HDDChecker();
void poweroffCallback(void *arg);
#else //this ensures that when HDD support is not available, loaded ELFs dont have any extra arg...
#define MPART NULL
#endif

#ifdef MX4SIO
int LookForBDMDevice(void);
#endif

#ifdef FILEXIO
#include <fileXio_rpc.h>
int LoadFIO(void); // Load FileXio and it´s dependencies
#endif

#endif