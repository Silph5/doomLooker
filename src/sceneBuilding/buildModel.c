//
// Created by tjada on 28/12/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#include "../../include/buildModel.h"
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

#define MSG_ERROR_VERT_ADD "Failed to add vertex to map model"
#define MSG_ERROR_WALL_ADD "Failed to add wall to map model"
#define MSG_ERROR_SIDE_ADD "Failed to add sidedef to map model"
#define MSG_ERROR_MAP_INIT "Failed to init map model"

ltc_status initMapModel(MapModel* model, size_t capacity) {
    model->verts = malloc(sizeof(ModelVert) * capacity);
    LTC_TRY(ltc_malloc((void**)&model->verts, sizeof(ModelVert) * capacity), "failed to malloc for expected vert count");

    model->vertCount = 0;
    model->vertCapacity = capacity;
    return ltc_success;
}

ltc_status addVert(MapModel* model, const ModelVert* vert) {

    if (model->vertCount == model->vertCapacity) {
        fprintf(stderr, "info: expected vertex capacity exceeded\n");
        size_t newCapacity = model->vertCount * 2;
        LTC_TRY(ltc_realloc((void**)&model->verts,sizeof(ModelVert) * newCapacity), "Failed to reallocate vert array");
    }

    const size_t newVertIndex = model->vertCount;
    model->verts[newVertIndex].x = vert->x;
    model->verts[newVertIndex].y = vert->y;
    model->verts[newVertIndex].z = vert->z;

    model->verts[newVertIndex].u = vert->u;
    model->verts[newVertIndex].v = vert->v;

    model->verts[newVertIndex].texID = vert->texID;

    model->vertCount++;
    return ltc_success;
}

ltc_status addWallFace(MapModel* model, const ModelVert* blCorner, const ModelVert* trCorner) {

    ModelVert tlCorner;
    tlCorner.x = blCorner->x;
    tlCorner.z = blCorner->z;
    tlCorner.y = trCorner->y;
    tlCorner.texID = trCorner->texID;
    tlCorner.u = blCorner->u;
    tlCorner.v = trCorner->v;

    ModelVert brCorner;
    brCorner.x = trCorner->x;
    brCorner.z = trCorner->z;
    brCorner.y = blCorner->y;
    brCorner.texID = trCorner->texID;
    brCorner.u = trCorner->u;
    brCorner.v = blCorner->v;

    LTC_TRY(addVert(model, &tlCorner), MSG_ERROR_VERT_ADD);
    LTC_TRY(addVert(model, blCorner), MSG_ERROR_VERT_ADD);
    LTC_TRY(addVert(model, &brCorner), MSG_ERROR_VERT_ADD);

    LTC_TRY(addVert(model, &tlCorner), MSG_ERROR_VERT_ADD);
    LTC_TRY(addVert(model, &brCorner), MSG_ERROR_VERT_ADD);
    LTC_TRY(addVert(model, trCorner), MSG_ERROR_VERT_ADD);

    return ltc_success;
}


void applyTexToSide(MapModel* model, ModelVert* blVert, ModelVert* trVert, const char* texName, int xOffset, int yOffset) {
    AtlasSubTexture* tex;
    HASH_FIND(hh, model->textureAtlas->subTextures, texName, 8, tex);
    if (!tex) {
        blVert->texID = 0;
        trVert->texID = 0;
        blVert->u = 0.0f; blVert->v = 1.0f;
        trVert->u = 1.0f; trVert->v = 0.0f;
        return;
    }

    blVert->texID = tex->UVindex; trVert->texID = tex->UVindex;

    float xTexStart = (float) xOffset; float yTexStart = (float) yOffset; //THESE OFFSETS DON'T WORK PROPERLY! looking into why
    float faceWidth = hypotf(blVert->x - trVert->x, blVert->z - trVert->z);
    float faceHeight = trVert->y - blVert->y;

    blVert->u = xTexStart / (float) tex->width;
    trVert->u = (xTexStart + faceWidth) / (float) tex->width;

    blVert->v = (yTexStart + faceHeight) / (float) tex->height;
    trVert->v = yTexStart / (float) tex->height;
}

