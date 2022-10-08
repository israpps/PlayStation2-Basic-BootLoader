#define MAX_HISTORY_ENTRIES                     21

/*  If the record is valid, the launch count will be >0.

    Starts with:
        SLPM_550.52, data: 01 01 00 00 75 1a
    After 2 launches:
        SLPM_550.52, data: 02 01 00 00 75 1a
    After 15 launches:
        SLPM_550.52, data: 0e 11 04 00 75 1a
    Clock advanced 1 day:
        SLPM_550.52, data: 01 01 00 00 76 1a
    Clock advanced 1 year:
        SLPM_550.52, data: 01 01 00 00 75 1c
    Clock set to Jan 1 2013:
        SLPM_550.52, data: 01 01 00 00 21 1a

    data[0] = LaunchCount
    data[1] = ?? (A bitmask)
    data[2] = ?? (The last shift amount used for generating the value of data[1])
    data[3] = ?? (Seems to not be initialized)
    data[4-5] = Timestamp in format: YYYYYYYMMMMDDDDD
    Note: Year is since year 2000.
*/

#define OSD_HISTORY_GET_YEAR(datestamp)         ((datestamp) >> 9 & 0x7F)
#define OSD_HISTORY_GET_MONTH(datestamp)        ((datestamp) >> 5 & 0xF)
#define OSD_HISTORY_GET_DATE(datestamp)         ((datestamp)&0x1F)
#define OSD_HISTORY_SET_DATE(year, month, date) (((u16)(year)) << 9 | ((u16)(month)&0xF) << 5 | ((date)&0x1F))

struct HistoryEntry
{
    char name[16];
    u8 LaunchCount;
    u8 bitmask;
    u8 ShiftAmount;
    u8 padding;
    u16 DateStamp;
};

void UpdatePlayHistory(const char *name);

// Low-level functions. Use them for writing your own functions (i.e. writing your own boot animation).
int LoadHistoryFile(int port);
int SaveHistoryFile(int port);
