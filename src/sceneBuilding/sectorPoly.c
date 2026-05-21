//
// Created by tjada on 03/05/2026.
//

#include "../../include/sectorPoly.h"

#include <stdbool.h>

#include "mapComponentStructs.h"

typedef enum {
    poly_initialised = 0,
    poly_hasDefs,
    poly_defsFullCombined,
    poly_defsReorganised,
} state;

typedef struct {
    int indexVal;
    int nextNodeI;
} IndexNode;

typedef struct {
    int headNodeI;
    int tailNodeI;
    int nodeCount;
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
    int headListI;
};

struct SectorPolyBuilder{
    LinkedListPool connectionListPool;
    NodePool indexNodePool;
    SectorPoly* sectorPolys;
    state state;
};

void initListNode (LinkedListNode* node) {
    node->list.headNodeI = -1;
    node->list.tailNodeI = -1;
    node->list.nodeCount = 0;
    node->nextListNodeI = -1;
}

void initIndexNode (IndexNode* node) {
    node->indexVal = 0;
    node->nextNodeI = -1;
}

ltc_status getNewNodeIndex (int* out, NodePool* pool) {
    if (pool->capacity <= pool->nodeCount) {
        int newCapacity = pool->capacity*2;
        printf("sectorPoly.c - expected line index node capacity exceeded\n");
        LTC_TRY(ltc_realloc((void**)&pool->nodeArr, sizeof(IndexNode) * newCapacity), "failed to expand (realloc) poly node pool");
        for (int n = pool->capacity; n < newCapacity; n++) {
            initIndexNode(&pool->nodeArr[n]);
        }
        pool->capacity = newCapacity;

    }

    *out = pool->nodeCount;
    pool->nodeCount++;
    return ltc_success;
}

ltc_status getNewLListIndex (int* out, LinkedListPool* pool) {
    if (pool->capacity <= pool->nodeCount) {
        int newCapacity = pool->capacity*2;
        printf("sectorPoly.c - expected linked list node capacity exceeded\n");
        LTC_TRY(ltc_realloc((void**)&pool->nodeArr, sizeof(LinkedListNode) * newCapacity), "failed to expand (realloc) linked list pool");
        for (int n = pool->capacity; n < newCapacity; n++) {
            initListNode(&pool->nodeArr[n]);
        }
        pool->capacity = newCapacity;
    }

    *out = pool->nodeCount;
    pool->nodeCount++;
    return ltc_success;
}

//guesses as to maximum/average needed
#define INITIAL_LISTS_PER_POLY 5
#define INITIAL_NODES_PER_POLY 8

ltc_status initSectorPoly (SectorPolyBuilder* polyBuildData, SectorPoly* poly) {
    LTC_TRY(getNewLListIndex(&poly->headListI, &polyBuildData->connectionListPool), "failed to get now LList index");
    return ltc_success;
}

ltc_status initSectorPolyBuilder(SectorPolyBuilder* polyBuildData, const DoomMap* mapData) {
    polyBuildData->state = poly_initialised;
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

    LTC_TRY(ltc_malloc((void**)&polyBuildData->sectorPolys, sizeof(SectorPoly) * mapData->sectorNum), "Failed to malloc for sector polys");
    for (int p = 0; p < mapData->sectorNum; p++) {
        LTC_TRY(initSectorPoly(polyBuildData, &polyBuildData->sectorPolys[p]), "failed to init a sector poly");
    }

    return ltc_success;
}

bool linesAreConnected (const LineDef* line1, const LineDef* line2) {
    if (line1->v1 == line2->v1 || line1->v1 == line2->v2 || line1->v2 == line2->v1 || line1->v2 == line2->v2) {
        return true;
    }
    return false;
}

