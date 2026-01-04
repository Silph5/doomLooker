//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_hash/uthash.h"
#include "mapStruct.h"
#include "mapComponentStructs.h"

//sizes of map component entries in DOOM wad format:
#define LINEDEF_SIZE_BYTES 14
#define SIDEDEF_SIZE_BYTES 30
#define VERTEX_SIZE_BYTES 4
#define SECTOR_SIZE_BYTES 26

typedef struct {
    char ident[5];
    int lumpsNum;
    int directoryOffset;
} header;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8];
} directoryEntry;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8]; //key

    UT_hash_handle hh;
} directoryEntryHashed;

typedef struct {
    directoryEntry mapMarkerEntry;
    directoryEntry lineDefsEntry;
    directoryEntry sideDefsEntry;
    directoryEntry verticesEntry;
    directoryEntry sectorsEntry;
} mapLumps;

typedef struct {
    mapLumps targMapLumps;
    directoryEntry pnames;
    directoryEntry textureDefs1;
    directoryEntry textureDefs2; //texture2's existence is a pain and only really matters if you are using doom 1 as an iwad

    directoryEntryHashed* patches;
} reqWadLumps;

void readHeader(FILE* wad, header* head) {
    fread(&head->ident, sizeof(char), 4, wad);
    head->ident[4] = '\0';

    fread(&head->lumpsNum, sizeof(int), 1, wad);
    fread(&head->directoryOffset, sizeof(int), 1, wad);
}

void getTargetMapComposition(FILE* wad, mapLumps* mLumpsEntries, int* entryNum) {

    //the lumps in each map follow a set order, so I can seek through them.

    fseek(wad, 16, SEEK_CUR); //skips the Things lump

    fread(&mLumpsEntries->lineDefsEntry, sizeof(directoryEntry), 1, wad);
    fread(&mLumpsEntries->sideDefsEntry, sizeof(directoryEntry), 1, wad);
    fread(&mLumpsEntries->verticesEntry, sizeof(directoryEntry), 1, wad);

    fseek(wad, 48, SEEK_CUR); //skips the Segs, Ssectors and Nodes lump

    fread(&mLumpsEntries->sectorsEntry, sizeof(directoryEntry), 1, wad);

    *entryNum += 8;

}

void collectPatchEntries(FILE* wad, directoryEntryHashed** patchTable, int* entryNum) {

    directoryEntry tempEntry;

    while (fread(&tempEntry, sizeof(directoryEntry), 1, wad) == 1) {

        *entryNum += 1;

        if (strncmp(tempEntry.lumpName, "P_END", 5) == 0) {
            break;
        }

        if (tempEntry.lumpSize == 0) {
            continue;
        }

        directoryEntryHashed* newPatchEntry = malloc(sizeof(*newPatchEntry));

        newPatchEntry->lumpOffs = tempEntry.lumpOffs;
        newPatchEntry->lumpSize = tempEntry.lumpSize;
        memcpy(newPatchEntry->lumpName, tempEntry.lumpName, 8);

        HASH_ADD(hh, *patchTable, lumpName, 8, newPatchEntry);

    }
}

int getRequiredLumpEntries(FILE* wad, reqWadLumps* wadLumps, const header* header, const char* targetMap) {
    fseek(wad, header->directoryOffset, 0);

    directoryEntry tempEntry;

    wadLumps->patches = NULL;

    for (int entryNum = 0; entryNum < header->lumpsNum; entryNum++) {
        fread(&tempEntry, sizeof(directoryEntry), 1, wad);

        if (strncmp(tempEntry.lumpName, "TEXTURE1", 8) == 0) {
            wadLumps->textureDefs1 = tempEntry;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "TEXTURE2", 8) == 0) {
            wadLumps->textureDefs2 = tempEntry;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "P_START", 7) == 0) {
            collectPatchEntries(wad, &wadLumps->patches, &entryNum);
            continue;
        }
        if (strncmp(tempEntry.lumpName, "PNAMES", 6) == 0) {
            wadLumps->pnames = tempEntry;
            continue;
        }
        if (strncmp(tempEntry.lumpName, targetMap, 5) == 0) {
            getTargetMapComposition(wad, &wadLumps->targMapLumps, &entryNum);
            continue;
        }
    }

    return 0;
}

