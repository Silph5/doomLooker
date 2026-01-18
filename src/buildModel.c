//
// Created by tjada on 28/12/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "../include/buildModel.h"
#include "vertex.h"

#include "mapStruct.h"
#include "mapComponentStructs.h"
#include "texture.h"
#include "atlas.h"




#define TRY(func, onError, errorMsg) do { \
    int retval = (func); \
    if (retval != 0) { \
        fprintf(stderr, errorMsg); \
        onError; \
    } \
} while (0);

#define MSG_ERROR_VERT_ADD "Failed to add vertex to map model\n"
#define MSG_ERROR_WALL_ADD "Failed to add wall to map model\n"
#define MSG_ERROR_SIDE_ADD "Failed to add sidedef to map model\n"
#define MSG_ERROR_MAP_INIT "Failed to init map model\n"

int initMapModel(mapModel* model, size_t capacity) {
    model->vertCoords = malloc(sizeof(float) * capacity * 3);
    if (!model->vertCoords) {
        return -1;
    }

    model->vertUVs = malloc(sizeof(float) * capacity * 2);
    if (!model->vertUVs) {
        return -1;
    }

    model->vertCount = 0;
    model->vertCapacity = capacity;
    return 0;
}

int addVert(mapModel* model, const modelVert* vert) {

    if (model->vertCount == model->vertCapacity) {
        fprintf(stderr, "info: expected vertex capacity exceeded\n");
        size_t newCapacity = model->vertCount * 2;
        float* temp = realloc(model->vertCoords,sizeof(float) * newCapacity * 3);
        if (!temp) {
            fprintf(stderr, "failed realloc during model build\n");
            return -1;
        }
        model->vertCoords = temp;

        temp = realloc(model->vertUVs,sizeof(float) * newCapacity * 2);
        if (!temp) {
            fprintf(stderr, "failed realloc during model build\n");
            return -1;
        }
        model->vertUVs = temp;

        model->vertCapacity = newCapacity;
    }

    const size_t xyzStart = model->vertCount * 3;
    model->vertCoords[xyzStart + 0] = vert->x;
    model->vertCoords[xyzStart + 1] = vert->y;
    model->vertCoords[xyzStart + 2] = vert->z;

    const size_t uvStart = model->vertCount * 2;
    model->vertUVs[uvStart + 0] = vert->u;
    model->vertUVs[uvStart + 1] = vert->v;

    model->vertCount++;

    return 0;
}

int addWallFace(mapModel* model, modelVert* blCorner, modelVert* trCorner) {

    modelVert tlCorner;
    tlCorner.x = blCorner->x;
    tlCorner.z = blCorner->z;
    tlCorner.y = trCorner->y;

    modelVert brCorner;
    brCorner.x = trCorner->x;
    brCorner.z = trCorner->z;
    brCorner.y = blCorner->y;

    //TEMPORARY UV ASSIGNMENT

    tlCorner.u = 0.0f;
    tlCorner.v = 1.0f;

    blCorner->u = 0.0f;
    blCorner->v = 0.0f;

    brCorner.u = 1.0f;
    brCorner.v = 0.0f;

    trCorner->u = 1.0f;
    trCorner->v = 1.0f;

    TRY(addVert(model, &tlCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, blCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, &brCorner), return -1, MSG_ERROR_VERT_ADD);

    TRY(addVert(model, &tlCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, &brCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, trCorner), return -1, MSG_ERROR_VERT_ADD);

    return 0;
}

int addSide(mapModel* model, sideDef* side, const sector* sectFacing, sector* sectBehind, const vertex* v1, const vertex* v2) {
    if (!side) {
        return 0;
    }

    //these verts are for addWallFace and are just created here to be modified and passed throughout all of this funcion
    modelVert blVert;
    modelVert trVert;

    if (!sectBehind) {
        if (side->midTexName[0] == '-') {
            return 0;
        }


        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectFacing->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectFacing->ceilHeight;

        TRY(addWallFace(model, &blVert, &trVert), return -1, MSG_ERROR_WALL_ADD)

        return 0;
    }

    if (side->midTexName[0] != '-') {
        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectBehind->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectBehind->ceilHeight;

        TRY(addWallFace(model, &blVert, &trVert), return -1, MSG_ERROR_WALL_ADD)

    }

    if (side->upperTexName[0] != '-') {
        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectBehind->ceilHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectFacing->ceilHeight;

        TRY(addWallFace(model, &blVert, &trVert), return -1, MSG_ERROR_WALL_ADD)

    }

    if (side->lowerTexName[0] != '-') {
        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectFacing->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectBehind->floorHeight;

        TRY(addWallFace(model, &blVert, &trVert), return -1, MSG_ERROR_WALL_ADD)

    }

    return 0;
}

mapModel* buildMapModel(doomMap* mapData) {

    mapModel* model = malloc(sizeof(mapModel));
    if (!model) {
        fprintf(stderr, "failed to malloc verts container");
        return NULL;
    }

    //texture atlas (temp implementation)
    model->textureAtlas = malloc(sizeof(atlas));
    initAtlas(model->textureAtlas);
    for (int t = 0; t < mapData->textureNum; t++) {
        addTextureToAtlas(model->textureAtlas, &mapData->textures[t]);
    }
    exportAtlas(model->textureAtlas);


    //temporarily only init for max sidedef verts until sectors added
    TRY(initMapModel(model, mapData->sideDefNum * 18), return NULL, MSG_ERROR_MAP_INIT);

    //sidedefs
    for (int lineNum = 0; lineNum < mapData->lineDefNum; lineNum++) {
        sideDef* frontSide = NULL;
        sideDef* backSide = NULL;
        sector* frontSector = NULL;
        sector* backSector = NULL;

        vertex* v1 = &mapData->vertices[mapData->lineDefs[lineNum].v1];
        vertex* v2 = &mapData->vertices[mapData->lineDefs[lineNum].v2];

        if (mapData->lineDefs[lineNum].frontSideNum != 65535) {
            frontSide = &mapData->sideDefs[mapData->lineDefs[lineNum].frontSideNum];
            frontSector = &mapData->sectors[frontSide->sectFacing];
        }
        if (mapData->lineDefs[lineNum].backSideNum != 65535) {
            backSide = &mapData->sideDefs[mapData->lineDefs[lineNum].backSideNum];
            backSector = &mapData->sectors[backSide->sectFacing];
        }

        TRY(addSide(model, frontSide, frontSector, backSector, v1, v2), break, MSG_ERROR_SIDE_ADD);
        TRY(addSide(model, backSide, backSector, frontSector, v2, v1), break, MSG_ERROR_SIDE_ADD);

    }

    return model;
}
