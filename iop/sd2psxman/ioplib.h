/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2009, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# taken from MX4SIO driver for simplicity.
# all credits go to maximus32 and qnox
*/

#ifndef IOPLIB_H
#define IOPLIB_H


#include <loadcore.h>


iop_library_t *ioplib_getByName(const char *name);
unsigned int ioplib_getTableSize(iop_library_t *lib);
void *ioplib_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func);
void ioplib_relinkExports(iop_library_t *lib);


#endif