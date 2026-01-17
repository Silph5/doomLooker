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

    atlas->bottomShelf.yOffset = 0;
    atlas->bottomShelf.height = ATLAS_INIT_SHELF_HEIGHT;
    atlas->bottomShelf.nextOriginX = 0;

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

    int xStart = atlas->bottomShelf.nextOriginX;
    int yStart = atlas->bottomShelf.yOffset;

    for (int tY = 0; tY < texture->height; tY++) {
        int texRowStartI = tY * texture->width;
        int atlasRowStartI = (yStart + tY) * atlas->width + xStart;

        memcpy(&atlas->pixels[atlasRowStartI], &texture->pixels[texRowStartI], texture->width * sizeof(uint32_t));
    }

    //need to still do atlas subtext hashtable add so each texture is accessible. testing actual image contruction first.
    return 0;
}

void exportAtlas(atlas* atlas) {

}