#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF

#ifdef EE_SIO
    #include "EEsio.h"
    #include <sio.h>
    #define DPRINTF_INIT() sio_init(38400, 0, 0, 0, 0);
    #define DPRINTF(x...) sio_printf(x)
#elif PCSX2
    #define DPRINTF(x...) printf(x)
#elif SCR_PRINT
    #include <debug.h>
    #define DPRINTF(x...) scr_printf("\t"x)
#else
    #define DPRINTF(x...)
    #define NO_DPRINTF
#endif

#ifndef DPRINTF_INIT
#define DPRINTF_INIT()
#endif

#endif //DEBUG_PRINTF