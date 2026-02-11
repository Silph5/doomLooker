#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wadComposite.h"
#include "mapStruct.h"
#include "game.h"
#include "buildModel.h"


//not currently functioning due to mid-progress refactor

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

    if (argc < 3) {
        printf("Minimum of 2 arguments needed: map name, path to iwad, (any pwads optional)");
        return -1;
    }

    if (!mapNameFormatValid(argv[1])) {
        printf("Given map name does not follow expected formats (MAPxx or ExMx)");
        return -1;
    }

    doomMap* mapData = readWadsToDoomMapData(argv[1], &argv[2], argc-2);
    if (!mapData) {
        printf("Failed to read wad");
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