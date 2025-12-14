#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <tamtypes.h>
#include <kernel.h>
#include <elf-loader.h>
#include "debugprintf.h"
#define MAX_PATH 1025
#ifdef DEBUG
#define DBGWAIT(T) sleep(T)
#else
#define DBGWAIT(T)
#endif


void RunLoaderElf(const char *filename, const char *party)
{
    DPRINTF("%s\n", __FUNCTION__);
    if (party == NULL || strlen(party) == 0) {
        DPRINTF("LoadELFFromFile(%s, 0, NULL)\n", filename);
        DBGWAIT(2);
        LoadELFFromFile(filename, 0, NULL);
    } else {
        DPRINTF("LoadELFFromFileWithPartition(%s, %s, 0, NULL);\n", filename, party);
        DBGWAIT(2);
        LoadELFFromFileWithPartition(filename, party, 0, NULL);
    }
}
