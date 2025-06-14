#include "game.h"
#include "assets.h"
#include "input.h"
#include <iostream>

bool just_pressed(GameInputType type){
    KeyMapping mapping = gameState->keyMappings[type];
    for(int i = 0; i < mapping.keys.count; i++){
        if(input->keys[mapping.keys[i]].justPressed){
            return true;
        }
    }

    return false;
}

bool is_down(GameInputType type){
    KeyMapping mapping = gameState->keyMappings[type];
    for(int i = 0; i < mapping.keys.count; i++){
        if(input->keys[mapping.keys[i]].isDown){
            return true;
        }
    }

    return false;
}

Tile* get_tile(int x, int y){
    Tile* tile = nullptr;
    if(x >= 0 && x < WORLD_GRID.x && y >= 0 && y < WORLD_GRID.y){
        tile = &gameState->worldGrid[x][y];
    }

    // std::cout<<"x value: " << x << "\ny value: " << y << std::endl;
    return tile;
}

Tile* get_tile(IVec2 worldPos){
    int x = worldPos.x / TILESIZE;
    int y = worldPos.y / TILESIZE;

    return get_tile(x, y);
}

EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn, Input* inputIn){
    if(renderData != renderDataIn){
        renderData = renderDataIn;
        gameState = gameStateIn;
        input = inputIn;
    }

    if(!gameState->isInitialized){
        renderData->camera.size = {WORLD_WIDTH, WORLD_HEIGHT};
        renderData->camera.pos.x = 160;
        renderData->camera.pos.y = -90;
        renderData->camera.zoom = 1.0f;

        gameState->keyMappings[MOVE_UP].keys.add(KEY_W);
        gameState->keyMappings[MOVE_UP].keys.add(KEY_UP);
        gameState->keyMappings[MOVE_DOWN].keys.add(KEY_S);
        gameState->keyMappings[MOVE_DOWN].keys.add(KEY_DOWN);
        gameState->keyMappings[MOVE_LEFT].keys.add(KEY_A);
        gameState->keyMappings[MOVE_LEFT].keys.add(KEY_LEFT);
        gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_D);
        gameState->keyMappings[MOVE_RIGHT].keys.add(KEY_RIGHT);

        gameState->keyMappings[MOUSE_LEFT].keys.add(KEY_MOUSE_LEFT);
        gameState->keyMappings[MOUSE_RIGHT].keys.add(KEY_MOUSE_RIGHT);

        IVec2 tilesPosition = {32, 0};
        for(int y = 0; y < 5; y++){
            for(int x = 0; x < 4; x++){
                gameState->tileCoords.add({tilesPosition.x + x * 8, tilesPosition.y + y * 8});
            }
        }

        gameState->tileCoords.add({tilesPosition.x, tilesPosition.y + 5 * 8});

        gameState->isInitialized = true;
    }

    draw_sprite(SPRITE_CURSOR, input->mousePosWorld);

    // for(int i = 0; i < (int)(1280 / 100) + 1; i++){
    //     for(int j = 0; j < (int)(720 / 100) + 1; j++){
    //         Sprite sprite = get_sprite(SPRITE_DICE);
    //         Vec2 pos = toVec2({sprite.size.x * i, sprite.size.y * j});
    //         pos.x += gameState->playerPos.x;
    //         pos.y += gameState->playerPos.y;
    //         draw_sprite(SPRITE_DICE, pos);
    //     }
    // }
    draw_sprite(SPRITE_DICE, gameState->playerPos);

    for(int y = 0; y < WORLD_GRID.y; y++){
        for(int x = 0; x < WORLD_GRID.x; x++){
            Tile* tile = get_tile(x, y);
            if(!tile->visible)
                continue;

            Transform transform = {};
            transform.pos = {x * (float)TILESIZE, y * (float)TILESIZE};
            transform.size = {8, 8};
            transform.spriteSize = {8, 8};
            transform.atlasOffset = gameState->tileCoords[tile->neighbourMask];

            draw_quad(transform);
        }
    }

    if(is_down(MOVE_LEFT)){
        gameState->playerPos.x += 1;
    }

    if(is_down(MOVE_RIGHT)){
        gameState->playerPos.x -= 1;
    }

    if(is_down(MOVE_UP)){
        gameState->playerPos.y -= 1;
    }

    if(is_down(MOVE_DOWN)){
        gameState->playerPos.y += 1;
    }

    bool updateTiles = false;
    if(is_down(MOUSE_LEFT)){
        IVec2 temp = input->mousePosWorld;
        // std::cout<<"x value: " << temp.x << "\ny value: " << temp.y << std::endl;
        Tile* tile = get_tile(input->mousePosWorld);
        if(tile){
            tile->visible = true;
            updateTiles = true;
        }
    }

    if(is_down(MOUSE_RIGHT)){
        IVec2 temp = input->mousePosWorld;
        // std::cout<<"x value: " << temp.x << "\ny value: " << temp.y << std::endl;
        Tile* tile = get_tile(input->mousePosWorld);
        if(tile){
            tile->visible = false;
            updateTiles = true;
        }
    }

    if(updateTiles){
        int neighbourOffsets[24] = {
//          Top         Left        Right       Bottom
            0,-1,       -1,0,       1,0,        0,1,
//          Topleft     Topright    Bottomleft  Bottomright
            -1,-1,      1,-1,       -1,1,       1,1,
//          Top2        Left2       Right2      Bottom2
            0,-2,       -2,0,       2,0,        0,2
        };

        for(int y = 0; y < WORLD_GRID.y; y++){
            for(int x = 0; x < WORLD_GRID.x; x++){
                Tile* tile = get_tile(x, y);

                if(!tile->visible)
                    continue;

                tile->neighbourMask = 0;
                int neighbourCount = 0;
                int emptyNeighbour = 0;
                int extendedNeighbours = 0;

                for(int n = 0; n < 12; n++){
                    Tile* neighbour = get_tile(x + neighbourOffsets[n * 2], y + neighbourOffsets[n * 2 + 1]);

                    if(!neighbour || neighbour->visible){
                        tile->neighbourMask |= BIT(n);
                        if(n < 8)
                            neighbourCount++;
                        else{
                            extendedNeighbours++;
                        }
                    }else if(n < 8){
                        emptyNeighbour = n;
                    }

                    if(neighbourCount == 7 && emptyNeighbour >= 4){
                        tile->neighbourMask = 16 + (emptyNeighbour -4);
                        // continue;
                    }else if(neighbourCount == 8 && extendedNeighbours == 4){
                        tile->neighbourMask = 20;
                        // continue;
                    }else{
                        tile->neighbourMask = tile->neighbourMask & 0b1111;
                    }
                        
                }
            }
        }
    }
}
