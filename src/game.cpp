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

bool any_down(){
    for(int i = 0; i < GAME_INPUT_COUNT; i++){
        if(is_down((GameInputType)i))
            return true;
    }
    return false;
}

Tile* get_tile(int x, int y){
    Tile* tile = nullptr;
    if(x >= 0 && x < WORLD_GRID.x && y >= 0 && y < WORLD_GRID.y){
        tile = &gameState->worldGrid[x][y];
    }

    return tile;
}

Tile* get_tile(IVec2 worldPos){
    int x = worldPos.x / TILESIZE;
    int y = worldPos.y / TILESIZE;

    return get_tile(x, y);
}

IVec2 get_grid_pos(IVec2 pos){
    return{
        pos.x / TILESIZE,
        pos.y / TILESIZE
    };
}

IRect get_player_rect(){
    return
    {
        gameState->player.pos.x -4, // shifts from center to left corner
        gameState->player.pos.y -8, // ... from center to top corner
        8,
        16
    };
}

IRect get_solid_rect(Solid solid){
    Sprite solidSprite = get_sprite(solid.spriteID);
    IVec2 solidPos = solid.pos;
    solidPos = screen_to_world_space(solidPos);

    return{
        solidPos.x,
        solidPos.y,
        solidSprite.size.x,
        solidSprite.size.y
    };
}

IRect get_tile_rect(int x, int y){
    int tileX = x * TILESIZE;
    int tileY = y * TILESIZE;

    return{
        tileX,
        tileY,
        8,
        8
    };
}

bool rect_collision(IRect a, IRect b){
    return
        a.pos.x <= b.pos.x + b.size.x &&
        a.pos.x + a.size.x >= b.pos.x &&
        a.pos.y <= b.pos.y + b.size.y &&
        a.pos.y + a.size.y >= b.pos.y;
}

void draw_rect(IRect rect){
    for(int i = rect.pos.x; i <= rect.pos.x + rect.size.x; i++){
        Vec2 pos = {(float)i, (float)rect.pos.y};
        Vec2 size = {1.0f, 1.0f};
        draw_quad(pos, size);
        pos.y = (float)(rect.pos.y + rect.size.y);
        draw_quad(pos, size);
    }

    for(int i = rect.pos.y; i <= rect.pos.y + rect.size.y; i++){
        Vec2 pos = {(float)rect.pos.x, (float)i};
        Vec2 size = {1.0f, 1.0f};
        draw_quad(pos, size);
        pos.x = (float)(rect.pos.x + rect.size.x);
        draw_quad(pos, size);
    }
}

