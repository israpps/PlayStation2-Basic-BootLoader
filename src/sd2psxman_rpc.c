
#include <stdbool.h>
#include <tamtypes.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include "../iop/sd2psxman/rpc_common.h"
#include "debugprintf.h"


static SifRpcClientData_t sd2psxman_RPC;
static int rpc_initialized = false;

#define CHECK_RPC_INIT() if (!rpc_initialized) {DPRINTF("ERROR: Cannot call %s if RPC server is not initialized\n", __FUNCTION__); return -2;}

int sd2psxman_init(void)
{
    if (rpc_initialized)
        {return -2;}
    
    int E;
	while(1)
	{
		if((E = SifBindRpc(&sd2psxman_RPC, SD2PSXMAN_IRX, 0)) < 0)
        {
            DPRINTF("Failed to bind RPC server for SD2PSXMAN (%d)\n", E);
			return -1;
        }

		if(sd2psxman_RPC.server != NULL)
			break;

		nopdelay();
	}


	rpc_initialized = true;

	return 0;
}

int sd2psx_umount_bootcard(int port, int slot)
{
    CHECK_RPC_INIT();
    if (SifCallRpc(&sd2psxman_RPC, SD2PSX_QUIT_BOOTCARD, 0, NULL, 0, NULL, 0, NULL, NULL) < 0)
    {
        DPRINTF("%s: RPC ERROR\n", __FUNCTION__);
        return -1;
    }
    return 0;

}

int sd2psx_switch_card_slot(int port, int slot, int VMCslot)
{
    sd2psxman_rpc_data_t PKT;
    CHECK_RPC_INIT();
    PKT.cardID = VMCslot;
    if (SifCallRpc(&sd2psxman_RPC, SD2PSX_SWITCH_CARD_SLOT, 0, &PKT, SD2PSXMAN_PKT_SIZE, NULL, 0, NULL, NULL) < 0)
    {
        DPRINTF("%s: RPC ERROR\n", __FUNCTION__);
        return -1;
    }
    return 0;

}

int sd2psx_switch_card_channel(int port, int slot, int channel)
{
    sd2psxman_rpc_data_t PKT;
    CHECK_RPC_INIT();
    PKT.channelID = channel;
    if (SifCallRpc(&sd2psxman_RPC, SD2PSX_SWITCH_CARD_CHANNEL, 0, &PKT, SD2PSXMAN_PKT_SIZE, NULL, 0, NULL, NULL) < 0)
    {
        DPRINTF("%s: RPC ERROR\n", __FUNCTION__);
        return -1;
    }
    return 0;

}

int sd2psx_switch_card_full(int port, int slot, int VMCslot, int channel)
{
    sd2psxman_rpc_data_t PKT;
    CHECK_RPC_INIT();
    PKT.cardID = VMCslot;
    PKT.channelID = channel;
    if (SifCallRpc(&sd2psxman_RPC, SD2PSX_SWITCH_CARD_FULL, 0, &PKT, SD2PSXMAN_PKT_SIZE, NULL, 0, NULL, NULL) < 0)
    {
        DPRINTF("%s: RPC ERROR\n", __FUNCTION__);
        return -1;
    }
    return 0;

}

int sd2psx_switch_card_by_gameID(int port, int slot, const char* ID)
{
    sd2psxman_rpc_data_t PKT;
    CHECK_RPC_INIT();
    strncpy(PKT.GAMEID, ID, sizeof(PKT.GAMEID));
    if (SifCallRpc(&sd2psxman_RPC, SD2PSX_SWITCH_CARD_SLOT_BY_GAMEID, 0, &PKT, SD2PSXMAN_PKT_SIZE, NULL, 0, NULL, NULL) < 0)
    {
        DPRINTF("%s: RPC ERROR\n", __FUNCTION__);
        return -1;
    }
    return 0;

}