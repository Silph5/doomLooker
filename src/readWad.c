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

void readHeader(FILE* wad, header* head) {
    fread(&head->ident, sizeof(char), 4, wad);
    head->ident[4] = '\0';

    fread(&head->lumpsNum, sizeof(int), 1, wad);
    fread(&head->directoryOffset, sizeof(int), 1, wad);
}

doomMap* readWadToMapData(const char* wadPath, char* mapName) {

    FILE* wad = fopen(wadPath, "rb");
    if (!wad) {
        return NULL;
    }

    header header;
    readHeader(wad, &header);
    if (strcmp(header.ident, "IWAD") != 0 && strcmp(header.ident, "PWAD") != 0) {
        fprintf(stderr, "Given file is not a WAD.\n");
        return NULL;
    }

    printf("Ident: %s\n", header.ident);
    printf("number of lumps: %i\n", header.lumpsNum);
    printf("directory offset: %i\n", header.directoryOffset);

    return NULL;
}