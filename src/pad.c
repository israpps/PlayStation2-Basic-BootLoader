#include <kernel.h>
#include <libpad.h>
#include <stdio.h>
#include <string.h>
#include <tamtypes.h>

#include "debugprintf.h"
#include "pad.h"

static unsigned char padArea[2][256] ALIGNED(64);
static unsigned int old_pad[2] = {0, 0};
int pad_inited = 0;

void PadInitPads(void)
{
    DPRINTF("%s: padinit\n", __FUNCTION__);
    padInit(0);
    DPRINTF("%s: padPortOpen(0, 0, padArea[0])\n", __FUNCTION__);
    padPortOpen(0, 0, padArea[0]);
    DPRINTF("%s: padPortOpen(1, 0, padArea[1])\n", __FUNCTION__);
    padPortOpen(1, 0, padArea[1]);

    old_pad[0] = 0;
    old_pad[1] = 0;
    pad_inited = 1;
}

void PadDeinitPads(void)
{
    if (pad_inited) {
        padPortClose(0, 0);
        padPortClose(1, 0);
        padEnd();
        pad_inited = 0;
    }
}

int ReadPadStatus_raw(int port, int slot)
{
    struct padButtonStatus buttons;
    u32 paddata;

    paddata = 0;
    if (padRead(port, slot, &buttons) != 0) {
        paddata = 0xffff ^ buttons.btns;
    }

    return paddata;
}

int ReadCombinedPadStatus_raw(void)
{
    return (ReadPadStatus_raw(0, 0) | ReadPadStatus_raw(1, 0));
}

int ReadPadStatus(int port, int slot)
{
    struct padButtonStatus buttons;
    u32 new_pad, paddata;

    new_pad = 0;
    if (padRead(port, slot, &buttons) != 0) {
        paddata = 0xffff ^ buttons.btns;

        new_pad = paddata & ~old_pad[port];
        old_pad[port] = paddata;
    }

    return new_pad;
}

int ReadCombinedPadStatus(void)
{
    return (ReadPadStatus(0, 0) | ReadPadStatus(1, 0));
}