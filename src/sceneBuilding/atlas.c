//
// Created by tjada on 16/01/2026.
//

#include <stdio.h>

#include "atlas.h"

ltc_status initAtlas (atlas* atlas) {

    atlas->width = ATLAS_ROW_WIDTH;
    atlas->height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->pixels = NULL;
    LTC_TRY(ltc_calloc((void**)&atlas->pixels, ATLAS_ROW_WIDTH * ATLAS_INIT_SHELF_HEIGHT, sizeof(uint32_t)), "Failed to alloc for initial alloc pixels");

    atlas->subTextures = NULL;
    atlas->subTUVS = NULL;

    atlas->bottomShelf.yOffset = 0;
    atlas->bottomShelf.height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->bottomShelf.nextOriginX = 0;

    atlas->subTexCount = 0;

    return ltc_success;
}

ltc_status changeAtlasHeight (atlas* atlas, int newHeight) {

    size_t oldSize = atlas->height * ATLAS_ROW_WIDTH * sizeof(uint32_t);
    size_t newSize = newHeight * ATLAS_ROW_WIDTH * sizeof(uint32_t);

    LTC_TRY(ltc_realloc((void**)&atlas->pixels, newSize), "Failed to re-allocate texture atlas pixel array");

    if (newSize > oldSize) {
        memset((char*) atlas->pixels + oldSize, 0, newSize - oldSize);
    }

    atlas->height = newHeight;

    return ltc_success;
}

ltc_status createNewAtlasShelf (atlas* atlas) {

    uint16_t newOffset = atlas->bottomShelf.height + atlas->bottomShelf.yOffset;

    atlas->bottomShelf.yOffset = newOffset;
    atlas->bottomShelf.height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->bottomShelf.nextOriginX = 0;

    LTC_TRY(changeAtlasHeight(atlas, atlas->height + ATLAS_INIT_SHELF_HEIGHT), "Failed to alter atlas height");

    return ltc_success;
}

ltc_status changeAtlasBottomShelfHeight (atlas* atlas, int newHeight) {

    LTC_TRY(changeAtlasHeight(atlas, atlas->bottomShelf.yOffset + newHeight), "Failed to alter atlas height");
    atlas->bottomShelf.height = newHeight;

    return ltc_success;
}

ltc_status addTextureToAtlas(atlas* atlas, const texture* texture) {

    if (atlas->bottomShelf.nextOriginX + texture->width > atlas->width) {
        LTC_TRY(createNewAtlasShelf(atlas), "failed to create new atlas shelf");
    }

    if (texture->height > atlas->bottomShelf.height) {
        LTC_TRY(changeAtlasBottomShelfHeight(atlas, texture->height), "Failed to alter atlas bottom shelf height");
    }

    atlasSubTexture* subTexture = NULL;
    LTC_TRY(ltc_malloc((void**)&subTexture, sizeof(atlasSubTexture)), "failed to allocate for atlas subTexture");

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

    return ltc_success;
}

ltc_status bakeAtlasUVs(atlas* atlas) {
    if (atlas->subTUVS) {
        free(atlas->subTUVS);
    }

    atlas->subTUVS = NULL;

    LTC_TRY(ltc_malloc((void**)&atlas->subTUVS, sizeof(float) * 4 * atlas->subTexCount), "Failed to allocate for atlas subtexture UVs");

    atlasSubTexture *cur, *tmp;
    HASH_ITER(hh, atlas->subTextures, cur, tmp) {
        int TUVBaseIndex = cur->UVindex * 4;
        atlas->subTUVS[TUVBaseIndex] = (float) cur->originX / (float) atlas->width;
        atlas->subTUVS[TUVBaseIndex + 1] = (float) cur->originY / (float) atlas->height;
        atlas->subTUVS[TUVBaseIndex + 2] = (float) (cur->originX + cur->width) / (float) atlas->width;
        atlas->subTUVS[TUVBaseIndex + 3] = (float) (cur->originY +cur-> height)/ (float) atlas->height;
    }

    return ltc_success;
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