//
// Created by tjada on 28/01/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapStruct.h"
#include "wad.h"
#include "DFread.h"
#include "textureBuilding.h"

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

int readEntriesBetweenMarkersToHashTable (wad* wad, directoryEntryHashed** entryHashTable, const char* endMarker, const int eMcharCount, int wadIndex, int* outEntryNum) {

    directoryEntry tempEntry;

    do {
        readDirectoryEntry(wad->stream, &tempEntry, outEntryNum);

        if (strncmp(tempEntry.lumpName, endMarker, eMcharCount) == 0) {
            break;
        }

        if (tempEntry.lumpSize == 0) {
            continue;
        }

        directoryEntryHashed* newHashEntry = malloc(sizeof(directoryEntryHashed));

        newHashEntry->lumpOffs = tempEntry.lumpOffs;
        newHashEntry->lumpSize = tempEntry.lumpSize;
        newHashEntry->wadIndex = wadIndex;
        newHashEntry->entryNum = *outEntryNum;

        memcpy(newHashEntry->lumpName, tempEntry.lumpName, 8);

        HASH_ADD(hh, *entryHashTable, lumpName, 8, newHashEntry);
        *outEntryNum += 1;

    } while (strncmp(tempEntry.lumpName, endMarker, eMcharCount) != 0);

    return 0;
}

int collectDirectoryEntries(wad* wad, int wadIndex, overrideEntries* mainEntries, const char* targetMapName) {

    fseek(wad->stream, wad->dirOffset, SEEK_SET);

    directoryEntry tempEntry;
    tempEntry.wadIndex = wadIndex;

    for (int entryNum = 0; entryNum < wad->lumpCount;) {
        readDirectoryEntry(wad->stream, &tempEntry, &entryNum);

        if (strncmp(tempEntry.lumpName, "PLAYPAL", 7) == 0) {
            mainEntries->playPal = tempEntry;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "TEXTURE1", 8) == 0 || strncmp(tempEntry.lumpName, "TEXTURE2", 8) == 0) {
            wad->uniqueLumps.textureXentries[wad->uniqueLumps.textureXcount] = tempEntry;
            wad->uniqueLumps.textureXcount += 1;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "P_START", 7) == 0) {
            readEntriesBetweenMarkersToHashTable(wad, &mainEntries->patches, "P_END", 5, wadIndex, &entryNum);
            continue;
        }
        if (strncmp(tempEntry.lumpName, "PP_START", 7) == 0) {
            readEntriesBetweenMarkersToHashTable(wad, &mainEntries->patches, "PP_END", 5, wadIndex, &entryNum);
            continue;
        }
        if (strncmp(tempEntry.lumpName, "PNAMES", 6) == 0) {
            wad->uniqueLumps.pnames = tempEntry;
            continue;
        }
        if (strncmp(tempEntry.lumpName, targetMapName, 5) == 0) {
            mainEntries->mapMarkerEntry = tempEntry;
        }
    }

    return 0;
}

int determineMapFormat(wadTable* wads, directoryEntry mapMarkerEntry, mapFormat* outFormat) {
    wad entryWad = wads->wadArr[mapMarkerEntry.wadIndex];
    int entryNum = mapMarkerEntry.entryNum + 1;
    goToEntryByNum(entryWad.stream, entryWad.dirOffset, entryNum);

    directoryEntry tempEntry;
    readDirectoryEntry(entryWad.stream, &tempEntry, &entryNum);

    if (strncmp(tempEntry.lumpName, "THINGS", 6) == 0) {
        *outFormat = DOOMformat;
        return 0;
    }
    if (strncmp(tempEntry.lumpName, "TEXTMAP", 7) == 0) {
        *outFormat = UDMF;
        return 0;
    }
    return -1;
}

int initWad(const char* wadPath, wad* outWad) {
    outWad->stream = fopen(wadPath, "rb");
    if (!outWad->stream) {
        fprintf(stderr, "Failed to open a wad %s\n", wadPath);
        return -1;
    }

    outWad->uniqueLumps.textureXentries = malloc(sizeof(directoryEntry) * MAX_TEXTUREX_EXPECTED);
    if (!outWad->uniqueLumps.textureXentries) {
        fprintf(stderr, "failed to malloc for textureX lumps");
        return -1;
    }

    readWadHeader(outWad->stream, &outWad->lumpCount, &outWad->dirOffset);

    return 0;
}

doomMap* readWadsToDoomMapData (const char* mapName, char** wadPaths, const int wadCount) {

    wadTable wads;
    wads.wadCount = wadCount;
    wads.wadArr = calloc(wadCount,sizeof(wad));
    if (!wads.wadArr) {
        return NULL; //clang insists there's a leak here, i have no idea why
    }

    doomMap* map = malloc(sizeof(doomMap));
    if (!map) {
        free(wads.wadArr);
        return NULL;
    }

    overrideEntries mainEntries = {0};
    for (int w = 0; w < wadCount; w++) {
        initWad(wadPaths[w], &wads.wadArr[w]);
        collectDirectoryEntries(&wads.wadArr[w], w, &mainEntries, mapName);
    }

    determineMapFormat(&wads, mainEntries.mapMarkerEntry, &mainEntries.mapFormat);
    if (mainEntries.mapFormat == DOOMformat) {
        DFreadMap(map, &wads.wadArr[mainEntries.mapMarkerEntry.wadIndex], mainEntries.mapMarkerEntry);
    } else if (mainEntries.mapFormat == UDMF) {
        printf("UDMF is not currently supported");
        return NULL;
    }

    getMapTextures(map, &mainEntries, &wads);

    free(wads.wadArr);
    return map;
}

void freeDoomMapData(doomMap* map) {
    free(map->lineDefs);
    free(map->sectors);
    free(map->sideDefs);
    free(map->vertices);
    free(map->textures);

    free(map);
}