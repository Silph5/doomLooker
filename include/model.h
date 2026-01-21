//
// Created by tjada on 30/12/2025.
//

#ifndef DOOMLOOKER_MODEL_H
#define DOOMLOOKER_MODEL_H
#include <stddef.h>
#include "atlas.h"
#include "vertex.h"

typedef struct {
    modelVert* verts;
    size_t vertCount;
    size_t vertCapacity;

    atlas* textureAtlas;
} mapModel;

#endif //DOOMLOOKER_MODEL_H