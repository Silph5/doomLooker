//
// Created by tjada on 03/05/2026.
//

#include "../../include/sectorPoly.h"
#include "mapComponentStructs.h"

typedef struct {
    int16_t indexVal;
    int nextNodeI;
} IndexNode;

typedef struct {
    int headNodeI;
    int tailNodeI;
} IndexedLinkedList;

typedef struct {
    IndexedLinkedList list;
    int nextListNodeI;
} LinkedListNode;

struct NodePool {
    IndexNode* nodeArr;
    int nodeCount;
    int capacity;
};

struct LinkedListPool {
    LinkedListNode* nodeArr;
    int nodeCount;
    int capacity;
};

struct SectorPoly{
    LinkedListNode headList;
};

struct SectorPolyBuilder{
    LinkedListPool connectionListPool;
    NodePool indexNodePool;
    SectorPoly* sectorPolys;
};

//guesses as to maximum/average needed
#define INITIAL_LISTS_PER_POLY 3
#define INITIAL_NODES_PER_POLY 8

void initSectorPoly (SectorPoly* poly) {
    poly->headList.nextListNodeI = -1;
    poly->headList.list.headNodeI = -1;
    poly->headList.list.tailNodeI = -1;
}

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, DoomMap* mapData) {
    LTC_TRY(ltc_malloc((void**)&polyBuildData->sectorPolys, sizeof(SectorPoly) * mapData->sectorNum), "Failed to malloc for sector polys");
    for (int p = 0; p < mapData->sectorNum; p++) {
        initSectorPoly(&polyBuildData->sectorPolys[p]);
    }
    LTC_TRY(ltc_malloc((void**)&polyBuildData->indexNodePool.nodeArr, sizeof(IndexNode) * INITIAL_NODES_PER_POLY * mapData->sectorNum), "Failed to malloc for node pool");
    polyBuildData->indexNodePool.capacity = mapData->sectorNum * INITIAL_NODES_PER_POLY;

    LTC_TRY(ltc_malloc((void**)&polyBuildData->connectionListPool.nodeArr, sizeof(LinkedListNode) * INITIAL_LISTS_PER_POLY * mapData->sectorNum), "failed to malloc for linked list pool");
    polyBuildData->connectionListPool.capacity = mapData->sectorNum * INITIAL_LISTS_PER_POLY;

    return ltc_success;
}