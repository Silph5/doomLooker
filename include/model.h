//
// Created by tjada on 30/12/2025.
//

#ifndef DOOMLOOKER_MODEL_H
#define DOOMLOOKER_MODEL_H

typedef struct {
    size_t vertCount;
    size_t vertCapacity;
    float* vertCoords;
    float* vertUVs;
    float* vertNorms;
} mapModel;

#endif //DOOMLOOKER_MODEL_H