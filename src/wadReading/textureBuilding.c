//
// Created by tjada on 02/02/2026.
//

#include <stdio.h>
#include <stdlib.h>

#include "directoryEntry.h"
#include "mapComponentStructs.h"
#include "mapStruct.h"
#include "texture.h"
#include "ut_hash/uthash.h"

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
        //normaliseTexName(table->names[p]);
    }

    return 1;
}

void addUsedTexture(mapTexNameHashed** usedTexTable, const char* texName, int* outCount) {

    if (texName[0] == '-' || texName[0] == '\0')
        return;

    char normTexName[8];

    memcpy(normTexName, texName, 8);
    //normaliseTexName(normTexName);

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



#include "../../include/textureBuilding.h"
