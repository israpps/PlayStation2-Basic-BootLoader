#ifndef SD2PSXMAN_MAIN_H
#define SD2PSXMAN_MAIN_H
#include "ioplib.h"
#include "irx_imports.h"

#define SecrSetMcCommandHandler_Expnum 0x04
typedef int (*McCommandHandler_t)(int port, int slot, sio2_transfer_data_t *sio2_trans_data); // to hook into secrman setter
typedef void (*SecrSetMcCommandHandler_hook_t)(McCommandHandler_t handler);

#endif