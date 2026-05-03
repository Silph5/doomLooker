//
// Created by tjada on 16/01/2026.
//

#ifndef DOOMLOOKER_ATLAS_H
#define DOOMLOOKER_ATLAS_H

#include <libtrychain.h>
#include "texture.h"
#include "ut_hash/uthash.h"

#define ATLAS_ROW_WIDTH 1024
#define ATLAS_INIT_SHELF_HEIGHT 128

typedef struct {
    uint16_t height;
    uint16_t yOffset;
    uint16_t nextOriginX;
} AtlasShelf;

typedef struct {
    char name[8]; //hash table key

    uint16_t originX;
    uint16_t originY;
    uint16_t width;
    uint16_t height;

    int UVindex;

    UT_hash_handle hh;
} AtlasSubTexture;

typedef struct {
    uint32_t* pixels;
    uint16_t height;
    uint16_t width;

    AtlasShelf bottomShelf; //to track where to insert textures and when to create a new shelf

    AtlasSubTexture* subTextures;
    int subTexCount;

    float* subTUVS;
} Atlas;

ltc_status initAtlas (Atlas* atlas);

ltc_status addTextureToAtlas(Atlas* atlas, const Texture* texture);

ltc_status bakeAtlasUVs(Atlas* atlas);

void exportAtlas(Atlas* atlas);

#endif //DOOMLOOKER_ATLAS_H