#include <kernel.h>
#include <sifrpc.h>
#include <string.h>
#include <stdio.h>
#include <libcdvd.h>

#include "libcdvd_add.h"

static unsigned char MECHACON_CMD_S36_supported = 0, MECHACON_CMD_S27_supported = 0, MECHACON_CMD_S24_supported = 0;

// Initialize add-on functions. Currently only retrieves the MECHACON's version to determine what sceCdAltGetRegionParams() should do.
int cdInitAdd(void)
{
    int result, i;
    u32 status;
    u8 MECHA_version_data[3];
    unsigned int MECHA_version;

    // Like how CDVDMAN checks sceCdMV(), do not continuously attempt to get the MECHACON version because some consoles (e.g. DTL-H301xx) can't return one.
    for (i = 0; i <= 100; i++) {
        if ((result = sceCdMV(MECHA_version_data, &status)) != 0 && ((status & 0x80) == 0)) {
            MECHA_version = MECHA_version_data[2] | ((unsigned int)MECHA_version_data[1] << 8) | ((unsigned int)MECHA_version_data[0] << 16);
            MECHACON_CMD_S36_supported = (0x5FFFF < MECHA_version); // v6.0 and later
            MECHACON_CMD_S27_supported = (0x501FF < MECHA_version); // v5.2 and later
            MECHACON_CMD_S24_supported = (0x4FFFF < MECHA_version); // v5.0 and later
            return 0;
        }
    }

    DPRINTF("Failed to get MECHACON version: %d 0x%x\n", result, status);

    return -1;
}

/*
    This function provides an equivalent of the sceCdGetRegionParams function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
    It's supported by only slimline consoles, and returns regional information (e.g. MECHACON version, MG region mask, DVD player region letter etc.).
*/
int custom_sceCdReadRegionParams(u8 *data, u32 *stat)
{
    unsigned char RegionData[15];
    int result;

    memset(data, 0, 13);
    if (MECHACON_CMD_S36_supported) {
        // if ((result = sceCdApplySCmd(0x36, NULL, 0, RegionData, sizeof(RegionData))) != 0)
        if ((result = sceCdApplySCmd(0x36, NULL, 0, RegionData)) != 0) {
            *stat = RegionData[0];
            memcpy(data, &RegionData[1], 13);
        }
    } else {
        *stat = 0x100;
        result = 1;
    }

    return result;
}

// This function provides an equivalent of the sceCdBootCertify function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
int sceCdBootCertify(const u8 *data)
{
    int result;
    unsigned char CmdResult;

    // if ((result = sceCdApplySCmd(0x1A, data, 4, &CmdResult, 1)) != 0)
    if ((result = sceCdApplySCmd(0x1A, data, 4, &CmdResult)) != 0) {
        result = CmdResult;
    }

    return result;
}

int sceCdRM(char *ModelName, u32 *stat)
{
    unsigned char rdata[9];
    unsigned char sdata;
    int result1, result2;

    sdata = 0;
    result1 = sceCdApplySCmd(0x17, &sdata, 1, rdata);
    // result1 = sceCdApplySCmd(0x17, &sdata, 1, rdata, 9);

    *stat = rdata[0];
    memcpy(ModelName, &rdata[1], 8);

    sdata = 8;
    result2 = sceCdApplySCmd(0x17, &sdata, 1, rdata);
    // result2 = sceCdApplySCmd(0x17, &sdata, 1, rdata, 9);

    *stat |= rdata[0];
    memcpy(&ModelName[8], &rdata[1], 8);

    return ((result1 != 0 && result2 != 0) ? 1 : 0);
}

/*
    This function provides an equivalent of the sceCdReadPS1BootParam function from the newer CDVDMAN modules. The old CDVDFSV and CDVDMAN modules don't support this S-command.
    It's supported by only slimline consoles, and returns the boot path for the inserted PlayStation disc.
*/
int custom_sceCdReadPS1BootParam(char *param, u32 *stat)
{
    u8 out[16];
    int result;

    memset(param, 0, 11);
    if (MECHACON_CMD_S27_supported) {
        // if ((result = sceCdApplySCmd(0x27, NULL, 0, out, 13)) != 0)
        if ((result = sceCdApplySCmd(0x27, NULL, 0, out)) != 0) {
            *stat = out[0];
            memcpy(param, &out[1], 11); // Yes, one byte is not copied.
        }
    } else {
        *stat = 0x100;
        result = 1;
    }

    return result;
}

int sceCdRcBypassCtl(int bypass, u32 *stat)
{ // TODO: not implemented.
    u8 in[16], out[16];
    int result;

    memset(in, 0, 11);
    if (MECHACON_CMD_S24_supported) {
        // TODO
        // if ((result = sceCdApplySCmd(0x24, &bypass, 4, out, 13)) != 0)
        if ((result = sceCdApplySCmd(0x24, &bypass, 4, out)) != 0) {
            *stat = out[0];
        }
    } else {
        *stat = 0x100;
        result = 1;
    }

    return result;
}
