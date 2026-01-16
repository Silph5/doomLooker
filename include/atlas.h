//
// Created by tjada on 16/01/2026.
//

#ifndef DOOMLOOKER_ATLAS_H
#define DOOMLOOKER_ATLAS_H

#include "texture.h"
#include "ut_hash/uthash.h"

#define ATLAS_ROW_WIDTH 1024

typedef struct {
    uint16_t height;
    uint16_t yOffset;
    uint16_t nextOriginX;
} atlasShelf;

typedef struct {
    char name[8]; //hash table key

    uint16_t originX;
    uint16_t originY;
    uint16_t width;
    uint16_t height;

    UT_hash_handle hh;
} atlasSubTexture;

typedef struct {
    uint32_t pixels;
    uint16_t height;
    uint16_t width;

    atlasSubTexture* subTextures;
    atlasShelf bottomShelf; //to track where to insert textures and when to create a new shelf
} atlas;

int initAtlas (atlas* atlas);

int addTextureToAtlas(atlas* atlas, texture* texture);

void exportAtlas(atlas* atlas);

#endif //DOOMLOOKER_ATLAS_H