#include "main.h"
#include "rpc_common.h"
#include "module_debug.h"

IRX_ID(MODNAME, MAJOR, MINOR);

SecrSetMcCommandHandler_hook_t ORIGINAL_SecrSetMcCommandHandler = NULL;
static McCommandHandler_t McCommandHandler = NULL;

static SifRpcDataQueue_t sd2psxman_queue;
static SifRpcServerData_t sd2psxman_server;
static u8 _rpc_buffer[512 * 4] __attribute__((__aligned__(4))); // TODO: ADJUST RPC BUFFER SIZE
static int RPCThreadID;

void HOOKED_SecrSetMcCommandHandler(McCommandHandler_t handler)
{
    DPRINTF("%s: handler ptr 0x%p\n",__FUNCTION__, handler);
    McCommandHandler = handler;
    ORIGINAL_SecrSetMcCommandHandler(handler); //we kept the original function to call it from here... else, SECRMAN wont auth cards...
}

static void * rpcHandlerFunction(unsigned int CMD, void * rpcBuffer, int size)
{
    printf("CMD %d\n", CMD);
	switch(CMD)
	{
    case SD2PSX_QUIT_BOOTCARD:
        break;
    case SD2PSX_SWITCH_CARD_SLOT:
        break;
    case SD2PSX_SWITCH_CARD_CHANNEL:
        break;
    case SD2PSX_SWITCH_CARD_SLOT_BY_GAMEID:
        break;
    case SD2PSX_SEND_CMD:
        break;
	default:
		printf(MODNAME": Unknown CMD (%d) called!\n", CMD);

  }

  return rpcBuffer;
}

static void threadRpcFunction(void *arg)
{
	(void)arg;

	DPRINTF("RPC Thread Started\n");

	SifSetRpcQueue( &sd2psxman_queue , GetThreadId() );
	SifRegisterRpc( &sd2psxman_server, SD2PSXMAN_IRX, (void *)rpcHandlerFunction,(u8 *)&_rpc_buffer,NULL,NULL, &sd2psxman_queue );
	SifRpcLoop( &sd2psxman_queue );
}


int __stop(int argc, char *argv[]);
int __start(int argc, char *argv[]);

int _start(int argc, char *argv[])
{
    if (argc >= 0) 
        return __start(argc, argv);
    else
        return __stop(argc, argv);
}

int __start(int argc, char *argv[])
{
    printf("SD2PSX Manager v%d.%d by El_isra\n", MAJOR, MINOR);

    if (ioplib_getByName("mcman") != NULL)
    {
        DPRINTF("MCMAN FOUND. Must be loaded after this module to intercept the McCommandHandler\n");
        return MODULE_NO_RESIDENT_END;
    }
    iop_library_t * SECRMAN = ioplib_getByName("secrman");
    if (SECRMAN == NULL)
    {
        DPRINTF("SECRMAN not found\n");
        return MODULE_NO_RESIDENT_END;
    }
    DPRINTF("Found SECRMAN. version 0x%X\n", SECRMAN->version);

    ORIGINAL_SecrSetMcCommandHandler = (SecrSetMcCommandHandler_hook_t)ioplib_hookExportEntry(SECRMAN, SecrSetMcCommandHandler_Expnum, HOOKED_SecrSetMcCommandHandler);
    if (ORIGINAL_SecrSetMcCommandHandler == NULL)
    {
        DPRINTF("Error hooking into SecrSetMcCommandHandler\n");
        return MODULE_NO_RESIDENT_END;
    } else {
        DPRINTF("Hooked SecrSetMcCommandHandler (ptr:0x%p)\n", HOOKED_SecrSetMcCommandHandler);
    }

    iop_library_t * lib_modload = ioplib_getByName("modload");
    if (lib_modload != NULL) {
        DPRINTF("modload 0x%x detected\n", lib_modload->version);
        if (lib_modload->version > 0x102) //IOP is running a MODLOAD version wich supports unloading IRX Modules
            return MODULE_REMOVABLE_END; // and we do support getting unloaded...
    } else {
        DPRINTF("modload not detected!\n");
    }

	iop_thread_t	T;

	DPRINTF("Creating RPC thread.\n");

	T.attr = TH_C;
	T.option = 0;
	T.thread = &threadRpcFunction;
	T.stacksize = 0x800;
	T.priority = 0x1e;

	RPCThreadID = CreateThread(&T);
	if (RPCThreadID < 0)
	{
		DPRINTF("CreateThread failed. (%i)\n", RPCThreadID );
		return MODULE_NO_RESIDENT_END;
	}
	else
	{
#ifdef DEBUG
		int TSTAT = 
#endif
        StartThread(RPCThreadID, NULL);
        DPRINTF("Thread started (%d)\n", TSTAT);
	}


    return MODULE_RESIDENT_END;
}

int __stop(int argc, char *argv[])
{
    DPRINTF("Unloading module\n");
    DeleteThread(RPCThreadID);
    return MODULE_NO_RESIDENT_END;
}