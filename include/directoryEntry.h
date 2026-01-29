//
// Created by tjada on 28/01/2026.
//

#ifndef DOOMLOOKER_DIRECTORYENTRY_H
#define DOOMLOOKER_DIRECTORYENTRY_H
#include <stdio.h>

#include "ut_hash/uthash.h"

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8];
    int wadIndex;
} directoryEntry;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8]; //key
    int wadIndex;

    UT_hash_handle hh;
} directoryEntryHashed;

typedef struct {
    directoryEntry playPal;
    directoryEntry targetMapMarker;
} overrideEntries;


#endif //DOOMLOOKER_DIRECTORYENTRY_H