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
    char *argv[1];
    argv[0] = filename;
    DPRINTF("\tLOADING [%s]\n", filename);
#ifndef NO_DPRINTF
    if (party != NULL)
        DPRINTF("\tparty is %s\n", __func__, party);
#endif
#ifdef SCR_PRINT
    sleep(5);
    DPRINTF(".\n");
#endif
    LoadELFFromFile(filename, 1, argv);
}
