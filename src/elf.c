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
	DPRINTF("LoadELFFromFileWithPartition(%s, %s, 0, NULL);\n", filename, party);
    LoadELFFromFileWithPartition(filename, party, 0, NULL);
}
