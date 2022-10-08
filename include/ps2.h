// Boots the inserted PlayStation 2 game disc. Returns 0 on success.
int PS2DiscBoot(int skip_PS2LOGO);

// Function that reboots the browser with the "BootError" argument.
// You can use this if an unexpected error occurs while booting the software that the user wants to use.
void BootError(void);
