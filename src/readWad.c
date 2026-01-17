//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#include "ut_hash/uthash.h"
#include "mapStruct.h"
#include "mapComponentStructs.h"
#include "texture.h"

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

#define MAX_TEXTUREX_EXPECTED 4

typedef struct {
    mapLumps targMapLumps;
    directoryEntry pnames;
    directoryEntry* textureDefs; //textureX
    int textureDefCount;

    directoryEntry playPal;
    directoryEntryHashed* patches;
} reqWadLumps;

typedef struct {
    char name[8];
    int textureIndex;

    UT_hash_handle hh;
} mapTexNameHashed;

typedef struct {
    int nameCount;
    char (*names)[8];
} namesTable;

typedef struct {
    uint8_t r, g, b;
} doomCol;

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

    wadLumps->textureDefCount = 0;
    wadLumps->textureDefs = malloc(sizeof(directoryEntry) * MAX_TEXTUREX_EXPECTED);
    if (!wadLumps->textureDefs) {
        fprintf(stderr, "failed to malloc for textureX lumps");
        return 0;
    }

    bool foundTex = false;
    bool foundPatches = false;
    bool foundPNames = false;
    bool foundMap = false;
    bool foundPlayPal = false;

    for (int entryNum = 0; entryNum < header->lumpsNum; entryNum++) {
        fread(&tempEntry, sizeof(directoryEntry), 1, wad);

        if (strncmp(tempEntry.lumpName, "PLAYPAL", 7) == 0) {
            wadLumps->playPal = tempEntry;
            foundPlayPal = true;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "TEXTURE1", 8) == 0 || strncmp(tempEntry.lumpName, "TEXTURE2", 8) == 0) {
            wadLumps->textureDefs[wadLumps->textureDefCount] = tempEntry;
            wadLumps->textureDefCount += 1;
            foundTex = true;
            continue;
        }

        if (strncmp(tempEntry.lumpName, "P_START", 7) == 0) {
            collectPatchEntries(wad, &wadLumps->patches, &entryNum);
            foundPatches = true;
            continue;
        }
        if (strncmp(tempEntry.lumpName, "PNAMES", 6) == 0) {
            wadLumps->pnames = tempEntry;
            foundPNames = true;
            continue;
        }
        if (strncmp(tempEntry.lumpName, targetMap, 5) == 0) {
            getTargetMapComposition(wad, &wadLumps->targMapLumps, &entryNum);
            foundMap = true;
        }
    }

    if (!(foundTex && foundPatches && foundPNames && foundMap && foundPlayPal)) {
        return 0;
    }
    return 1;
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

int readMapGeometry (FILE* wad, doomMap* map, mapLumps* mLumpsInfo) {

    map->lineDefNum = mLumpsInfo->lineDefsEntry.lumpSize / LINEDEF_SIZE_BYTES;
    map->sideDefNum = mLumpsInfo->sideDefsEntry.lumpSize / SIDEDEF_SIZE_BYTES;
    map->vertexNum = mLumpsInfo->verticesEntry.lumpSize / VERTEX_SIZE_BYTES;
    map->sectorNum = mLumpsInfo->sectorsEntry.lumpSize / SECTOR_SIZE_BYTES;

    map->lineDefs = malloc(sizeof(lineDef) * map->lineDefNum);
    if (map->lineDefs == NULL) {fprintf(stderr, "Failed to allocate memory for linedefs"); return 0;}
    for (int lineNum = 0; lineNum < map->lineDefNum; lineNum++) {
        readLineDef(wad, &map->lineDefs[lineNum], mLumpsInfo->lineDefsEntry.lumpOffs + (lineNum * 14));
    }

    map->sideDefs = malloc(sizeof(sideDef) * map->sideDefNum);
    if (map->sideDefs == NULL) {fprintf(stderr, "Failed to allocate memory for sidedefs"); return 0;}
    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        readSideDef(wad, &map->sideDefs[sideNum], mLumpsInfo->sideDefsEntry.lumpOffs + (sideNum * 30));
    }

    map->vertices = malloc(sizeof(vertex) * map->sideDefNum);
    if (map->vertices == NULL) {fprintf(stderr, "Failed to allocate memory for vertices"); return 0;}
    for (int vertNum = 0; vertNum < map->vertexNum; vertNum++) {
        readVertex(wad, &map->vertices[vertNum], mLumpsInfo->verticesEntry.lumpOffs + (vertNum * 4));
    }

    map->sectors = malloc(sizeof(sector) * map->sectorNum);
    if (map->sectors == NULL) {fprintf(stderr, "Failed to allocate memory for sectors"); return 0;}
    for (int sectNum = 0; sectNum < map->sectorNum; sectNum++) {
        readSector(wad, &map->sectors[sectNum], mLumpsInfo->sectorsEntry.lumpOffs + (sectNum * 26));
    }

    return 1;
}

void normaliseTexName(char* name) {
    for (int c = 0; c < 8; c++ /*heh*/) {
        if (name[c] == '\0') {
            break;
        }
        name[c] = toupper((unsigned char)name[c]);
    }
}