ltc_status addLinedefToPoly(SectorPolyBuilder* polyBuildData, const DoomMap* mapData, const int lineI, const int sectorI) {
    if (polyBuildData->state != poly_initialised && polyBuildData->state != poly_hasDefs) {
        return ltc_fail_invalid_state;
    }
    polyBuildData->state = poly_hasDefs;
    SectorPoly* poly = &polyBuildData->sectorPolys[sectorI];
    LinkedListNode* curListNode = &polyBuildData->connectionListPool.nodeArr[poly->headListI];
    int newNodeI = 0;
    LTC_TRY(getNewNodeIndex(&newNodeI, &polyBuildData->indexNodePool), "failed to get a new node index")
    polyBuildData->indexNodePool.nodeArr[newNodeI].indexVal = lineI;

    bool reachedListEnd = false;
    while (!reachedListEnd) {
        if (curListNode->list.headNodeI == -1) {
            curListNode->list.headNodeI = newNodeI;
            curListNode->list.tailNodeI = newNodeI;
            curListNode->list.nodeCount++;
            return ltc_success;
        }

        if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curListNode->list.headNodeI].indexVal], &mapData->lineDefs[lineI])) {
            polyBuildData->indexNodePool.nodeArr[newNodeI].nextNodeI = curListNode->list.headNodeI;
            curListNode->list.headNodeI = newNodeI;
            curListNode->list.nodeCount++;
            return ltc_success;
        }

        if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curListNode->list.tailNodeI].indexVal], &mapData->lineDefs[lineI])) {
            polyBuildData->indexNodePool.nodeArr[curListNode->list.tailNodeI].nextNodeI = newNodeI;
            curListNode->list.tailNodeI = newNodeI;
            polyBuildData->indexNodePool.nodeArr[newNodeI].nextNodeI = -1;
            curListNode->list.nodeCount++;
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

void combinePolyLines(const SectorPolyBuilder* polyBuildData, const DoomMap* mapData, const int sectorI) {
    SectorPoly* poly = &polyBuildData->sectorPolys[sectorI];
    LinkedListNode* curCombiningListNode = &polyBuildData->connectionListPool.nodeArr[poly->headListI];
    bool reachedConnectionListsEnd = false;
    while (!reachedConnectionListsEnd) {

        if (curCombiningListNode->nextListNodeI == -1) {
            reachedConnectionListsEnd = true;
            continue;
        }

        LinkedListNode* aheadListNode = &polyBuildData->connectionListPool.nodeArr[curCombiningListNode->nextListNodeI];

        bool reachedSubConnectionListsEnd = false;
        LinkedListNode* checkingNode = aheadListNode;
        while (!reachedSubConnectionListsEnd) {

            if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[checkingNode->list.tailNodeI].indexVal],
                &mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.headNodeI].indexVal])) {

                polyBuildData->indexNodePool.nodeArr[checkingNode->list.tailNodeI].nextNodeI = curCombiningListNode->list.headNodeI;
                checkingNode->list.tailNodeI = curCombiningListNode->list.tailNodeI;
                curCombiningListNode->list.headNodeI = -1;
                curCombiningListNode->list.tailNodeI = -1;
                checkingNode->list.nodeCount += curCombiningListNode->list.nodeCount++;
                curCombiningListNode->list.nodeCount = 0;

                break;
            }
            if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[checkingNode->list.headNodeI].indexVal],
                &mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.tailNodeI].indexVal])) {

                polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.tailNodeI].nextNodeI = checkingNode->list.headNodeI;
                checkingNode->list.headNodeI = curCombiningListNode->list.headNodeI;
                checkingNode->list.nodeCount += curCombiningListNode->list.nodeCount++;

                curCombiningListNode->list.nodeCount = 0;
                curCombiningListNode->list.headNodeI = -1;
                curCombiningListNode->list.tailNodeI = -1;
                break;
            }

            if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[checkingNode->list.headNodeI].indexVal],
                &mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.headNodeI].indexVal])) {

                bool reachedEnd = false;
                while (!reachedEnd) {
                    if (polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.headNodeI].nextNodeI == -1) {
                        reachedEnd = true;
                    }
                    int nextCCLNodeI = polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.headNodeI].nextNodeI;
                    polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.headNodeI].nextNodeI = checkingNode->list.headNodeI;
                    checkingNode->list.headNodeI = curCombiningListNode->list.headNodeI;

                    curCombiningListNode->list.headNodeI = nextCCLNodeI;

                }
                checkingNode->list.nodeCount += curCombiningListNode->list.nodeCount++;

                curCombiningListNode->list.nodeCount = 0;
                curCombiningListNode->list.headNodeI = -1;
                curCombiningListNode->list.tailNodeI = -1;
                break;
            }

            if (linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[checkingNode->list.tailNodeI].indexVal],
                &mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[curCombiningListNode->list.tailNodeI].indexVal])) {

                //reverse linked list
                int prevNodeI = -1;
                int currentNodeI = curCombiningListNode->list.headNodeI;
                int oldHeadNodeI = curCombiningListNode->list.headNodeI;

                while (currentNodeI != -1) {
                    int nextNodeI = polyBuildData->indexNodePool.nodeArr[currentNodeI].nextNodeI;

                    polyBuildData->indexNodePool.nodeArr[currentNodeI].nextNodeI = prevNodeI;

                    prevNodeI = currentNodeI;
                    currentNodeI = nextNodeI;
                }

                curCombiningListNode->list.headNodeI = prevNodeI;
                curCombiningListNode->list.tailNodeI = oldHeadNodeI;

                polyBuildData->indexNodePool.nodeArr[checkingNode->list.tailNodeI].nextNodeI = curCombiningListNode->list.headNodeI;
                checkingNode->list.tailNodeI = curCombiningListNode->list.tailNodeI;
                checkingNode->list.nodeCount += curCombiningListNode->list.nodeCount++;
                curCombiningListNode->list.nodeCount = 0;
                curCombiningListNode->list.headNodeI = -1;
                curCombiningListNode->list.tailNodeI = -1;

                break;
            }

            if (checkingNode->nextListNodeI == -1) {
                reachedSubConnectionListsEnd = true;
            } else {
                checkingNode = &polyBuildData->connectionListPool.nodeArr[checkingNode->nextListNodeI];
            }
        }
        curCombiningListNode = aheadListNode;

    }

     LinkedListNode* discardCheckingNode = &polyBuildData->connectionListPool.nodeArr[poly->headListI];
     reachedConnectionListsEnd = false;
     while (!reachedConnectionListsEnd) {
         if (discardCheckingNode->nextListNodeI == -1) {
             reachedConnectionListsEnd = true;
         }

         if (discardCheckingNode->list.headNodeI == -1) {
             discardCheckingNode = &polyBuildData->connectionListPool.nodeArr[discardCheckingNode->nextListNodeI];
             continue;
         }

         if (!linesAreConnected(&mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[discardCheckingNode->list.headNodeI].indexVal],
             &mapData->lineDefs[polyBuildData->indexNodePool.nodeArr[discardCheckingNode->list.tailNodeI].indexVal])) {
             //printf("got one.");
             discardCheckingNode->list.headNodeI = -1;
             discardCheckingNode->list.tailNodeI = -1;
         }
         discardCheckingNode = &polyBuildData->connectionListPool.nodeArr[discardCheckingNode->nextListNodeI];
     }
}

