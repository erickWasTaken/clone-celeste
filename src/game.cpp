#include "game.h"
#include "assets.h"

EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn){
    if(renderData != renderDataIn){
        // gameState = gameStateIn;
        renderData = renderDataIn;
    }

    // if(!gameState->isInitialized){
        renderData->camera.size = {WORLD_WIDTH, WORLD_HEIGHT};
        renderData->camera.pos.x = 100;
        renderData->camera.pos.y = -90;
        renderData->camera.zoom = 3.0f;
    // }


    for(int i = 0; i < (int)(1280 / 100) + 1; i++){
        for(int j = 0; j < (int)(720 / 100) + 1; j++){
            Sprite sprite = get_sprite(SPRITE_DICE);
            draw_sprite(SPRITE_DICE, {(float)(sprite.size.x) * i, (float)(sprite.size.y) * j});
        }
    }
}
