//
// Created by tjada on 28/01/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapStruct.h"
#include "wad.h"

typedef struct {
    wad* wads;
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
    wads.wads = malloc(sizeof(wad) * wadCount);
    doomMap* map = malloc(sizeof(doomMap));

    for (int w = 0; w < wadCount; w++) {
        initWad(wadPaths[w], &wads.wads[w]);
    }

    free(wads.wads);
    return map;
}