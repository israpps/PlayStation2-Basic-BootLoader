#ifndef BANNER_H
#define BANNER_H


const char* BANNER =
// change the banner text depending on system type, leave versioning and credits the same
#ifdef PSX
    "\t\t__________  _____________  _______________________.____"
    "\t\t\\______   \\/   _____/\\   \\/  /\\______   \\______   \\    |"
    "\t\t |     ___/\\_____  \\  \\     /  |    |  _/|    |  _/    |"
    "\t\t |    |    /        \\ /     \\  |    |   \\|    |   \\    |___"
    "\t\t |____|   /_______  //___/\\  \\ |______  /|______  /_______ \\"
    "\t\t                  \\/       \\_/        \\/        \\/        \\/"
#else
    "\t\t__________  _________________   ____________________.____\n"
    "\t\t\\______   \\/   _____/\\_____  \\  \\______   \\______   \\    |\n"
    "\t\t |     ___/\\_____  \\  /  ____/   |    |  _/|    |  _/    |\n"
    "\t\t |    |    /        \\/       \\   |    |   \\|    |   \\    |___\n"
    "\t\t |____|   /_______  /\\_______ \\  |______  /|______  /_______ \\\n"
    "\t\t                  \\/         \\/         \\/        \\/        \\/\n"
#endif
    "\t\t\tv" VERSION "-" SUBVERSION "-" PATCHLEVEL " - " STATUS "\n"
    "\n";
#define BANNER_FOOTER \
    "\t\t		PlayStation2 Basic BootLoader - By Matias Israelson\n" \
    "\t\t                                             (AKA: El_isra)\n" \
    "\t\tGet it Free on: github.com/israpps/PlayStation2-Basic-BootLoader\n"

#endif
