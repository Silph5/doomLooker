//
// Created by tjada on 28/12/2025.
//

#include "../include/buildModel.h"
#include "vertex.h"

#include <stdio.h>
#include <stdlib.h>

#define TRY(func, onError) do { \
    int retval = (func); \
    if (retval != 0) { \
        fprintf(stderr, "error passed down during model build\n"); \
        onError; \
    } \
} while (0);

int initVertList(vertsList* list, size_t capacity) {
    list->vertPositions = malloc(sizeof(float) * capacity * 3);
    if (!list->vertPositions) {
        return -1;
    }

    list->size = 0;
    list->capacity = capacity;
    return 0;
}

int addVert(vertsList* list, const modelVert* vert) {

    if (list->size == list->capacity) {
        fprintf(stderr, "info: expected vertex capacity exceeded\n");
        size_t newCapacity = list->capacity * 2;
        float* temp = realloc(list->vertPositions,sizeof(float) * newCapacity * 3);
        if (!temp) {
            fprintf(stderr, "failed realloc during model build\n");
            return -1;
        }
        list->vertPositions = temp;
        list->capacity = newCapacity;
    }

    const size_t start = list->size * 3;
    list->vertPositions[start + 0] = vert->x;
    list->vertPositions[start + 1] = vert->y;
    list->vertPositions[start + 2] = vert->z;

    list->size++;

    return 0;
}

int addWallFace(vertsList* list, const modelVert* blCorner, const modelVert* trCorner) {

    modelVert tlCorner;
    tlCorner.x = blCorner->x;
    tlCorner.z = blCorner->z;
    tlCorner.y = trCorner->y;

    modelVert brCorner;
    brCorner.x = trCorner->x;
    brCorner.z = trCorner->z;
    brCorner.y = blCorner->y;

    TRY(addVert(list, &tlCorner), return -1);
    TRY(addVert(list, &brCorner), return -1);
    TRY(addVert(list, blCorner), return -1);

    TRY(addVert(list, &tlCorner), return -1);
    TRY(addVert(list, trCorner), return -1);
    TRY(addVert(list, &brCorner), return -1);

    return 0;
}

float* buildTestVerts() {

    vertsList* list = malloc(sizeof(vertsList));

    initVertList(list, 12);

    modelVert bl; bl.x = 0.0f; bl.y = 0.0f; bl.z = 1.0f;

    modelVert tr; tr.x = 1.0f; tr.y = 1.0f; tr.z = 0.0f;

    modelVert tr2; tr2.x = 1.0f; tr2.y = 0.0f; tr2.z = -1.0f;

    modelVert bl2; bl2.x = 1.0f; bl2.y = 1.0f; bl2.z = 0.0f;

    TRY(addWallFace(list, &bl, &tr), return NULL);
    TRY(addWallFace(list, &bl2, &tr2), return NULL);

    return list->vertPositions;
}
