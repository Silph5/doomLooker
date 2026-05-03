//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_READWAD_H
#define DOOMLOOKER_READWAD_H
#include "mapStruct.h"


DoomMap* readWadToMapData(const char* wadPath, char* mapName);

void freeDoomMapData(DoomMap* map);

#endif //DOOMLOOKER_READWAD_H