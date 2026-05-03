//
// Created by tjada on 03/12/2025.
//

#ifndef DOOMLOOKER_LEVEL_H
#define DOOMLOOKER_LEVEL_H

//forward declaration to obscure map components from includes of only mapStruct
typedef struct LineDef LineDef;
typedef struct SideDef SideDef;
typedef struct Vertex Vertex;
typedef struct Sector Sector;
typedef struct Texture Texture;

typedef struct {
    LineDef* lineDefs;
    SideDef* sideDefs;
    Vertex* vertices;
    Sector* sectors;
    Texture* textures;
    int lineDefNum;
    int sideDefNum;
    int vertexNum;
    int sectorNum;
    int textureNum;
}   DoomMap;

#endif //DOOMLOOKER_LEVEL_H