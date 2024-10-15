#ifdef F_DVDPLAYER
/** @brief initialize DVDPlayer
 *
 * @returns 0 on success, non-zero on error.
 * @warning It is normal for this to fail on consoles that have no DVD ROM chip (i.e. DEX or the SCPH-10000/SCPH-15000).
*/
int DVDPlayerInit(void);


/**
 * @returns 0 on success, non-zero on error.
*/
int DVDPlayerBoot(void);

/** @brief get dvdplayer version
 * @returns a human-readable version number for the DVD Player.
 */
const char *DVDPlayerGetVersion(void);

#define GDVDPLAYER(x...) x//DVDPlayer Guard
#else
#define GDVDPLAYER(x...)//DVDPlayer Guard
#endif
