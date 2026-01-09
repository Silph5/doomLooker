//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_READWAD_H
#define DOOMLOOKER_READWAD_H
#include "mapStruct.h"

doomMap* readWadToMapData(const char* wadPath, char* mapName);

void freeDoomMapData(doomMap* map);

#endif //DOOMLOOKER_READWAD_H