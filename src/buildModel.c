//
// Created by tjada on 28/12/2025.
//

#include "../include/buildModel.h"
#include "vertex.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define TRY(func, onError, errorMsg) do { \
    int retval = (func); \
    if (retval != 0) { \
        fprintf(stderr, errorMsg); \
        onError; \
    } \
} while (0);

#define MSG_ERROR_VERT_ADD "Failed to add vertex to map model\n"
#define MSG_ERROR_WALL_ADD "Failed to add wall to map model\n"

int initMapModel(mapModel* model, size_t capacity) {
    model->vertCoords = malloc(sizeof(float) * capacity * 3);
    if (!model->vertCoords) {
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
        model->vertCapacity = newCapacity;
    }

    const size_t start = model->vertCount * 3;
    model->vertCoords[start + 0] = vert->x;
    model->vertCoords[start + 1] = vert->y;
    model->vertCoords[start + 2] = vert->z;

    model->vertCount++;

    return 0;
}

int addWallFace(mapModel* model, const modelVert* blCorner, const modelVert* trCorner) {

    modelVert tlCorner;
    tlCorner.x = blCorner->x;
    tlCorner.z = blCorner->z;
    tlCorner.y = trCorner->y;

    modelVert brCorner;
    brCorner.x = trCorner->x;
    brCorner.z = trCorner->z;
    brCorner.y = blCorner->y;

    TRY(addVert(model, &tlCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, &brCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, blCorner), return -1, MSG_ERROR_VERT_ADD);

    TRY(addVert(model, &tlCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, trCorner), return -1, MSG_ERROR_VERT_ADD);
    TRY(addVert(model, &brCorner), return -1, MSG_ERROR_VERT_ADD);

    return 0;
}

mapModel* buildTestVerts() {

    mapModel* list = malloc(sizeof(mapModel));
    if (!list) {
        fprintf(stderr, "failed to malloc verts container");
        return NULL;
    }

    initMapModel(list, 12);

    modelVert bl; bl.x = 0.0f; bl.y = 0.0f; bl.z = 1.0f;

    modelVert tr; tr.x = 1.0f; tr.y = 1.0f; tr.z = 0.0f;

    modelVert tr2; tr2.x = 1.0f; tr2.y = 0.0f; tr2.z = -1.0f;

    modelVert bl2; bl2.x = 1.0f; bl2.y = 1.0f; bl2.z = 0.0f;

    TRY(addWallFace(list, &bl, &tr), return NULL, MSG_ERROR_WALL_ADD);
    TRY(addWallFace(list, &bl2, &tr2), return NULL, MSG_ERROR_WALL_ADD);

    return list;
}
