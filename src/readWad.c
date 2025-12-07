//
// Created by tjada on 03/12/2025.
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapStruct.h"
#include "mapComponentStructs.h"

typedef struct {
    char ident[5];
    int lumpsNum;
    int directoryOffset;
} header;

typedef struct {
    int lumpOffs;
    int lumpSize;
    char lumpName[8];
} directoryEntry;

void readHeader(FILE* wad, header* head) {
    fread(&head->ident, sizeof(char), 4, wad);
    head->ident[4] = '\0';

    fread(&head->lumpsNum, sizeof(int), 1, wad);
    fread(&head->directoryOffset, sizeof(int), 1, wad);
}

int findTargetMapLump(FILE* wad, directoryEntry* entry, int dirOffset, int lumpsNum, char* mapName) {
    fseek(wad, dirOffset, 0);

    for (int l = 0; l < lumpsNum; l++) {
        fread(&entry->lumpOffs, sizeof(int), 1, wad);
        fread(&entry->lumpSize, sizeof(int), 1, wad);
        fread(&entry->lumpName, sizeof(char), 8, wad);

        if (strcmp(entry->lumpName, mapName) == 0) {
            printf("lumpOffset: %i\n", entry->lumpOffs);
            printf("lumpSize: %i\n", entry->lumpSize);
            printf("lumpName: %s\n", entry->lumpName);
            return entry->lumpOffs;
        }
    }
    return -1;
}

doomMap* readWadToMapData(const char* wadPath, char* mapName) {

    FILE* wad = fopen(wadPath, "rb");
    if (!wad) {
        return NULL;
    }

    header header;
    readHeader(wad, &header);
    if (strcmp(header.ident, "IWAD") != 0 && strcmp(header.ident, "PWAD") != 0) {
        fprintf(stderr, "Incorrect file type given.\nfile ident:%s\n", header.ident);
        return NULL;
    }

    printf("Ident: %s\n", header.ident);
    printf("number of lumps: %i\n", header.lumpsNum);
    printf("directory offset: %i\n", header.directoryOffset);

    directoryEntry entry;

    int mapPtr = findTargetMapLump(wad, &entry, header.directoryOffset, header.lumpsNum, mapName);

    return NULL;
}