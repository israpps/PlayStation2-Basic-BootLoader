
/**
 * @brief Initialize the PlayStation driver in ROM
 * @returns 0 on success.
*/
int PS1DRVInit(void);

/**
 * @brief Boots the inserted PlayStation game disc.
*/
int PS1DRVBoot(void);


/**
 * @brief gets the version number for the PlayStation driver.
 * @returns a human-readable version number for the PlayStation driver.
*/
const char *PS1DRVGetVersion(void);
