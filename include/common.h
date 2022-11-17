#ifndef COMMONDEF
#define COMMONDEF
enum {SOURCE_INVALID = 0, SOURCE_MC0, SOURCE_MC1, SOURCE_MASS} CONFIG_SOURCES_ID;

#define MAX_LEN 64
#define CNF_LEN_MAX 20480 // 20kb should be enough for massive CNF's
#define DEFDELAY 5000
enum {AUTO, SELECT, L3, R3, START, UP, RIGHT, DOWN, LEFT, L2, R2, L1, R1, TRIANGLE, CIRCLE, CROSS, SQUARE};
const char* KEYS_ID[17] = {
    "AUTO",
    "SELECT",    // 0x0001
    "L3",        // 0x0002
    "R3",        // 0x0004
    "START",     // 0x0008
    "UP",        // 0x0010
    "RIGHT",     // 0x0020
    "DOWN",      // 0x0040
    "LEFT",      // 0x0080
    "L2",        // 0x0100
    "R2",        // 0x0200
    "L1",        // 0x0400
    "R1",        // 0x0800
    "TRIANGLE",  // 0x1000
    "CIRCLE",    // 0x2000
    "CROSS",     // 0x4000
    "SQUARE"     // 0x8000
};

char* DEFPATH[] = {
    "mc?:/BOOT/BOOT.ELF",  // AUTO [0]
    "mc?:/APPS/ULE/ELF",
    "mass:/PS2BBL/BOOT.ELF",
    "mass:/PS2BBL/L2[1].ELF",  // L2 [3]
    "mass:/PS2BBL/L2[2].ELF",
    "mass:/PS2BBL/L2[3].ELF",
    "mass:/PS2BBL/R2[1].ELF",  // R2 [6]
    "mass:/PS2BBL/R2[2].ELF",
    "mass:/PS2BBL/R2[3].ELF",
    "mc?:/OPL/OPNPS2LD.ELF",  // L1 [9]
    "mc?:/APPS/OPNPS2LD/ELF",
    "mass:/PS2BBL/OPNPS2LD.ELF",
    "mass:/RESCUE.ELF",  // R1 [12]
    "mc?:/BOOT/BOOT2.ELF",
    "mc?:/APPS/ULE.ELF",
};

#ifndef COMMIT_HASH
#define COMMIT_HASH "UNKNOWN"
#endif

#endif //COMMONDEF
