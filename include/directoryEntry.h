//
// Created by tjada on 28/01/2026.
//

#ifndef DOOMLOOKER_DIRECTORYENTRY_H
#define DOOMLOOKER_DIRECTORYENTRY_H
#include <stdio.h>

#include "libtrychain.h"
#include "ut_hash/uthash.h"

typedef enum {
    DOOMformat,
    UDMF
} MapFormat;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8];
    int wadIndex;
    int entryNum;
} DirectoryEntry;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8]; //key
    int wadIndex;
    int entryNum;

    UT_hash_handle hh;
} DirectoryEntryHashed;

typedef struct {

    DirectoryEntryHashed* patches;
    DirectoryEntry playPal;

    DirectoryEntry mapMarkerEntry;
    MapFormat mapFormat;

} OverrideEntries;

ltc_status readDirectoryEntry(FILE *wad, DirectoryEntry *outEntry, int *entryNum);

void skipEntries (FILE* wad, int skipCount, int* outEntryNum);

void goToEntryByNum (FILE* wad, int dirOffset, int entryNum);

#endif //DOOMLOOKER_DIRECTORYENTRY_H