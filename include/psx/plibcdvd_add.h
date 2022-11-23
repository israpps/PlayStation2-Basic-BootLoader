#include <tamtypes.h>
/**
 * @brief Custom implementation of sceCdChgSys, changes the PSX DESR disc reader mode
 * @param mode operation mode (2 means PS2 mode, any other value is unknown/untested)
 * @returns 1: sucess, 0: failure
 * @warning it needs PCDVDMAN and PCDVDFS to function
*/
int custom_sceCdChgSys(int mode);

/**
 * @brief Signals the start of a game, so that the user can use the "quit" game button.
 * @param mode unknown
 * @param result when function succeeds 8th bit is set, aka: (result & 0x80) returns true
 * @return cero on success
 * @warning it needs PCDVDMAN and PCDVDFS to function
*/
int custom_sceCdNoticeGameStart(int mode, u32 *result);
