//
// Created by tjada on 28/12/2025.
//

#ifndef DOOMLOOKER_BUILDMODEL_H
#define DOOMLOOKER_BUILDMODEL_H
#include <stddef.h>

typedef struct {
    size_t size;
    size_t capacity;
    float* vertPositions;
} vertsList;

vertsList* buildTestVerts();

#endif //DOOMLOOKER_BUILDMODEL_H