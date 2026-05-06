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

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, DoomMap* mapData);

#endif //DOOMLOOKER_SECTORPOLY_H