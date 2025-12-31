#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "readWad.h"
#include "mapStruct.h"
#include "game.h"
#include "buildModel.h"

bool mapNameFormatValid(char* mapName) {

    if (mapName[0] == 'E' && isdigit(mapName[1]) && mapName[2] == 'M' && isdigit(mapName[3])) {
        return true;
    }

    if (strncmp(mapName, "MAP", 3) == 0 && isdigit(mapName[3]) && isdigit(mapName[4])) {
        return true;
    }

    return false;
}

int main(const int argc, char* argv[]) {
    if (argc < 2) {
        printf("Minimum of 2 arguments needed: \npath to Iwad\nlevel/map name\n");
        return -1;
    }

    char* iwadPath = argv[1];
    char* mapName = argv[2];

    if (!mapNameFormatValid(mapName)) {
        printf("Given map name does not follow expected formats (MAPxx or ExMx)");
        return -1;
    }

    doomMap* mapData = readWadToMapData(iwadPath, mapName);
    if (!mapData) {
        printf("Failed to read wad: %s\n", iwadPath);
        return -1;
    }

    const mapModel* model = buildMapModel(mapData);
    if (!model) {
        printf("Failed to build map model\n");
        return -1;
    }

    if (!startGame(model)) {
        printf("An error occured and the program had to exit");
        return -1;
    }
}