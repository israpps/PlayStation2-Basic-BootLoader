#include <errno.h>
#include <fcntl.h>
#include <kernel.h>
#include <libmc.h>
#include <libcdvd.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include "OSDInit.h"
#include "OSDHistory.h"
#include <stdlib.h>

/*  The OSDs have this weird bug whereby the size of the icon file is hardcoded to 1776 bytes... even though that is way too long!
    Unfortunately, using the right size will cause the icon to be deemed as corrupted data by the HDDOSD. */
#define SYSDATA_ICON_SYS_SIZE 1776

#define NUM_TABLES       1024
#define HISTORY_PATH_LEN 1024

extern unsigned char icon_sys_A[];
extern unsigned char icon_sys_J[];
extern unsigned char icon_sys_C[];
#ifdef WRITE_ICOBYSYS
extern unsigned char icobysys_icn[];
extern unsigned int size_icobysys_icn;
#endif

/* Original used to maintain the time in global variables. These were updated by another thread. */
/* extern int currentYear;
extern int currentMonth;
extern int currentDate;
extern int currentHour;
extern int currentSecond;
extern int currentMinute; */

static int HasTooManyHistoryRecords;
static struct HistoryEntry HistoryEntries[MAX_HISTORY_ENTRIES], OldHistoryEntry;

static int WriteHistoryFile(int port, const char *path, const void *buffer, int len, int append)
{
    int fd, result, size;

    if (append) {
        sceMcOpen(port, 0, path, O_RDWR);
        sceMcSync(0, NULL, &fd);
    }

    if ((!append) || (fd < 0)) // If not opened in append mode or if the file could not be opened.
        sceMcOpen(port, 0, path, O_CREAT | O_WRONLY);

    sceMcSync(0, NULL, &fd);

    if (fd >= 0) {
        if (append) { // Get length of file
            sceMcSeek(fd, 0, SEEK_END);
            sceMcSync(0, NULL, &size);

            if (size >= 0) { // Seek to the last position within the file.
                sceMcSeek(fd, size, SEEK_SET);
                sceMcSync(0, NULL, &size);
            }

            if (size < 0) { // Seek error
                result = sceMcClose(fd);
                sceMcSync(0, NULL, &result);
                return size;
            }
        }

        sceMcWrite(fd, buffer, len);
        sceMcSync(0, NULL, &size);

        if (size >= 0) {
            sceMcClose(fd);
            sceMcSync(0, NULL, &result);

            return (sceMcResSucceed <= result ? 0 : result);
        } else {
            result = sceMcClose(fd);
            sceMcSync(0, NULL, &result);

            return size;
        }
    } else
        return fd;
}

static int McCheckFileExists(int port, const char *path)
{
    int fd, result;

    if (sceMcOpen(port, 0, path, O_RDONLY) < sceMcResSucceed)
        return -1;

    sceMcSync(0, NULL, &fd);

    if (fd >= 0) {
        sceMcClose(fd);
        sceMcSync(0, NULL, &result);
        return (sceMcResSucceed <= result ? 1 : result);
    } else
        return 0;
}

int LoadHistoryFile(int port)
{
    char fullpath[64];
    int fd, result, result2, type, format;

    sceMcGetInfo(port, 0, &type, NULL, &format);
    sceMcSync(0, NULL, &result);

    if (result < sceMcResChangedCard || type != sceMcTypePS2 || format == 0)
        return -1;

    sprintf(fullpath, "%s/%s", OSDGetHistoryDataFolder(), "history");
    fd = sceMcOpen(port, 0, fullpath, O_RDONLY);
    sceMcSync(0, NULL, &fd);

    if (fd >= 0) {
        sceMcRead(fd, HistoryEntries, MAX_HISTORY_ENTRIES * sizeof(struct HistoryEntry));
        sceMcSync(0, NULL, &result);

        // Original had duplicated this block.
        sceMcClose(fd);
        sceMcSync(0, NULL, &result2);

        return ((result >= 0) ? (result2 >= 0 ? 0 : -1) : result);
    } else
        result = fd;

    return result;
}

int SaveHistoryFile(int port)
{
    /// TODO: Rewrite me please!
}

static u16 GetTimestamp(void)
{
    // The original obtained the time and date from globals.
    // return OSD_HISTORY_SET_DATE(currentYear, currentMonth, currentDate);
    sceCdCLOCK time;
    sceCdReadClock(&time);
    return OSD_HISTORY_SET_DATE(btoi(time.year), btoi(time.month & 0x7F), btoi(time.day));
}

