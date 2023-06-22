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


void RunLoaderElf(const char *filename, const char *party)
{
    DPRINTF("\tLOADING [%s]\n", filename);
#ifndef NO_DPRINTF
    if (party != NULL)
        DPRINTF("%s\tparty is %s\n", __func__, party);
#endif
#ifdef SCR_PRINT
    sleep(5);
    DPRINTF(".\n");
#endif
    if (party == NULL)
	{
		DPRINTF("LoadELFFromFile(%s, 0, NULL)\n", filename);
        LoadELFFromFile(filename, 0, NULL);
	}
    else
	{
		DPRINTF("LoadELFFromFileWithPartition(%s, %s, 0, NULL);\n", filename, party);
        LoadELFFromFileWithPartition(filename, party, 0, NULL);
	}
}
