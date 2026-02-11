//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#include "mapStruct.h"
#include "mapComponentStructs.h"
#include "directoryEntry.h"
#include "wad.h"

#define LINEDEF_SIZE_BYTES 14
#define SIDEDEF_SIZE_BYTES 30
#define VERTEX_SIZE_BYTES 4
#define SECTOR_SIZE_BYTES 26

typedef struct {
    directoryEntry lineDefsEntry;
    directoryEntry sideDefsEntry;
    directoryEntry verticesEntry;
    directoryEntry sectorsEntry;
} mapLumpEntries;

void normaliseTexName(char* name) {
    for (int c = 0; c < 8; c++) {
        if (name[c] == '\0') {
            break;
        }
        name[c] = toupper((unsigned char)name[c]);
    }
}

void getTargetMapComposition(wad* wad, directoryEntry markerEntry, mapLumpEntries* mLumpEntries) {

    int entryNum = markerEntry.entryNum;
    goToEntryByNum(wad->stream, wad->dirOffset, markerEntry.entryNum+1); //go to things
    //the lumps in each map follow a set order THINGS, LINEDEFS, SIDEDEFS, VERTEXES, SEGS, SSECTORS, NODES, SECTORS, REJECT, BLOCKMAP

    skipEntries(wad->stream, 1, &entryNum); //skips the Things lump

    readDirectoryEntry(wad->stream, &mLumpEntries->lineDefsEntry, &entryNum);
    readDirectoryEntry(wad->stream, &mLumpEntries->sideDefsEntry, &entryNum);
    readDirectoryEntry(wad->stream, &mLumpEntries->verticesEntry, &entryNum);

    skipEntries(wad->stream, 3, &entryNum); //skips the Segs, Ssectors and Nodes lump

    readDirectoryEntry(wad->stream, &mLumpEntries->sectorsEntry, &entryNum);

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

    fread(&targetStruct->xTexOffset, sizeof(int16_t), 1, wad);
    fread(&targetStruct->yTexOffset, sizeof(int16_t), 1, wad);
    fread(&targetStruct->upperTexName, sizeof(char), 8, wad);
    normaliseTexName(targetStruct->upperTexName);
    fread(&targetStruct->lowerTexName, sizeof(char), 8, wad);
    normaliseTexName(targetStruct->lowerTexName);
    fread(&targetStruct->midTexName, sizeof(char), 8, wad);
    normaliseTexName(targetStruct->midTexName);
    fread(&targetStruct->sectFacing, sizeof(uint16_t), 1, wad);
}

void readVertex (FILE* wad, vertex* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->x, sizeof(int16_t), 1, wad);
    fread(&targetStruct->y, sizeof(int16_t), 1, wad);
    targetStruct->y *= -1; //the map model ends up mirrored without this
}

void readSector (FILE* wad, sector* targetStruct, int offs) {
    fseek(wad, offs, 0);

    fread(&targetStruct->floorHeight, sizeof(int16_t), 1, wad);
    fread(&targetStruct->ceilHeight, sizeof(int16_t), 1, wad);
    fread(&targetStruct->floorTex, sizeof(char), 8, wad);
    fread(&targetStruct->ceilTex, sizeof(char), 8, wad);
    fread(&targetStruct->brightness, sizeof(int16_t), 1, wad);
}

int readMapGeometry (FILE* wad, doomMap* map, mapLumpEntries* mLumpsInfo) {

    map->lineDefNum = mLumpsInfo->lineDefsEntry.lumpSize / LINEDEF_SIZE_BYTES;
    map->sideDefNum = mLumpsInfo->sideDefsEntry.lumpSize / SIDEDEF_SIZE_BYTES;
    map->vertexNum = mLumpsInfo->verticesEntry.lumpSize / VERTEX_SIZE_BYTES;
    map->sectorNum = mLumpsInfo->sectorsEntry.lumpSize / SECTOR_SIZE_BYTES;

    map->lineDefs = malloc(sizeof(lineDef) * map->lineDefNum);
    if (map->lineDefs == NULL) {fprintf(stderr, "Failed to allocate memory for linedefs"); return 0;}
    for (int lineNum = 0; lineNum < map->lineDefNum; lineNum++) {
        readLineDef(wad, &map->lineDefs[lineNum], mLumpsInfo->lineDefsEntry.lumpOffs + (lineNum * LINEDEF_SIZE_BYTES));
    }

    map->sideDefs = malloc(sizeof(sideDef) * map->sideDefNum);
    if (map->sideDefs == NULL) {fprintf(stderr, "Failed to allocate memory for sidedefs"); return 0;}
    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        readSideDef(wad, &map->sideDefs[sideNum], mLumpsInfo->sideDefsEntry.lumpOffs + (sideNum * SIDEDEF_SIZE_BYTES));
    }

    map->vertices = malloc(sizeof(vertex) * map->vertexNum);
    if (map->vertices == NULL) {fprintf(stderr, "Failed to allocate memory for vertices"); return 0;}
    for (int vertNum = 0; vertNum < map->vertexNum; vertNum++) {
        readVertex(wad, &map->vertices[vertNum], mLumpsInfo->verticesEntry.lumpOffs + (vertNum * VERTEX_SIZE_BYTES));
    }

    map->sectors = malloc(sizeof(sector) * map->sectorNum);
    if (map->sectors == NULL) {fprintf(stderr, "Failed to allocate memory for sectors"); return 0;}
    for (int sectNum = 0; sectNum < map->sectorNum; sectNum++) {
        readSector(wad, &map->sectors[sectNum], mLumpsInfo->sectorsEntry.lumpOffs + (sectNum * SECTOR_SIZE_BYTES));
    }

    return 1;
}

int DFreadMap(doomMap* map, wad* wad, directoryEntry mapMarkerEntry) {

    mapLumpEntries mLumpEntries;
    getTargetMapComposition(wad, mapMarkerEntry, &mLumpEntries);
    readMapGeometry(wad->stream, map, &mLumpEntries);

    return 0;
}



