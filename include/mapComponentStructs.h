//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_MAPCOMPONENTSTRUCTS_H
#define DOOMLOOKER_MAPCOMPONENTSTRUCTS_H
#include <stdint.h>

typedef struct lineDef {
    int16_t v1;
    int16_t v2;
    int16_t frontSideNum;
    int16_t backSideNum;
} lineDef;

typedef struct sideDef {
    int16_t xTexOffset;
    int16_t yTexOffset;
    char upperTexName[8];
    char lowerTexName[8];
    char midTexName[8];
    int16_t sectFacing;
} sideDef;

typedef struct vertex {
    int16_t x;
    int16_t y;
} vertex;

typedef struct sector {
    int16_t floorHeight;
    int16_t ceilHeight;
    char floorTex[8];
    char ceilTex[8];
    int16_t brightness;
} sector;

#endif //DOOMLOOKER_MAPCOMPONENTSTRUCTS_H