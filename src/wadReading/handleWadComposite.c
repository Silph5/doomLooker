//
// Created by tjada on 28/01/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapStruct.h"
#include "wad.h"
#include "directoryEntry.h"

typedef struct {
    wad* wadArr;
    int wadCount;
} wadTable;

int readWadHeader(FILE* wadStream, int* outLumpCount, int* outDirOffset) {
    fseek(wadStream, 0, SEEK_SET);
    char ident[4];
    fread(ident, sizeof(char), 4, wadStream);
    if (strncmp(ident, "IWAD", 4) != 0 && strncmp(ident, "PWAD", 4) != 0) {
        fprintf(stderr, "Detected an invalid wad\n");
        return -1;
    }

    fread(outLumpCount, sizeof(int), 1, wadStream);
    fread(outDirOffset, sizeof(int), 1, wadStream);

    return 0;
}

int initWad(const char* wadPath, wad* outWad) {
    outWad->stream = fopen(wadPath, "rb");
    if (!outWad->stream) {
        fprintf(stderr, "Failed to open a wad\n");
        return -1;
    }

    readWadHeader(outWad->stream, &outWad->lumpCount, &outWad->dirOffset);

    outWad->format = DOOMformat; //it is assumed for now that all wads are in the 1990s doom format
    return 0;

}

doomMap* readWadsToDoomMapData (const char** wadPaths, const int wadCount) {

    wadTable wads;
    wads.wadCount = wadCount;
    wads.wadArr = malloc(sizeof(wad) * wadCount);
    if (!wads.wadArr) {
        return NULL; //intellisense insists there's a leak here, i have no idea why
    }

    doomMap* map = malloc(sizeof(doomMap));
    if (!map) {
        free(wads.wadArr);
        return NULL;
    }

    for (int w = 0; w < wadCount; w++) {
        initWad(wadPaths[w], &wads.wadArr[w]);
        if (wads.wadArr[w].format == DOOMformat) {

            continue;
        }
        if (wads.wadArr[w].format == UDMF) {

            continue;
        }
    }

    free(wads.wadArr);
    return map;
}