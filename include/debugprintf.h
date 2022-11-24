#ifndef DEBUG_PRINTF
#define DEBUG_PRINTF

#define DEBUG
#define SCR_PRINT

#ifdef DEBUG

#ifdef EE_SIO
    #define DPRINTF(x...) sio_printf(x)
#endif

#ifdef PCSX2
    #define DPRINTF(x...) printf(x)
#endif

#ifdef SCR_PRINT
    #include <debug.h>
    #define DPRINTF(x...) scr_printf("\t"x)
#endif

#else
    #define DPRINTF() do {} while (0)
#endif //DEBUG

#endif //DEBUG_PRINTF