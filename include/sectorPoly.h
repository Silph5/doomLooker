//
// Created by tjada on 03/05/2026.
//



#ifndef DOOMLOOKER_SECTORPOLY_H
#define DOOMLOOKER_SECTORPOLY_H
#include "libtrychain.h"
#include "mapStruct.h"

typedef struct LinkedListPool LinkedListPool;
typedef struct NodePool NodePool;
typedef struct SectorPoly SectorPoly;
typedef struct SectorPolyBuilder SectorPolyBuilder;

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, const DoomMap* mapData);
void printConnectionLists(const SectorPolyBuilder* polyBuildData, int polyI);
ltc_status addLinedefToPoly(SectorPolyBuilder* polyBuildData, const DoomMap* mapData, int lineI, int sectorI);
ltc_status fullyCombinePolyLines(SectorPolyBuilder* polyBuildData, const DoomMap* mapData);

#endif //DOOMLOOKER_SECTORPOLY_H