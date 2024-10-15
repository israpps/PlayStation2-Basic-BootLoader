#ifdef F_PS2
/**
 * @brief  Boots the inserted PlayStation 2 game disc
 * @param skip_PS2LOGO wheter to load the game main executable via rom0:PS2LOGO or run it directly
 * @returns 0 on success.
*/
int PS2DiscBoot(int skip_PS2LOGO);

/**
 * @brief Function that reboots the browser with the "BootError" argument.
 * @note You can use this if an unexpected error occurs while booting the software that the user wants to use.
*/
void BootError(void);
#define GPS2DISC(x...) x
#else
#define GPS2DISC(x...)
#endif
