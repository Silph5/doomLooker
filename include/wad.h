//
// Created by tjada on 28/01/2026.
//

#ifndef DOOMLOOKER_WAD_H
#define DOOMLOOKER_WAD_H
#include <stdio.h>
#include "directoryEntry.h"

#define MAX_TEXTUREX_EXPECTED 2

typedef struct {
    DirectoryEntry pnames;
    DirectoryEntry* textureXentries;
    int textureXcount;

} ExpectedUniqueLumpEntries;

typedef struct {
    int lumpCount;
    int dirOffset;
    ExpectedUniqueLumpEntries uniqueLumps;

    FILE* stream;
} Wad;

typedef struct {
    Wad* wadArr;
    int wadCount;
} WadTable;

#endif //DOOMLOOKER_WAD_H