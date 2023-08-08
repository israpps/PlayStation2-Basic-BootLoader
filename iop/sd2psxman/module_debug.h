#ifndef SD2PSXMAN_DBG
#define SD2PSXMAN_DBG

#ifdef DEBUG
#define DPRINTF(fmt, x...) printf(MODNAME": "fmt, ##x)
#else
#define DPRINTF(x...) 
#endif

#define MODNAME "sd2psx_manager"
#define MAJOR 1
#define MINOR 0

#endif
