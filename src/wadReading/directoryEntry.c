//
// Created by tjada on 30/01/2026.
//

#include "directoryEntry.h"

ltc_status readDirectoryEntry(FILE *wad, DirectoryEntry *outEntry, int *entryNum) {
    if (fread(&outEntry->lumpOffs, sizeof(int), 1, wad) != 1) {
        ltc_captureErrno(errno); return ltc_fail_io;
    }
    if (fread(&outEntry->lumpSize, sizeof(int), 1, wad) != 1) {
        ltc_captureErrno(errno); return ltc_fail_io;
    }
    if (fread(outEntry->lumpName, 1, 8, wad) != 8) {
        ltc_captureErrno(errno); return ltc_fail_io;
    }
    outEntry->entryNum = *entryNum;
    *entryNum += 1;
    return ltc_success;
}

void goToEntryByNum (FILE* wad, int dirOffset, int entryNum) {
    fseek(wad, dirOffset + (16*entryNum), SEEK_SET);
}

void skipEntries (FILE* wad, int skipCount, int* outEntryNum) {
    fseek(wad, 16 * skipCount, SEEK_CUR);
    *outEntryNum += skipCount;
}