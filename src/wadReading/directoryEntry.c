//
// Created by tjada on 30/01/2026.
//

#include "directoryEntry.h"

void readDirectoryEntry(FILE* wad, directoryEntry* outEntry, int* entryNum) {
    fread(&outEntry->lumpOffs, sizeof(int), 1, wad);
    fread(&outEntry->lumpSize, sizeof(int), 1, wad);
    fread(outEntry->lumpName, 1, 8, wad);
    outEntry->entryNum = *entryNum;
    *entryNum += 1;
}

void goToEntryByNum (FILE* wad, int dirOffset, int entryNum) {
    fseek(wad, dirOffset + (16*entryNum), SEEK_SET);
}

void skipEntries (FILE* wad, int skipCount, int* outEntryNum) {
    fseek(wad, 16 * skipCount, SEEK_CUR);
    *outEntryNum += skipCount;
}