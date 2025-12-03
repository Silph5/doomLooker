#include <stdio.h>
#include "readWad.h"
#include "mapStruct.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Minimum of 2 arguments needed: \npath to Iwad\nlevel/map name\n");
        return -1;
    }

    char* iwadPath = argv[1];
    char* mapName = argv[2];

    doomMap* mapData = readWadToMapData(iwadPath, mapName);
    if (!mapData) {
        printf("Failed to read wad: %sz\n", iwadPath);
        return -1;
    }

}