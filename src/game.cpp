#include "game.h"
#include "assets.h"
#include "input.h"

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

EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn, Input* inputIn){
    if(renderData != renderDataIn){
        renderData = renderDataIn;
        gameState = gameStateIn;
        input = inputIn;
    }

    if(!gameState->isInitialized){
        renderData->camera.size = {WORLD_WIDTH, WORLD_HEIGHT};
        renderData->camera.pos.x = 100;
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

        gameState->isInitialized = true;
    }


    for(int i = 0; i < (int)(1280 / 100) + 1; i++){
        for(int j = 0; j < (int)(720 / 100) + 1; j++){
            Sprite sprite = get_sprite(SPRITE_DICE);
            Vec2 pos = toVec2({sprite.size.x * i, sprite.size.y * j});
            pos.x += gameState->playerPos.x;
            pos.y += gameState->playerPos.y;
            draw_sprite(SPRITE_DICE, pos);
        }
    }

    // draw_sprite(SPRITE_DICE, gameState->playerPos);

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
}
