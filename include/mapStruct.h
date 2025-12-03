//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_LEVEL_H
#define DOOMLOOKER_LEVEL_H

//forward declaration to obscure map components from includes of only mapStruct
typedef struct lineDef lineDef;
typedef struct sideDef sideDef;
typedef struct vertex vertex;
typedef struct sector sector;

typedef struct {
    lineDef* lineDefs;
    sideDef* sideDefs;
    vertex* vertices;
    sector* sectors;
    int lineDefNum;
    int sideDefNum;
    int vertexNum;
    int sectorNum;
} doomMap;

#endif //DOOMLOOKER_LEVEL_H