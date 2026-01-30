//
// Created by tjada on 30/01/2026.
//

#include "directoryEntry.h"

void readDirectoryEntry(FILE* wad, directoryEntry* outEntry) {
    fread(&outEntry->lumpOffs, sizeof(int), 1, wad);
    fread(&outEntry->lumpSize, sizeof(int), 1, wad);
    fread(outEntry->lumpName, 1, 8, wad);
}