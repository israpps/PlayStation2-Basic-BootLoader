#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF

#ifdef EE_SIO
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

#endif //DEBUG_PRINTF