void simulate(float deltaTime){
    {
        Player& player = gameState->player;
        player.prevPos = gameState->player.pos;

        static Vec2 remainder = {};
        static bool grounded = false;
        constexpr float runSpeed = 2.0f;
        constexpr float runAcceleration = 10.0f;
        constexpr float fallSpeed = 3.0f;
        constexpr float gravity = 13.0f;
        constexpr float friction = 22.0f;
        
        if(is_down(MOVE_LEFT)){
            if(player.speed.x > 0.0f)
                player.speed.x = 0;
            player.speed.x = approach(player.speed.x, -runSpeed, runAcceleration * deltaTime);
        }

        if(is_down(MOVE_RIGHT)){
            if(player.speed.x < 0.0f)
                player.speed.x = 0;
            player.speed.x = approach(player.speed.x, runSpeed, runAcceleration * deltaTime);
        }

        if(just_pressed(MOVE_UP)){
            
        }

        if(is_down(JUMP)){
            player.pos = {};            
        }

        // Friction
        if(!is_down(MOVE_LEFT) && !is_down(MOVE_RIGHT)){
            player.speed.x = approach(player.speed.x, 0, friction * deltaTime);
        }

        player.speed.y = approach(player.speed.y, fallSpeed, gravity * deltaTime);

        {// Move X
            IRect playerRect = get_player_rect();
            remainder.x += player.speed.x;
            int moveX = round(remainder.x);
            if(moveX){
                remainder.x -= moveX;

                int moveSign = sign(moveX);
                bool hasCollided = false;

                auto movePlayerX = [&]{
                    while(moveX != 0){
                        playerRect.pos.x += moveSign;
                        for(int i = 0; i < gameState->solids.count; i++){
                            Solid& solid = gameState->solids[i];
                            IRect solidRect = get_solid_rect(solid);

                            if(rect_collision(playerRect, solidRect)){
                                player.speed.x = 0; 
                                return;
                            }
                        }

                        IVec2 playerGridPos = get_grid_pos(player.pos);
                        for(int x = playerGridPos.x -1; x <= playerGridPos.x +1; x++){
                            for(int y = playerGridPos.y -2; y <= playerGridPos.y +2; y++){
                                Tile* tile = get_tile(x, y);
                                if(!tile || !tile->visible){
                                    continue;
                                }

                                IRect tileRect = get_tile_rect(x, y);
                                if(rect_collision(playerRect, tileRect)){
                                    player.speed.x = 0;
                                    return;
                                }
                            }
                        }

                        player.pos.x += moveSign;
                        moveX -= moveSign;
                    }

                };
                movePlayerX();
            }
        }


        {// Move Y
            IRect playerRect = get_player_rect();
            remainder.y += player.speed.y;
            int moveY = round(remainder.y);

            if(moveY){
                remainder.y -= moveY;
                int moveSign = sign(moveY);

                auto movePlayerY = [&]{
                    while(moveY){
                        playerRect.pos.y += moveSign;

                        for(int i = 0; i < gameState->solids.count; i++){
                            Solid& solid = gameState->solids[i];
                            IRect solidRect = get_solid_rect(solid);

                            if(rect_collision(playerRect, solidRect)){
                                if(player.speed.y > 0.0f)
                                    grounded = true;
                                player.speed.y = 0;
                                return;
                            }
                        }
                        
                        IVec2 playerGridPos = get_grid_pos(player.pos);
                        for(int x = playerGridPos.x -1; x <= playerGridPos.x + 1; x++){
                            for(int y = playerGridPos.y -2; y <= playerGridPos.y + 2; y++){
                                Tile* tile = get_tile(x, y);
                                if(!tile || !tile->visible)
                                    continue;
                                
                                IRect tileRect = get_tile_rect(x,y);
                                if(rect_collision(playerRect, tileRect)){
                                    if(player.speed.y > 0.0f)
                                        grounded = true;

                                    player.speed.y = 0.0f;
                                    return;
                                }
                            }
                        }

                        player.pos.y += moveSign;
                        moveY -= moveSign;
                    }
                };
                movePlayerY();
            }
        }
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

EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn, Input* inputIn, float deltaTime){
    if(renderData != renderDataIn){
        renderData = renderDataIn;
        gameState = gameStateIn;
        input = inputIn;
    }

    if(!gameState->isInitialized){

        {
            renderData->camera.size = {WORLD_WIDTH, WORLD_HEIGHT};
            renderData->camera.pos.x = 160;
            renderData->camera.pos.y = -90;
            renderData->camera.zoom = 1.0f;
        }

        {
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

            gameState->keyMappings[JUMP].keys.add(KEY_SPACE);
        }

        {
            Solid solid = {};
            solid.spriteID = SPRITE_SOLID;
            solid.pos = screen_to_world_space({640, 480});
            solid.speed = {.0f, .0f};
            gameState->solids.add(solid);
        }

        IVec2 tilesPosition = {32, 0};
        for(int y = 0; y < 5; y++){
            for(int x = 0; x < 4; x++){
                gameState->tileCoords.add({tilesPosition.x + x * 8, tilesPosition.y + y * 8});
            }
        }

        gameState->tileCoords.add({tilesPosition.x, tilesPosition.y + 5 * 8});

        gameState->isInitialized = true;
    }

    gameState->updateTimer += deltaTime;
    while(gameState->updateTimer >= UPDATE_DELAY){
        gameState->updateTimer -= UPDATE_DELAY;

        simulate(deltaTime);
    }

    float currentStep = (float)(gameState->updateTimer / UPDATE_DELAY);
    draw_sprite(SPRITE_CURSOR, input->mousePosWorld);

    // draw_rect(get_player_rect());
    
    IVec2 quadPos = {
        round_to_int(gameState->player.pos.x),
        round_to_int(gameState->player.pos.y)
    };

    // draw_quad(toVec2(get_grid_pos(quadPos) * TILESIZE), {(float) TILESIZE, (float)TILESIZE});
    { // render player
        Player& player = gameState->player;
        IVec2 playerPos = lerp(player.prevPos, player.pos, currentStep);
        draw_sprite(SPRITE_PLAYER, playerPos);
    }

    {// render solids
        Solid& solid = gameState->solids[0];
        draw_sprite(solid.spriteID, screen_to_world_space(solid.pos));
    }


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

            // draw_rect(get_tile_rect(x, y));
            draw_quad(transform);
        }
    }


}
