//
// Created by tjada on 03/05/2026.
//

#include "../../include/sectorPoly.h"
#include "mapComponentStructs.h"

typedef struct {
    int16_t lineDefNum;
    int nextNodeI;
} LineNode;

typedef struct {
    int headNodeI;
    int tailNodeI;
} LinkedLineList;

typedef struct {
    LineNode* nodeArr;
    int nodeCount;
    int capacity;
} LineNodePool;

typedef struct {
    LinkedLineList* lineGroups;
    int lineGroupCount;
    int lineGroupCapacity;
} SectorPoly;