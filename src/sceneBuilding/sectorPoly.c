//
// Created by tjada on 03/05/2026.
//

#include "../../include/sectorPoly.h"

#include <bemapiset.h>
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

void initListNode (LinkedListNode* node) {
    node->list.headNodeI = -1;
    node->list.tailNodeI = -1;
    node->nextListNodeI = -1;
}

void initIndexNode (IndexNode* node) {
    node->indexVal = 0;
    node->nextNodeI = -1;
}

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, const DoomMap* mapData) {
    LTC_TRY(ltc_malloc((void**)&polyBuildData->sectorPolys, sizeof(SectorPoly) * mapData->sectorNum), "Failed to malloc for sector polys");
    for (int p = 0; p < mapData->sectorNum; p++) {
        initSectorPoly(&polyBuildData->sectorPolys[p]);
    }

    LTC_TRY(ltc_malloc((void**)&polyBuildData->indexNodePool.nodeArr, sizeof(IndexNode) * INITIAL_NODES_PER_POLY * mapData->sectorNum), "Failed to malloc for node pool");
    polyBuildData->indexNodePool.capacity = mapData->sectorNum * INITIAL_NODES_PER_POLY;
    for (int n = 0; n < polyBuildData->indexNodePool.capacity; n++) {
        initIndexNode(&polyBuildData->indexNodePool.nodeArr[n]);
    }
    polyBuildData->indexNodePool.nodeCount = 0;

    LTC_TRY(ltc_malloc((void**)&polyBuildData->connectionListPool.nodeArr, sizeof(LinkedListNode) * INITIAL_LISTS_PER_POLY * mapData->sectorNum), "failed to malloc for linked list pool");
    polyBuildData->connectionListPool.capacity = mapData->sectorNum * INITIAL_LISTS_PER_POLY;
    for (int l = 0; l < polyBuildData->connectionListPool.capacity; l++) {
        initListNode(&polyBuildData->connectionListPool.nodeArr[l]);
    }
    polyBuildData->connectionListPool.nodeCount = 0;

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
    int newNodeI = 0;
    LTC_TRY(getNewNodeIndex(&newNodeI, &polyBuildData->indexNodePool), "failed to get a new node index")
    polyBuildData->indexNodePool.nodeArr[newNodeI].indexVal = lineI;

    bool reachedListEnd = false;
    while (!reachedListEnd) {
        if (curListNode->list.headNodeI == -1) {
            curListNode->list.headNodeI = newNodeI;
            curListNode->list.tailNodeI = newNodeI;
            return ltc_success;
        }

        if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curListNode->list.headNodeI].indexVal], &mapData->lineDefs[lineI])) {
            polyBuildData->indexNodePool.nodeArr[newNodeI].nextNodeI = curListNode->list.headNodeI;
            curListNode->list.headNodeI = newNodeI;
            return ltc_success;
        }

        if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curListNode->list.tailNodeI].indexVal], &mapData->lineDefs[lineI])) {
            polyBuildData->indexNodePool.nodeArr[curListNode->list.tailNodeI].nextNodeI = newNodeI;
            polyBuildData->indexNodePool.nodeArr[newNodeI].nextNodeI = -1;
            return ltc_success;
        }

        if (curListNode->nextListNodeI == -1) {
            reachedListEnd = true;
        } else {
            curListNode = &polyBuildData->connectionListPool.nodeArr[curListNode->nextListNodeI];
        }
    }

    int newListI = 0;
    LTC_TRY(getNewLListIndex(&newListI, &polyBuildData->connectionListPool), "failed to get new list index")
    curListNode->nextListNodeI = newListI;
    polyBuildData->connectionListPool.nodeArr[newListI].list.headNodeI = newNodeI;
    polyBuildData->connectionListPool.nodeArr[newListI].list.tailNodeI = newNodeI;

    return ltc_success;
}

//for debug only
void printConnectionLists(SectorPolyBuilder* polyBuildData, int polyI) {
    SectorPoly* poly = &polyBuildData->sectorPolys[polyI];
    LinkedListNode* curListNode = &poly->headList;
    int listCount = 0;
    bool reachedListEnd = false;
    printf("Printing sector poly lists for sector num %i", polyI);
    while (!reachedListEnd) {
        printf("CONNECTION LIST %i\n", listCount);

        bool reachedConnectionListEnd = false;
        IndexNode* curNode = &polyBuildData->indexNodePool.nodeArr[curListNode->list.headNodeI];
        while (!reachedConnectionListEnd) {
            printf("%i -> ", curNode->indexVal);
            if (curNode->indexVal == -1) {
                reachedConnectionListEnd = true;
                printf("\n\n");
            } else {
                curNode = &polyBuildData->indexNodePool.nodeArr[curNode->nextNodeI];
            }
        }
        if (curListNode->nextListNodeI == -1) {
            reachedListEnd = true;
        } else {
            curListNode = &polyBuildData->connectionListPool.nodeArr[curListNode->nextListNodeI];
            listCount++;
        }
    }

}