//
// Created by tjada on 03/05/2026.
//

#include "../../include/sectorPoly.h"

#include <stdbool.h>

#include "mapComponentStructs.h"

typedef struct {
    int indexVal;
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

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, const DoomMap* mapData) {
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

ltc_status getNewNodeIndex (int* out, NodePool* pool) {
    if (pool->capacity <= pool->nodeCount) {
        pool->capacity *= 2;
        LTC_TRY(ltc_realloc((void**)&pool->nodeArr, sizeof(IndexNode) * pool->capacity), "failed to expand (realloc) poly node pool")
    }

    *out = pool->nodeCount;
    pool->nodeCount++;
    return ltc_success;
}

ltc_status getNewLListIndex (int* out, LinkedListPool* pool) {
    if (pool->capacity <= pool->nodeCount) {
        pool->capacity *= 2;
        LTC_TRY(ltc_realloc((void**)&pool->nodeArr, sizeof(IndexNode) * pool->capacity), "failed to expand (realloc) linked list pool")
    }

    *out = pool->nodeCount;
    pool->nodeCount++;
    return ltc_success;
}

bool linesAreConnected (LineDef* line1, LineDef* line2) {
    if (line1->v1 == line2->v1 || line1->v1 == line2->v2 || line1->v2 == line2->v1 || line1->v2 == line2->v2) {
        return true;
    }
    return false;
}

ltc_status addLinedefToPoly(SectorPolyBuilder* polyBuildData, DoomMap* mapData, int lineI, int sectorI) {
    SectorPoly* poly = &polyBuildData->sectorPolys[sectorI];
    LinkedListNode* curListNode = &poly->headList;
    if (curListNode->list.headNodeI == -1) {
        int newI = 0;
        LTC_TRY(getNewNodeIndex(&newI, &polyBuildData->indexNodePool), "failed to get a new node index")
        curListNode->list.headNodeI = newI;
        curListNode->list.tailNodeI = newI;
        polyBuildData->indexNodePool.nodeArr[newI].indexVal = lineI;
        return ltc_success;
    }

    return ltc_success;
}