ltc_status fullyCombinePolyLines(SectorPolyBuilder* polyBuildData, const DoomMap* mapData) {
    if (polyBuildData->state != poly_hasDefs) {
        return ltc_fail_invalid_state;
    }

    for (int s = 0; s < mapData->sectorNum; s++) {
        combinePolyLines(polyBuildData, mapData, s);
    }

    polyBuildData->state = poly_defsFullCombined;
    return ltc_success;
}

//for debug only
void printConnectionLists(const SectorPolyBuilder* polyBuildData, const int polyI) {
    SectorPoly* poly = &polyBuildData->sectorPolys[polyI];
    LinkedListNode* curListNode = &polyBuildData->connectionListPool.nodeArr[poly->headListI];

    int listCount = 0;
    bool reachedListEnd = false;
    printf("Printing sector poly lists for sector num %i\n", polyI);
    while (!reachedListEnd) {
        printf("CONNECTION LIST %i\n", listCount);

        IndexNode* curNode = &polyBuildData->indexNodePool.nodeArr[curListNode->list.headNodeI];
        if (curListNode->list.headNodeI != -1) {
            bool reachedConnectionListEnd = false;
            while (!reachedConnectionListEnd) {
                printf("%i -> ", curNode->indexVal);
                if (curNode->nextNodeI == -1) {
                    reachedConnectionListEnd = true;
                    printf("end\n h: %i t: %i c: %i\n\n", polyBuildData->indexNodePool.nodeArr[curListNode->list.headNodeI].indexVal, polyBuildData->indexNodePool.nodeArr[curListNode->list.tailNodeI].indexVal, curListNode->list.nodeCount);

                } else {
                    curNode = &polyBuildData->indexNodePool.nodeArr[curNode->nextNodeI];
                }
            }
        } else {
            printf("empty\n\n");
        }
        if (curListNode->nextListNodeI == -1) {
            reachedListEnd = true;
        } else {
            curListNode = &polyBuildData->connectionListPool.nodeArr[curListNode->nextListNodeI];
            listCount++;
        }
    }

}