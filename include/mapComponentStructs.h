//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_MAPCOMPONENTSTRUCTS_H
#define DOOMLOOKER_MAPCOMPONENTSTRUCTS_H
#include <stdint.h>

typedef struct lineDef {
    uint16_t v1;
    uint16_t v2;
    uint16_t frontSideNum;
    uint16_t backSideNum;
} lineDef;

typedef struct sideDef {
    uint16_t xTexOffset;
    uint16_t yTexOffset;
    char upperTexName[8];
    char lowerTexName[8];
    char midTexName[8];
    uint16_t sectFacing;
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

typedef struct texture {
    char name[8];
    int16_t width;
    int16_t height;
    int8_t* pixels;
} texture;

#endif //DOOMLOOKER_MAPCOMPONENTSTRUCTS_H