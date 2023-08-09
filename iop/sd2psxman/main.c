#include "main.h"
#include "module_debug.h"

IRX_ID(MODNAME, MAJOR, MINOR);

SecrSetMcCommandHandler_hook_t ORIGINAL_SecrSetMcCommandHandler = NULL;
static McCommandHandler_t McCommandHandler = NULL;

void HOOKED_SecrSetMcCommandHandler(McCommandHandler_t handler)
{
    DPRINTF("%s: handler ptr 0x%p\n",__FUNCTION__, handler);
    McCommandHandler = handler;
    ORIGINAL_SecrSetMcCommandHandler(handler); //we kept the original function to call it from here... else, SECRMAN wont auth cards...
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
    return MODULE_RESIDENT_END;
}

int __stop(int argc, char *argv[])
{
    DPRINTF("Unloading module\n");
    return MODULE_NO_RESIDENT_END;
}