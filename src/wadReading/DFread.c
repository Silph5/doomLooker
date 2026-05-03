//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdalign.h>

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

ltc_status getTargetMapComposition(wad* wad, directoryEntry markerEntry, mapLumpEntries* mLumpEntries) {

    int entryNum = markerEntry.entryNum;
    goToEntryByNum(wad->stream, wad->dirOffset, markerEntry.entryNum+1); //go to things
    //the lumps in each map follow a set order THINGS, LINEDEFS, SIDEDEFS, VERTEXES, SEGS, SSECTORS, NODES, SECTORS, REJECT, BLOCKMAP

    skipEntries(wad->stream, 1, &entryNum); //skips the Things lump

    LTC_TRY(readDirectoryEntry(wad->stream, &mLumpEntries->lineDefsEntry, &entryNum), "failed to read linedefs entry");
    LTC_TRY(readDirectoryEntry(wad->stream, &mLumpEntries->sideDefsEntry, &entryNum), "failed to read sidedefs entry");
    LTC_TRY(readDirectoryEntry(wad->stream, &mLumpEntries->verticesEntry, &entryNum), "failed to read verts entry");

    skipEntries(wad->stream, 3, &entryNum); //skips the Segs, Ssectors and Nodes lump

    LTC_TRY(readDirectoryEntry(wad->stream, &mLumpEntries->sectorsEntry, &entryNum), "failed to read sectors entry");

    return ltc_success;
}

//todo: error checking for below 4 funcs

ltc_status readLineDef(FILE *wad, lineDef *targetStruct, int offs) {
    fseek(wad, offs, 0);

    if (fread(&targetStruct->v1, sizeof(uint16_t), 1, wad) != 1) {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->v2, sizeof(uint16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}

    fseek(wad, 6, SEEK_CUR); //skip flags, type, sector tag

    if (fread(&targetStruct->frontSideNum, sizeof(uint16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->backSideNum, sizeof(uint16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}

    return ltc_success;
}

ltc_status readSideDef (FILE* wad, sideDef* targetStruct, int offs) {
    fseek(wad, offs, 0);

    if (fread(&targetStruct->xTexOffset, sizeof(int16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->yTexOffset, sizeof(int16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->upperTexName, sizeof(char), 8, wad) != 8)  {ltc_captureErrno(errno); return ltc_fail_io;}
    normaliseTexName(targetStruct->upperTexName);
    if (fread(&targetStruct->lowerTexName, sizeof(char), 8, wad) != 8)  {ltc_captureErrno(errno); return ltc_fail_io;}
    normaliseTexName(targetStruct->lowerTexName);
    if (fread(&targetStruct->midTexName, sizeof(char), 8, wad) != 8)  {ltc_captureErrno(errno); return ltc_fail_io;}
    normaliseTexName(targetStruct->midTexName);
    if (fread(&targetStruct->sectFacing, sizeof(uint16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}

    return ltc_success;
}

ltc_status readVertex (FILE* wad, vertex* targetStruct, int offs) {
    fseek(wad, offs, 0);

    if (fread(&targetStruct->x, sizeof(int16_t), 1, wad) != 1) {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->y, sizeof(int16_t), 1, wad) != 1) {ltc_captureErrno(errno); return ltc_fail_io;}
    targetStruct->y *= -1; //the map model ends up mirrored without this

    return ltc_success;
}

ltc_status readSector (FILE* wad, sector* targetStruct, int offs) {
    fseek(wad, offs, 0);

    if (fread(&targetStruct->floorHeight, sizeof(int16_t), 1, wad) != 1) {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->ceilHeight, sizeof(int16_t), 1, wad) != 1) {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->floorTex, sizeof(char), 8, wad) != 8)  {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->ceilTex, sizeof(char), 8, wad) != 8) {ltc_captureErrno(errno); return ltc_fail_io;}
    if (fread(&targetStruct->brightness, sizeof(int16_t), 1, wad) != 1)  {ltc_captureErrno(errno); return ltc_fail_io;}

    return ltc_success;
}

int readMapGeometry (FILE* wad, doomMap* map, mapLumpEntries* mLumpsInfo) {

    map->lineDefNum = mLumpsInfo->lineDefsEntry.lumpSize / LINEDEF_SIZE_BYTES;
    map->sideDefNum = mLumpsInfo->sideDefsEntry.lumpSize / SIDEDEF_SIZE_BYTES;
    map->vertexNum = mLumpsInfo->verticesEntry.lumpSize / VERTEX_SIZE_BYTES;
    map->sectorNum = mLumpsInfo->sectorsEntry.lumpSize / SECTOR_SIZE_BYTES;

    size_t offs = 0;
    size_t linedefsOffs = offs;
    offs += map->lineDefNum * sizeof(lineDef);

    offs = (offs + alignof(lineDef) - 1) & ~(alignof(lineDef) - 1);
    const size_t sidedefsOffs = offs;
    offs += map->sideDefNum * sizeof(sideDef);

    offs = (offs + alignof(vertex) - 1) & ~(alignof(vertex) - 1);
    const size_t vertsOffs = offs;
    offs += map->vertexNum * sizeof(vertex);

    offs = (offs + alignof(sector) - 1) & ~(alignof(sector) - 1);
    const size_t sectorsOffs = offs;
    offs += map->sectorNum * sizeof(sector);

    void* mapDataBlock = NULL;
    LTC_TRY(ltc_malloc(&mapDataBlock, offs), "Failed to malloc for map data block");

    map->lineDefs = (lineDef*)(mapDataBlock + linedefsOffs);
    map->sideDefs = (sideDef*)(mapDataBlock + sidedefsOffs);
    map->vertices = (vertex*)(mapDataBlock + vertsOffs);
    map->sectors = (sector*)(mapDataBlock + sectorsOffs);

    for (int lineNum = 0; lineNum < map->lineDefNum; lineNum++) {
        LTC_TRY(readLineDef(wad, &map->lineDefs[lineNum], mLumpsInfo->lineDefsEntry.lumpOffs + (lineNum * LINEDEF_SIZE_BYTES)), "failed to read DF linedef")
    }
    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        LTC_TRY(readSideDef(wad, &map->sideDefs[sideNum], mLumpsInfo->sideDefsEntry.lumpOffs + (sideNum * SIDEDEF_SIZE_BYTES)), "failed to read DF sidedef");
    }
    for (int vertNum = 0; vertNum < map->vertexNum; vertNum++) {
        LTC_TRY(readVertex(wad, &map->vertices[vertNum], mLumpsInfo->verticesEntry.lumpOffs + (vertNum * VERTEX_SIZE_BYTES)), "failed to read DF vertex")
    }
    for (int sectNum = 0; sectNum < map->sectorNum; sectNum++) {
        LTC_TRY(readSector(wad, &map->sectors[sectNum], mLumpsInfo->sectorsEntry.lumpOffs + (sectNum * SECTOR_SIZE_BYTES)), "failed to read DF sector");
    }

    return ltc_success;
}

ltc_status DFreadMap(doomMap* map, wad* wad, directoryEntry mapMarkerEntry) {

    mapLumpEntries mLumpEntries;
    LTC_TRY(getTargetMapComposition(wad, mapMarkerEntry, &mLumpEntries), "failed to get map composition");
    LTC_TRY(readMapGeometry(wad->stream, map, &mLumpEntries), "failed to read map geometry");

    return ltc_success;
}



