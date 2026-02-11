//
// Created by tjada on 28/01/2026.
//

#ifndef DOOMLOOKER_WAD_H
#define DOOMLOOKER_WAD_H
#include <stdio.h>
#include "directoryEntry.h"

#define MAX_TEXTUREX_EXPECTED 2

typedef struct {
    directoryEntry pnames;
    directoryEntry* textureXentries;
    int textureXcount;

} expectedUniqueLumpEntries;

typedef struct {
    int lumpCount;
    int dirOffset;
    expectedUniqueLumpEntries uniqueLumps;

    FILE* stream;
} wad;

typedef struct {
    wad* wadArr;
    int wadCount;
} wadTable;

#endif //DOOMLOOKER_WAD_H