#include <tamtypes.h>
/**
 * @brief Custom implementation of sceCdChgSys, changes the PSX DESR disc reader mode
 * @param mode operation mode (2 means PS2 mode, any other value is unknown/untested)
 * @returns tray mode after change.
 * @note for checking success the return value must match the parameter you pass to it
 * @warning it needs PCDVDMAN and PCDVDFS to function
*/
int custom_sceCdChgSys(int mode);
#define sceDVRTrayModePS2 2 // Invented this name

/**
 * @brief Signals the start of a game, so that the user can use the "quit" game button.
 * @param mode unknown. use `1`
 * @param result when function succeeds 8th bit is set, aka: (result & 0x80) returns true
 * @return cero on success
 * @note both return value and result parameter must be considered for checking the success, evaluate both with OR
 * @warning it needs PCDVDMAN and PCDVDFS to function
*/
int custom_sceCdNoticeGameStart(int mode, u32 *result);