ltc_status addSide(MapModel* model, SideDef* side, const Sector* sectFacing, Sector* sectBehind, const Vertex* v1, const Vertex* v2) {
    if (!side) {
        ltc_setArgFailSubject(2); return ltc_fail_invalid_arg;
    }

    //these verts are for addWallFace and are just created here to be modified and passed throughout all of this funcion
    ModelVert blVert;
    ModelVert trVert;

    if (!sectBehind) {
        if (side->midTexName[0] == '-') {
            return 0;
        }

        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectFacing->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectFacing->ceilHeight;

        applyTexToSide(model, &blVert, &trVert, side->midTexName, side->xTexOffset, side->yTexOffset);

        LTC_TRY(addWallFace(model, &blVert, &trVert), MSG_ERROR_WALL_ADD);

        return ltc_success;
    }

    if (side->midTexName[0] != '-') {

        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectBehind->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectBehind->ceilHeight;

        applyTexToSide(model, &blVert, &trVert, side->midTexName, side->xTexOffset, side->yTexOffset);

        LTC_TRY(addWallFace(model, &blVert, &trVert), MSG_ERROR_WALL_ADD);

    }

    if (side->upperTexName[0] != '-') {

        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectBehind->ceilHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectFacing->ceilHeight;

        applyTexToSide(model, &blVert, &trVert, side->upperTexName, side->xTexOffset, side->yTexOffset);

        LTC_TRY(addWallFace(model, &blVert, &trVert), MSG_ERROR_WALL_ADD);

    }

    if (side->lowerTexName[0] != '-') {

        blVert.x = v1->x; blVert.z = v1->y; blVert.y = sectFacing->floorHeight;
        trVert.x = v2->x; trVert.z = v2->y; trVert.y = sectBehind->floorHeight;

        int yOffs = side->yTexOffset;

        if (side->upperTexName[0] != '-') {
            yOffs += (sectFacing->ceilHeight - sectBehind->floorHeight);
        }

        applyTexToSide(model, &blVert, &trVert, side->lowerTexName, side->xTexOffset, yOffs);
        //lower texture offsets seem to be based on the mid and upper textures, which i didn't consider, so i had to do some hackyness
        //this still likely causes mismatches in some places

        LTC_TRY(addWallFace(model, &blVert, &trVert), MSG_ERROR_WALL_ADD);

    }

    return ltc_success;
}

ltc_status buildMapModel(MapModel* model, DoomMap* mapData) {

    //texture atlas
    model->textureAtlas = NULL;
    LTC_TRY(ltc_malloc((void**)&model->textureAtlas, sizeof(Atlas)), "failed to alloc for texture atlas");
    LTC_TRY(initAtlas(model->textureAtlas), "failed to init texture atlas");
    for (int t = 0; t < mapData->textureNum; t++) {
        LTC_TRY(addTextureToAtlas(model->textureAtlas, &mapData->textures[t]), "failed to add a texture to the texture atlas");
    }
    LTC_TRY(bakeAtlasUVs(model->textureAtlas), "failed to bake atlas UVs");

    //temporarily only init for max sidedef verts until sectors added
    LTC_TRY(initMapModel(model, mapData->sideDefNum * 18), "failed to init map model");

    //sidedefs
    for (int lineNum = 0; lineNum < mapData->lineDefNum; lineNum++) {
        SideDef* frontSide = NULL;
        SideDef* backSide = NULL;
        Sector* frontSector = NULL;
        Sector* backSector = NULL;

        Vertex* v1 = &mapData->vertices[mapData->lineDefs[lineNum].v1];
        Vertex* v2 = &mapData->vertices[mapData->lineDefs[lineNum].v2];

        if (mapData->lineDefs[lineNum].frontSideNum != 65535) {
            frontSide = &mapData->sideDefs[mapData->lineDefs[lineNum].frontSideNum];
            frontSector = &mapData->sectors[frontSide->sectFacing];
        }
        if (mapData->lineDefs[lineNum].backSideNum != 65535) {
            backSide = &mapData->sideDefs[mapData->lineDefs[lineNum].backSideNum];
            backSector = &mapData->sectors[backSide->sectFacing];
        }

        if (frontSide) {
            LTC_TRY(addSide(model, frontSide, frontSector, backSector, v1, v2), MSG_ERROR_SIDE_ADD);
        }

        if (backSide) {
            LTC_TRY(addSide(model, backSide, backSector, frontSector, v2, v1), MSG_ERROR_SIDE_ADD);
        }
    }

    return ltc_success;
}