//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    directoryEntry mapMarkerEntry;
    directoryEntry lineDefsEntry;
    directoryEntry sideDefsEntry;
    directoryEntry verticesEntry;
    directoryEntry sectorsEntry;
} mapLumpsDirEntries;

void readHeader(FILE* wad, header* head) {
    fread(&head->ident, sizeof(char), 4, wad);
    head->ident[4] = '\0';

    fread(&head->lumpsNum, sizeof(int), 1, wad);
    fread(&head->directoryOffset, sizeof(int), 1, wad);
}

bool getTargetMapComposition(FILE* wad, mapLumpsDirEntries* mLumpsEntries, const int dirOffset, const int lumpsNum, const char* mapName) {
    fseek(wad, dirOffset, 0);

    directoryEntry tempMMarkerEntry;

    bool foundMap = false;

    for (int l = 0; l < lumpsNum; l++) {
        fread(&tempMMarkerEntry, sizeof(directoryEntry), 1, wad);

        if (strcmp(tempMMarkerEntry.lumpName, mapName) == 0) {
            mLumpsEntries->mapMarkerEntry = tempMMarkerEntry;
            foundMap = true;
            break;
        }
    }

    if (foundMap == false) {
        return false;
    }

    //the lumps in each map follow a set order, so I can seek through them.

    fseek(wad, 16, SEEK_CUR); //skips the Things lump

    fread(&mLumpsEntries->lineDefsEntry, sizeof(directoryEntry), 1, wad);
    fread(&mLumpsEntries->sideDefsEntry, sizeof(directoryEntry), 1, wad);
    fread(&mLumpsEntries->verticesEntry, sizeof(directoryEntry), 1, wad);

    fseek(wad, 48, SEEK_CUR); //skips the Segs, Ssectors and Nodes lump

    fread(&mLumpsEntries->sectorsEntry, sizeof(directoryEntry), 1, wad);

    return true;
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

    mapLumpsDirEntries mLumpsInfo;
    bool foundMap = getTargetMapComposition(wad, &mLumpsInfo, header.directoryOffset, header.lumpsNum, mapName);

    if (!foundMap) {
        fprintf(stderr, "Failed to locate map lump.\n");
        return NULL;
    }

    map->lineDefNum = mLumpsInfo.lineDefsEntry.lumpSize / LINEDEF_SIZE_BYTES;
    map->sideDefNum = mLumpsInfo.sideDefsEntry.lumpSize / SIDEDEF_SIZE_BYTES;
    map->vertexNum = mLumpsInfo.verticesEntry.lumpSize / VERTEX_SIZE_BYTES;
    map->sectorNum = mLumpsInfo.sectorsEntry.lumpSize / SECTOR_SIZE_BYTES;

    map->lineDefs = malloc(sizeof(lineDef) * map->lineDefNum);
    for (int lineNum = 0; lineNum < map->lineDefNum; lineNum++) {
        readLineDef(wad, &map->lineDefs[lineNum], mLumpsInfo.lineDefsEntry.lumpOffs + (lineNum * 14));
        printf("lineDef data:\n");
        printf("v1: %u\n", map->lineDefs[lineNum].v1);
        printf("v2: %u\n", map->lineDefs[lineNum].v2);
        printf("frontSideNum: %u\n", map->lineDefs[lineNum].frontSideNum);
        printf("backSideNum: %u\n\n", map->lineDefs[lineNum].backSideNum);
    }

    map->sideDefs = malloc(sizeof(sideDef) * map->sideDefNum);
    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        readSideDef(wad, &map->sideDefs[sideNum], mLumpsInfo.sideDefsEntry.lumpOffs + (sideNum * 30));
        printf("sideDef data:\n");
        printf("xTexOffset: %u\n", map->sideDefs[sideNum].xTexOffset);
        printf("yTexOffset: %u\n", map->sideDefs[sideNum].yTexOffset);
        printf("upperTexName:%.8s\n", map->sideDefs[sideNum].upperTexName);
        printf("lowerTexName:%.8s\n", map->sideDefs[sideNum].lowerTexName);
        printf("midTexName:%.8s\n", map->sideDefs[sideNum].midTexName);
        printf("sectFacing:%u\n\n", map->sideDefs[sideNum].sectFacing);
    }

    map->vertices = malloc(sizeof(vertex) * map->sideDefNum);
    for (int vertNum = 0; vertNum < map->vertexNum; vertNum++) {
        readVertex(wad, &map->vertices[vertNum], mLumpsInfo.verticesEntry.lumpOffs + (vertNum * 4));
        printf("vertex data:\n");
        printf("x: %u\n", map->vertices[vertNum].x);
        printf("y: %u\n\n", map->vertices[vertNum].y);
    }

    map->sectors = malloc(sizeof(sector) * map->sectorNum);
    for (int sectNum = 0; sectNum < map->sectorNum; sectNum++) {
        readSector(wad, &map->sectors[sectNum], mLumpsInfo.sectorsEntry.lumpOffs + (sectNum * 26));
        printf("sector data:\n");
        printf("floorHeight: %u\n", map->sectors[sectNum].floorHeight);
        printf("ceilHeight: %u\n", map->sectors[sectNum].ceilHeight);
        printf("floorTex: %.8s\n", map->sectors[sectNum].floorTex);
        printf("ceilTex: %.8s\n", map->sectors[sectNum].ceilTex);
        printf("brightness: %u\n\n", map->sectors[sectNum].brightness);
    }

    return map;
}