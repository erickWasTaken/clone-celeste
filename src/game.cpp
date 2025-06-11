#include "game.h"
#include "assets.h"
#include "input.h"

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
    }


    // for(int i = 0; i < (int)(1280 / 100) + 1; i++){
    //     for(int j = 0; j < (int)(720 / 100) + 1; j++){
    //         Sprite sprite = get_sprite(SPRITE_DICE);
    //         draw_sprite(SPRITE_DICE, toVec2({(sprite.size.x) * i, (sprite.size.y) * j}));
    //     }
    // }

    draw_sprite(SPRITE_DICE, gameState->playerPos);

    if(key_is_down(KEY_A)){
        gameState->playerPos.x += 1;
    }

    if(key_is_down(KEY_D)){
        gameState->playerPos.x -= 1;
    }

    if(key_is_down(KEY_S)){
        gameState->playerPos.y -= 1;
    }

    if(key_is_down(KEY_W)){
        gameState->playerPos.y += 1;
    }
}
