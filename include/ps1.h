// Initialize the PlayStation driver in ROM. Returns 0 on success.
int PS1DRVInit(void);

// Boots the inserted PlayStation game disc.
int PS1DRVBoot(void);

// Returns a human-readable version number for the PlayStation driver.
const char *PS1DRVGetVersion(void);
