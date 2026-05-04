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
    int16_t vertNum;
    int nextNodeI;
} VertNode;

typedef struct {
    int headNodeI;
    int tailNodeI;
} IndexedLinkedList;

typedef struct {
    LineNode* nodeArr;
    int nodeCount;
    int capacity;
} LineNodePool;

typedef struct {
    VertNode* nodeArr;
    int nodeCount;
    int capacity;
} VertNodePool;

typedef struct {
    IndexedLinkedList* lineGroups;
    int lineGroupCount;
    int lineGroupCapacity;
} SectorPoly;
