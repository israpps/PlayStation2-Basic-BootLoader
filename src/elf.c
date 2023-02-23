//--------------------------------------------------------------
// Borrowed from wLaunchELF
//--------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#include <elf-loader.h>
#include "debugprintf.h"
#define MAX_PATH 1025


void RunLoaderElf(char *filename, char *party)
{
    DPRINTF("\tLOADING [%s]\n", filename);
#ifndef NO_DPRINTF
    if (party != NULL)
        DPRINTF("\t%s: party is %s\n", __func__, party);
#endif
#ifdef SCR_PRINT
    sleep(5);
    DPRINTF(".\n");
#endif
    LoadELFFromFile(filename, 0, NULL);
}