static void AddHistoryRecord(const char *name)
{
    int i, value, LeastUsedRecord, LeastUsedRecordLaunchCount, LeastUsedRecordTimestamp, NewLaunchCount;
    u8 BlankSlotList[MAX_HISTORY_ENTRIES];
    int IsNewRecord;

    LeastUsedRecord = 0;
    LeastUsedRecordTimestamp = INT_MAX;
    LeastUsedRecordLaunchCount = INT_MAX;
    HasTooManyHistoryRecords = 0;
    IsNewRecord = 1;

    for (i = 0; i < MAX_HISTORY_ENTRIES; i++) {
        if (HistoryEntries[i].LaunchCount < LeastUsedRecordLaunchCount) {
            LeastUsedRecord = i;
            LeastUsedRecordLaunchCount = HistoryEntries[i].LaunchCount;
        }
        if (LeastUsedRecordLaunchCount == HistoryEntries[i].LaunchCount) {
            if (HistoryEntries[i].DateStamp < LeastUsedRecordTimestamp) {
                LeastUsedRecordTimestamp = HistoryEntries[i].DateStamp;
                LeastUsedRecord = i;
            }
        }

        // In v1.0x, this was strcmp
        if (!strncmp(HistoryEntries[i].name, name, sizeof(HistoryEntries[i].name))) {
            IsNewRecord = 0;

            HistoryEntries[i].DateStamp = GetTimestamp();
            if ((HistoryEntries[i].bitmask & 0x3F) != 0x3F) {
                NewLaunchCount = HistoryEntries[i].LaunchCount + 1;
                if (NewLaunchCount >= 0x80)
                    NewLaunchCount = 0x7F;

                if (NewLaunchCount >= 14) {
                    if ((NewLaunchCount - 14) % 10 == 0) {
                        while ((HistoryEntries[i].bitmask >> (value = rand() % 6)) & 1) {};
                        HistoryEntries[i].ShiftAmount = value;
                        HistoryEntries[i].bitmask |= 1 << value;
                    }
                }

                HistoryEntries[i].LaunchCount = NewLaunchCount;
            } else {
                if (HistoryEntries[i].LaunchCount < 0x3F) { // Was a check against 0x40 in v1.0x
                    HistoryEntries[i].LaunchCount++;
                } else {
                    HistoryEntries[i].LaunchCount = HistoryEntries[i].bitmask & 0x3F;
                    HistoryEntries[i].ShiftAmount = 7;
                }
            }
        }
    }

    /* i = 0;
    do
    { // Original does this. I guess, it is used to ensure that the next random value is truly random?
        rand();
        i++;
    } while (i < (currentMinute * 60 + currentSecond)); */

    if (IsNewRecord) {
        // Count and consolidate a list of blank slots.
        int NumBlankSlots = 0, NumSlotsUsed = 0;
        for (i = 0; i < MAX_HISTORY_ENTRIES; i++) {
            if (HistoryEntries[i].name[0] == '\0') {
                BlankSlotList[NumBlankSlots] = i;
                NumBlankSlots++;
            } else { // Not present in v1.0x.
                if (HistoryEntries[i].ShiftAmount == 0x7)
                    NumSlotsUsed++;
            }
        }

        if (NumSlotsUsed != MAX_HISTORY_ENTRIES) {
            struct HistoryEntry *NewEntry;
            if (NumBlankSlots > 0) { // Randomly choose an empty slot.
                NewEntry = &HistoryEntries[BlankSlotList[rand() % NumBlankSlots]];
            } else { // Copy out the victim record
                NewEntry = &HistoryEntries[LeastUsedRecord];
                memcpy(&OldHistoryEntry, NewEntry, sizeof(OldHistoryEntry));
                HasTooManyHistoryRecords = 1;
            }

            // Initialize the new entry.
            strncpy(NewEntry->name, name, sizeof(NewEntry->name));
            NewEntry->LaunchCount = 1;
            NewEntry->bitmask = 1;
            NewEntry->ShiftAmount = 0;
            NewEntry->DateStamp = GetTimestamp();
        }
    }
}

void UpdatePlayHistory(const char *name)
{
    // Try to load the history file from memory card slot 1
    if (LoadHistoryFile(0) < 0) { // Try memory card slot 2
        if (LoadHistoryFile(1) < 0)
            memset(HistoryEntries, 0, sizeof(HistoryEntries));
    }

    AddHistoryRecord(name);
    if (SaveHistoryFile(0) < 0) {
        SaveHistoryFile(1);
    }
}
