// Returns 0 on success, non-zero on error.
// It is normal for this to fail on consoles that have no DVD ROM chip (i.e. DEX or the SCPH-10000/SCPH-15000).
int DVDPlayerInit(void);

// Returns 0 on success, non-zero on error.
int DVDPlayerBoot(void);

// Returns a human-readable version number for the DVD Player.
const char *DVDPlayerGetVersion(void);
