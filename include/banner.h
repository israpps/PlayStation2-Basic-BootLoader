#ifndef BANNER_H
#define BANNER_H


const char* BANNER =
// change the banner text depending on system type, leave versioning and credits the same
#ifdef PSX
    "\t\t__________  _____________  _______________________.____\n"
    "\t\t\\______   \\/   _____/\\   \\/  /\\______   \\______   \\    |\n"
    "\t\t |     ___/\\_____  \\  \\     /  |    |  _/|    |  _/    |\n"
    "\t\t |    |    /        \\ /     \\  |    |   \\|    |   \\    |___\n"
    "\t\t |____|   /_______  //___/\\  \\ |______  /|______  /_______ \\\n"
    "\t\t                  \\/       \\_/        \\/        \\/        \\/\n"
#elif COH
"\t\t  ______________.___. _________________           ________\n"
"\t\t /   _____/\\__  |   |/   _____/\\_____  \\ ___  ___/  _____/\n"
"\t\t \\_____  \\  /   |   |\\_____  \\  /  ____/ \\  \\/  /   __  \\ \n"
"\t\t /        \\ \\____   |/        \\/       \\  >    <\\  |__\\  \\\n"
"\t\t/_______  / / ______/_______  /\\_______ \\/__/\\_ \\\\_____  /\n"
"\t\t        \\/  \\/              \\/         \\/      \\/      \\/ \n"
#else
    "\t\t__________  _________________   ____________________.____\n"
    "\t\t\\______   \\/   _____/\\_____  \\  \\______   \\______   \\    |\n"
    "\t\t |     ___/\\_____  \\  /  ____/   |    |  _/|    |  _/    |\n"
    "\t\t |    |    /        \\/       \\   |    |   \\|    |   \\    |___\n"
    "\t\t |____|   /_______  /\\_______ \\  |______  /|______  /_______ \\\n"
    "\t\t                  \\/         \\/         \\/        \\/        \\/\n"
#endif
    "\t\t\tv" VERSION "-" SUBVERSION "-" PATCHLEVEL " - " STATUS 
#ifdef DEBUG
" - DEBUG"
#endif
#ifdef COH
" - COH-H"
#endif
    "\n"
    "\n";
#define BANNER_FOOTER \
    "\t\t		PlayStation2 Basic BootLoader - By Matias Israelson\n" \
    "\t\t                                             (AKA: El_isra)\n" \
    "\t\tGet it Free on: github.com/israpps/PlayStation2-Basic-BootLoader\n"

#endif
