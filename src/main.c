#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtrychain.h>

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

    ltc_setOutStream(stderr);

    doomMap* mapData = NULL;
    LTC_TRY_ROOT(ltc_malloc((void**)&mapData, sizeof(doomMap)), "Failed to allocate for map data", return -1);
    LTC_TRY_ROOT(readWadsToDoomMapData(mapData, argv[1], &argv[2], argc-2), "Failed to read wad(s)", return -1);

    mapModel* model = NULL;
    LTC_TRY_ROOT(ltc_malloc((void**)&model, sizeof(mapModel)), "Failed to allocate for map model", return -1;);
    LTC_TRY_ROOT(buildMapModel(model, mapData), "Failed to build map model", return -1;);

    if (!startGame(model)) {
        printf("An error occured and the program had to exit");
        return -1;
    }
}