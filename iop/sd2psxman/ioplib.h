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

/** @brief get library object of any IRX Module
 * @param name name of the module to search for
 * @return NULL on error.
 */
iop_library_t *ioplib_getByName(const char *name);
unsigned int ioplib_getTableSize(iop_library_t *lib);


/** @brief hook an IRX Module exported function
 * @param lib Library object of the module to hook
 * @param entry Export number to be hooked
 * @param func Hook function
 * @return on error: NULL | on success: original function pointer
 */
void *ioplib_hookExportEntry(iop_library_t *lib, unsigned int entry, void *func);

void ioplib_relinkExports(iop_library_t *lib);


#endif