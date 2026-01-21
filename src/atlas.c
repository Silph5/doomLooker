//
// Created by tjada on 16/01/2026.
//

#include <stdio.h>

#include "atlas.h"

int initAtlas (atlas* atlas) {

    atlas->width = ATLAS_ROW_WIDTH;
    atlas->height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->pixels = calloc(ATLAS_ROW_WIDTH * ATLAS_INIT_SHELF_HEIGHT, sizeof(uint32_t));
    if (!atlas->pixels) {
        fprintf(stderr, "failed to initialise texture atlas\n");
        return -1;
    }

    atlas->subTextures = NULL;
    atlas->subTUVS = NULL;

    atlas->bottomShelf.yOffset = 0;
    atlas->bottomShelf.height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->bottomShelf.nextOriginX = 0;

    atlas->subTexCount = 0;

    return 0;
}

int changeAtlasHeight (atlas* atlas, int newHeight) {

    size_t oldSize = atlas->height * ATLAS_ROW_WIDTH * sizeof(uint32_t);
    size_t newSize = newHeight * ATLAS_ROW_WIDTH * sizeof(uint32_t);

    uint32_t* temp = realloc(atlas->pixels, newSize);
    if (!temp) {
        return -1;
    }

    if (newSize > oldSize) {
        memset((char*) temp + oldSize, 0, newSize - oldSize);
    }

    atlas->pixels = temp;
    atlas->height = newHeight;

    return 0;
}

int createNewAtlasShelf (atlas* atlas) {

    uint16_t newOffset = atlas->bottomShelf.height + atlas->bottomShelf.yOffset;

    atlas->bottomShelf.yOffset = newOffset;
    atlas->bottomShelf.height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->bottomShelf.nextOriginX = 0;

    changeAtlasHeight(atlas, atlas->height + ATLAS_INIT_SHELF_HEIGHT);

    return 0;
}

int changeAtlasBottomShelfHeight (atlas* atlas, int newHeight) {

    changeAtlasHeight(atlas, atlas->bottomShelf.yOffset + newHeight);
    atlas->bottomShelf.height = newHeight;

    return 0;
}

int addTextureToAtlas(atlas* atlas, const texture* texture) {

    if (atlas->bottomShelf.nextOriginX + texture->width > atlas->width) {
        createNewAtlasShelf(atlas);
    }

    if (texture->height > atlas->bottomShelf.height) {
        changeAtlasBottomShelfHeight(atlas, texture->height);
    }

    atlasSubTexture* subTexture = malloc(sizeof(atlasSubTexture));
    if (!subTexture) {
        return -1;
    }

    int xStart = atlas->bottomShelf.nextOriginX;
    int yStart = atlas->bottomShelf.yOffset;

    subTexture->originX = xStart;
    subTexture->originY = yStart;
    subTexture->width = texture->width;
    subTexture->height = texture->height;
    memcpy(subTexture->name, texture->name, 8);

    subTexture->UVindex = atlas->subTexCount;
    atlas->subTexCount++;

    HASH_ADD(hh, atlas->subTextures, name, 8, subTexture);

    for (int tY = 0; tY < texture->height; tY++) {
        int texRowStartI = tY * texture->width;
        int atlasRowStartI = (yStart + tY) * atlas->width + xStart;

        memcpy(&atlas->pixels[atlasRowStartI], &texture->pixels[texRowStartI], texture->width * sizeof(uint32_t));
    }

    atlas->bottomShelf.nextOriginX += texture->width;

    return 0;
}

int bakeAtlasUVs(atlas* atlas) {
    if (atlas->subTUVS) {
        free(atlas->subTUVS);
    }

    atlas->subTUVS = NULL;

    atlas->subTUVS = malloc(sizeof(float) * 4 * atlas->subTexCount);
    if (!atlas->subTUVS) {
        fprintf(stderr, "failed to bake atlas UVs");
        return -1;
    }

    atlasSubTexture *cur, *tmp;
    HASH_ITER(hh, atlas->subTextures, cur, tmp) {
        int TUVBaseIndex = cur->UVindex * 4;
        atlas->subTUVS[TUVBaseIndex] = (float) cur->originX / (float) atlas->width;
        atlas->subTUVS[TUVBaseIndex + 1] = (float) cur->originY / (float) atlas->height;
        atlas->subTUVS[TUVBaseIndex + 2] = (float) (cur->originX + cur->width) / (float) atlas->width;
        atlas->subTUVS[TUVBaseIndex + 3] = (float) (cur->originY +cur-> height)/ (float) atlas->height;
    }

    return 0;
}

void exportAtlas(atlas* atlas) { //TEMPORARY EXPORTING AS PPM, this isn't widely supported
    FILE* ppm = fopen("../texAtlas.ppm", "wb");
    if (!ppm) {
        return;
    }

    fprintf(ppm, "P6\n%d %d \n255\n", atlas->width, atlas->height);

    for (int y = 0; y < atlas->height; y++) {
        for (int x = 0; x < atlas->width; x++) {
            uint32_t pixel = atlas->pixels[y * atlas->width + x];

            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            fputc(r, ppm);
            fputc(g, ppm);
            fputc(b, ppm);
        }
    }
}