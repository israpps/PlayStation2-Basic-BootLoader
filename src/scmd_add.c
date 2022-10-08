/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
## (C) 2002 Nicholas Van Veen (nickvv@xtra.co.nz)
#     2003 loser (loser@internalreality.com)
# (c) 2004 Marcus R. Brown <mrbrown@0xd6.org> Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id$
# Function definitions for libsceCdvd (EE side calls to the iop module sceCdvdfsv).
#
# NOTE: These functions will work with the CDVDMAN/CDVDFSV or XCDVDMAN/XCDVDFSV
# modules stored in rom0.
#
# NOTE: not all functions work with each set of modules!
*/

#include <stdio.h>
#include <kernel.h>
#include <sifrpc.h>
#include <libcdvd.h>
#include "psx/plibcdvd_add.h"
#include <string.h>

enum PSX_CD_SCMD_CMDS
{
    CD_SCMD_CHG_SYS           = 0x2D, // PSX-only
    CD_SCMD_NOTICE_GAME_START = 0x2F  // PSX-only
};

extern int bindSCmd;
extern SifRpcClientData_t clientSCmd;
extern int sCmdSemaId;
extern u8 sCmdRecvBuff[];
extern u8 sCmdSendBuff[];
extern int sCmdNum;

int _CdCheckSCmd(int cmd);

// **** S-Command Functions ****

int custom_sceCdChgSys(int mode)
{
    int result;

    if (_CdCheckSCmd(CD_SCMD_CHG_SYS) == 0)
        return 0;

    *(int *)sCmdSendBuff = mode;
    if (SifCallRpc(&clientSCmd, CD_SCMD_CHG_SYS, 0, sCmdSendBuff, 4, sCmdRecvBuff, 4, NULL, NULL) >= 0)
    {
        result = *(int *)UNCACHED_SEG(sCmdRecvBuff);
    }
    else
    {
        result = 0;
    }

    SignalSema(sCmdSemaId);
    return result;
}

int custom_sceCdNoticeGameStart(int mode, u32 *result)
{
    int status;

    if (_CdCheckSCmd(CD_SCMD_NOTICE_GAME_START) == 0)
        return 0;

    *(u32 *)sCmdSendBuff = mode;
    if (SifCallRpc(&clientSCmd, CD_SCMD_NOTICE_GAME_START, 0, sCmdSendBuff, 4, sCmdRecvBuff, 8, NULL, NULL) >= 0)
    {
        *result = *(u32 *)UNCACHED_SEG(&sCmdRecvBuff[4]);
        status  = *(int *)UNCACHED_SEG(sCmdRecvBuff);
    }
    else
    {
        status = 0;
    }

    SignalSema(sCmdSemaId);
    return status;
}