void addUsedTexture(mapTexNameHashed** usedTexTable, const char* texName, int* outCount) {

    if (texName[0] == '-' || texName[0] == '\0')
        return;

    char normTexName[8];

    memcpy(normTexName, texName, 8);
    normaliseTexName(normTexName);

    mapTexNameHashed *entry;
    HASH_FIND(hh, *usedTexTable, normTexName, 8, entry);

    if (entry == NULL) {
        entry = malloc(sizeof(*entry));
        memcpy(entry->name, normTexName, 8);
        entry->textureIndex = -1;

        HASH_ADD(hh, *usedTexTable, name, 8, entry);
        *outCount += 1;
    }
}

void collectUsedTextures (mapTexNameHashed** usedTexTable, doomMap* map, int* outCount) {

    for (int sideNum = 0; sideNum < map->sideDefNum; sideNum++) {
        addUsedTexture(usedTexTable, map->sideDefs[sideNum].lowerTexName, outCount);
        addUsedTexture(usedTexTable, map->sideDefs[sideNum].midTexName, outCount);
        addUsedTexture(usedTexTable, map->sideDefs[sideNum].upperTexName, outCount);
    }
}

void getColourPalette (FILE* wad, directoryEntry* entry, doomCol* palette) {

    //i'm only reading the first palette from playPal, i'll try to emulate doom's shading via GLSL
    fseek(wad, entry->lumpOffs, SEEK_SET);
    for (int col = 0; col < 256; col++) {
        fread(&palette[col], sizeof(doomCol), 1, wad);
    }
}

int collectPNames(FILE *wad, const directoryEntry* entry, namesTable* table) {

    fseek(wad, entry->lumpOffs, SEEK_SET);
    fread(&table->nameCount, sizeof(int), 1, wad);

    table->names = malloc(table->nameCount * sizeof(*table->names));
    if (!table->names) {
        fprintf(stderr, "Failed malloc\n");
        return 0;
    }

    fread(table->names, sizeof(*table->names), table->nameCount, wad);

    for (int p = 0; p < table->nameCount; p++) {
        normaliseTexName(table->names[p]);
    }

    return 1;
}

void insertPatchToTexture(FILE* wad, directoryEntryHashed* patchEntry, texture* tex, int16_t yStart, int16_t xStart, doomCol* palette) {

    fseek(wad, patchEntry->lumpOffs, SEEK_SET);

    uint16_t pWidth; uint16_t pHeight;

    fread(&pWidth, sizeof(uint16_t), 1, wad);
    fread(&pHeight, sizeof(uint16_t), 1, wad);

    fseek(wad, 4, SEEK_CUR);

    uint32_t* columnOffs = malloc(pWidth*sizeof(uint32_t));
    if (!columnOffs) {
        return;
    }

    fread(columnOffs, sizeof(uint32_t), pWidth, wad);

    for (int pX = 0; pX < pWidth; pX++) {
        fseek(wad, patchEntry->lumpOffs + (long) columnOffs[pX], SEEK_SET);

        uint8_t topDelta = 0; uint8_t len;

        while (topDelta != 0xFF) {
            fread(&topDelta, sizeof(uint8_t), 1, wad);
            if (topDelta == 0xFF) {
                break;
            }

            fread(&len, sizeof(uint8_t), 1, wad);
            fseek(wad, 1, SEEK_CUR); //padding byte

            for (int p = 0; p < len; p++) {
                uint8_t palIndex;
                fread(&palIndex, sizeof(uint8_t), 1, wad);

                int tX = xStart + pX;
                int tY = yStart + topDelta + p;

                doomCol curCol = palette[palIndex];

                uint32_t packedCol = 0xFF << 24 | curCol.r << 16 | curCol.g << 8 | curCol.b;

                if (tX >= 0 && tX < tex->width && tY >= 0 && tY < tex->height) {
                    tex->pixels[tY * tex->width + tX] = packedCol;
                }
            }

            fseek(wad, 1, SEEK_CUR); //padding byte
        }
    }

    free(columnOffs);
}

void compositeTexture(FILE *wad, texture* outTex, int offset, directoryEntryHashed* patches, namesTable* patchTable, doomCol* palette) {

    fseek(wad, offset, SEEK_SET);

    fread(&outTex->name, sizeof(char), 8, wad);

    fseek(wad, 4, SEEK_CUR);

    fread(&outTex->width, sizeof(int16_t), 1, wad);
    fread(&outTex->height, sizeof(int16_t), 1, wad);
    outTex->pixels = calloc(outTex->width * outTex->height, sizeof(uint32_t));
    if (!outTex->pixels) {
        fprintf(stderr, "failed to malloc for texture %.8s pixel data", outTex->name);
        return;
    }

    fseek(wad, 4, SEEK_CUR);

    int16_t patchCount;

    fread(&patchCount, sizeof(int16_t), 1, wad);

    int patchStartOffs = offset + 22;

    for (int p = 0; p < patchCount; p++) {
        fseek(wad, patchStartOffs + (10 * p), SEEK_SET);

        int16_t xStart, yStart, patchId;

        fread(&xStart, sizeof(int16_t), 1, wad);
        fread(&yStart, sizeof(int16_t), 1, wad);
        fread(&patchId, sizeof(int16_t), 1, wad);

        char patchName[8];
        memcpy(patchName, patchTable->names[patchId], 8);

        directoryEntryHashed* patchEntry = NULL;

        HASH_FIND(hh, patches, patchName, 8, patchEntry);
        if (patchEntry) {
            insertPatchToTexture(wad, patchEntry, outTex, yStart, xStart, palette);
        }
    }
}

