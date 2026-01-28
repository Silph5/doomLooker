//
// Created by tjada on 28/01/2026.
//

#ifndef DOOMLOOKER_WAD_H
#define DOOMLOOKER_WAD_H
#include <stdio.h>

typedef enum {
    DOOMformat,
    UDMF
} wadFormat;

typedef struct {
    int lumpCount;
    int dirOffset;
    FILE* stream;
    wadFormat format;
} wad;

#endif //DOOMLOOKER_WAD_H