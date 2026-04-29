//
// Created by tjada on 28/01/2026.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtrychain.h>

#include "mapStruct.h"
#include "wad.h"
#include "DFread.h"
#include "textureBuilding.h"

ltc_status readWadHeader(FILE* wadStream, int* outLumpCount, int* outDirOffset) {
    //todo: PROPER ERROR CHECKING OF IO FUNCS

    fseek(wadStream, 0, SEEK_SET);
    char ident[4];
    fread(ident, sizeof(char), 4, wadStream);
    if (strncmp(ident, "IWAD", 4) != 0 && strncmp(ident, "PWAD", 4) != 0) {
        fprintf(stderr, "Detected an invalid wad\n");
        return -1;
    }

    fread(outLumpCount, sizeof(int), 1, wadStream);
    fread(outDirOffset, sizeof(int), 1, wadStream);

    return ltc_success;
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

ltc_status collectDirectoryEntries(wad* wad, int wadIndex, overrideEntries* mainEntries, const char* targetMapName) {

    fseek(wad->stream, wad->dirOffset, SEEK_SET);

    directoryEntry tempEntry;
    tempEntry.wadIndex = wadIndex;

    for (int entryNum = 0; entryNum < wad->lumpCount;) {
        LTC_TRY(readDirectoryEntry(wad->stream, &tempEntry, &entryNum), "failed to read directory entry");

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

    return ltc_success;
}

ltc_status determineMapFormat(wadTable* wads, directoryEntry mapMarkerEntry, mapFormat* outFormat) {
    wad entryWad = wads->wadArr[mapMarkerEntry.wadIndex];
    int entryNum = mapMarkerEntry.entryNum + 1;
    goToEntryByNum(entryWad.stream, entryWad.dirOffset, entryNum);

    directoryEntry tempEntry;
    readDirectoryEntry(entryWad.stream, &tempEntry, &entryNum);

    if (strncmp(tempEntry.lumpName, "THINGS", 6) == 0) {
        *outFormat = DOOMformat;
        return ltc_success;
    }
    if (strncmp(tempEntry.lumpName, "TEXTMAP", 7) == 0) {
        *outFormat = UDMF;
        return ltc_success;
    }
    return ltc_fail;
}

ltc_status initWad(const char* wadPath, wad* outWad) {
    LTC_TRY(ltc_fopen(&outWad->stream, wadPath, "rb"), "Failed to open wad file");
    LTC_TRY(ltc_malloc((void**)&outWad->uniqueLumps.textureXentries, sizeof(directoryEntry) * MAX_TEXTUREX_EXPECTED), "failed to malloc for textureX lumps");
    LTC_TRY(readWadHeader(outWad->stream, &outWad->lumpCount, &outWad->dirOffset), "failed to read wad header");

    return ltc_success;
}

ltc_status readWadsToDoomMapData (doomMap* map, char* mapName, char** wadPaths, const int wadCount) {

    wadTable wads;
    wads.wadCount = wadCount;
    wads.wadArr = calloc(wadCount,sizeof(wad));
    if (!wads.wadArr) {
        return ltc_fail_no_mem; //clang insists there's a leak here, i have no idea why
    }

    overrideEntries mainEntries = {0};
    for (int w = 0; w < wadCount; w++) {
        LTC_TRY(initWad(wadPaths[w], &wads.wadArr[w]), "failed to init a wad");
        LTC_TRY(collectDirectoryEntries(&wads.wadArr[w], w, &mainEntries, mapName), "failed to collect a wad's dir entries");
    }

    LTC_TRY(determineMapFormat(&wads, mainEntries.mapMarkerEntry, &mainEntries.mapFormat), "failed to determine map format");
    if (mainEntries.mapFormat == DOOMformat) {
        LTC_TRY(DFreadMap(map, &wads.wadArr[mainEntries.mapMarkerEntry.wadIndex], mainEntries.mapMarkerEntry), "failed to read a Doom Format map");
    } else if (mainEntries.mapFormat == UDMF) {
        return ltc_fail_not_supported;
    }

    LTC_TRY(getMapTextures(map, &mainEntries, &wads), "failed to get map textures");

    free(wads.wadArr);

    return ltc_success;
}

void freeDoomMapData(doomMap* map) {
    free(map->lineDefs);
    free(map->sectors);
    free(map->sideDefs);
    free(map->vertices);
    free(map->textures);

    free(map);
}