void readTextureDefAndCompositeUsed(FILE *wad, texture* textures, directoryEntry* textureDefEntry, directoryEntryHashed* patches, namesTable* patchTable, mapTexNameHashed* usedTextureTable, doomCol* palette, int* compTexCount) {

    //texture1
    int textureDefTexCount;
    fseek(wad, textureDefEntry->lumpOffs, SEEK_SET); //use file stream

    fread(&textureDefTexCount, sizeof(int), 1, wad);

    int* textureOffsets = malloc(sizeof(int) * textureDefTexCount);

    for (int t = 0; t < textureDefTexCount; t++) {
        fread(&textureOffsets[t], sizeof(int), 1, wad);
    }

    for (int t = 0; t < textureDefTexCount; t++) {
        char nextTexName[8];
        mapTexNameHashed *texEntry = NULL;

        fseek(wad, textureDefEntry->lumpOffs + textureOffsets[t], SEEK_SET);
        fread(nextTexName, sizeof(char), 8, wad);

        HASH_FIND(hh, usedTextureTable, nextTexName, 8, texEntry);
        if (texEntry && texEntry->textureIndex == -1) {

            texEntry->textureIndex = *compTexCount;
            compositeTexture(wad, &textures[*compTexCount], textureDefEntry->lumpOffs + textureOffsets[t], patches, patchTable, palette);

            *compTexCount += 1;

        }

    }

    free(textureOffsets);
}


doomMap* readWadToMapData(const char* wadPath, const char* mapName) {

    doomMap* map = NULL;
    FILE* wad = NULL;
    doomCol* colPalette = NULL;
    mapTexNameHashed *usedTextures = NULL;
    header header;
    reqWadLumps reqLumpEntries;
    namesTable pNamesTable;

    map = malloc(sizeof(doomMap));
    if (!map) {
        fprintf(stderr, "Failed malloc\n");
        return NULL;
    }

    wad = fopen(wadPath, "rb");
    if (!wad) {
        free(map);
        return NULL;
    }

    readHeader(wad, &header);
    if (strcmp(header.ident, "IWAD") != 0 && strcmp(header.ident, "PWAD") != 0) {
        fprintf(stderr, "Incorrect file type given.\nfile ident:%s\n", header.ident);
        free(map);
        return NULL;
    }

    if (!getRequiredLumpEntries(wad, &reqLumpEntries, &header, mapName)) {
        fprintf(stderr, "Failed to locate vital wad lumps\n");
        return NULL;
    }
    if (!readMapGeometry(wad, map, &reqLumpEntries.targMapLumps)) {
        fprintf(stderr, "Failed to read map geometry\n");
        return NULL;
    }

    colPalette = malloc(sizeof(doomCol) * 256);
    if (!colPalette) {
        fprintf(stderr, "Failed to allocate memory for colour palette\n");
        return NULL;
    }
    getColourPalette(wad, &reqLumpEntries.playPal, colPalette);

    if (!collectPNames(wad, &reqLumpEntries.pnames, &pNamesTable)) {
        fprintf(stderr, "Failed to read Pnames lump\n");
        return NULL;
    }

    int usedTexCount = 0;
    collectUsedTextures(&usedTextures, map, &usedTexCount);
    map->textures = malloc(sizeof(texture) * usedTexCount);
    if (!map->textures) {
        fprintf(stderr, "Failed to allocate memory for textures");
        return NULL;
    }

    int compositedTexCount = 0;
    for (int t = 0; t < reqLumpEntries.textureDefCount; t++) {
        readTextureDefAndCompositeUsed(wad, map->textures, &reqLumpEntries.textureDefs[t], reqLumpEntries.patches, &pNamesTable, usedTextures, colPalette, &compositedTexCount);
    }
    map->textureNum = compositedTexCount;

    texture* temp = realloc(map->textures,sizeof(texture) * map->textureNum);
    if (temp) {map->textures = temp;} //this realloc is really unnecessary but it just feels right

    //cleanup
    free(colPalette);
    free(reqLumpEntries.patches);
    free(reqLumpEntries.textureDefs);

    mapTexNameHashed *cur, *tmp;
    HASH_ITER(hh, usedTextures, cur, tmp) {
        HASH_DEL(usedTextures, cur);
        free(cur);
    }

    fclose(wad);

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