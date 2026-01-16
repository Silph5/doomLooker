//
// Created by tjada on 16/01/2026.
//

#ifndef DOOMLOOKER_TEXTURE_H
#define DOOMLOOKER_TEXTURE_H
#include <stdint.h>

typedef struct texture {
    char name[8];
    int16_t width;
    int16_t height;
    uint32_t* pixels;
} texture;

#endif //DOOMLOOKER_TEXTURE_H