void readLineDef (FILE* wad, lineDef* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->v1, sizeof(uint16_t), 1, wad);
    fread(&targetStruct->v2, sizeof(uint16_t), 1, wad);

    fseek(wad, 6, SEEK_CUR); //skip flags, type, sector tag

    fread(&targetStruct->frontSideNum, sizeof(uint16_t), 1, wad);
    fread(&targetStruct->backSideNum, sizeof(uint16_t), 1, wad);
}

void readSideDef (FILE* wad, sideDef* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->xTexOffset, sizeof(uint16_t), 1, wad);
    fread(&targetStruct->yTexOffset, sizeof(uint16_t), 1, wad);
    fread(&targetStruct->upperTexName, sizeof(char), 8, wad);
    fread(&targetStruct->lowerTexName, sizeof(char), 8, wad);
    fread(&targetStruct->midTexName, sizeof(char), 8, wad);
    fread(&targetStruct->sectFacing, sizeof(uint16_t), 1, wad);
}

void readVertex (FILE* wad, vertex* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->x, sizeof(int16_t), 1, wad);
    fread(&targetStruct->y, sizeof(int16_t), 1, wad);
}

void readSector (FILE* wad, sector* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->floorHeight, sizeof(int16_t), 1, wad);
    fread(&targetStruct->ceilHeight, sizeof(int16_t), 1, wad);
    fread(&targetStruct->floorTex, sizeof(char), 8, wad);
    fread(&targetStruct->ceilTex, sizeof(char), 8, wad);
    fread(&targetStruct->brightness, sizeof(int16_t), 1, wad);
}

void readMapGeometry (FILE* wad, doomMap* map, mapLumps* mLumpsInfo) {

    map->lineDefNum = mLumpsInfo->lineDefsEntry.lumpSize / LINEDEF_SIZE_BYTES;
    map->sideDefNum = mLumpsInfo->sideDefsEntry.lumpSize / SIDEDEF_SIZE_BYTES;
    map->vertexNum = mLumpsInfo->verticesEntry.lumpSize / VERTEX_SIZE_BYTES;
    map->sectorNum = mLumpsInfo->sectorsEntry.lumpSize / SECTOR_SIZE_BYTES;

    map->lineDefs = malloc(sizeof(lineDef) * map->lineDefNum);
    for (int lineNum = 0; lineNum < map->lineDefNum; lineNum++) {
        readLineDef(wad, &map->lineDefs[lineNum], mLumpsInfo->lineDefsEntry.lumpOffs + (lineNum * 14));
    }

    map->sideDefs = malloc(sizeof(sideDef) * map->sideDefNum);
    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        readSideDef(wad, &map->sideDefs[sideNum], mLumpsInfo->sideDefsEntry.lumpOffs + (sideNum * 30));
    }

    map->vertices = malloc(sizeof(vertex) * map->sideDefNum);
    for (int vertNum = 0; vertNum < map->vertexNum; vertNum++) {
        readVertex(wad, &map->vertices[vertNum], mLumpsInfo->verticesEntry.lumpOffs + (vertNum * 4));
    }

    map->sectors = malloc(sizeof(sector) * map->sectorNum);
    for (int sectNum = 0; sectNum < map->sectorNum; sectNum++) {
        readSector(wad, &map->sectors[sectNum], mLumpsInfo->sectorsEntry.lumpOffs + (sectNum * 26));
    }

}

doomMap* readWadToMapData(const char* wadPath, const char* mapName) {

    doomMap* map = malloc(sizeof(doomMap));

    if (!map) {
        fprintf(stderr, "failed malloc");
        return NULL;
    }

    FILE* wad = fopen(wadPath, "rb");
    if (!wad) {
        free(map);
        return NULL;
    }

    header header;
    readHeader(wad, &header);
    if (strcmp(header.ident, "IWAD") != 0 && strcmp(header.ident, "PWAD") != 0) {
        fprintf(stderr, "Incorrect file type given.\nfile ident:%s\n", header.ident);
        free(map);
        return NULL;
    }

    reqWadLumps reqLumpEntries;
    getRequiredLumpEntries(wad, &reqLumpEntries, &header, mapName);

    readMapGeometry(wad, map, &reqLumpEntries.targMapLumps);

    return map;
}