#include "game.h"
#include "assets.h"

EXPORT_FN void update_game(RenderData* renderDataIn){
    if(renderData != renderDataIn)
        renderData = renderDataIn;

    renderData->camera.size = {WORLD_WIDTH, WORLD_HEIGHT};
    renderData->camera.pos.x = 160;
    renderData->camera.pos.y = -90;

    for(int i = 0; i < (int)(1280 / 100) + 1; i++){
        for(int j = 0; j < (int)(720 / 100) + 1; j++){
            draw_sprite(SPRITE_DICE, {100.0f * i, 100.0f * j}, {100.0f, 100.0f});
            // draw_sprite(SPRITE_DICE, {100.0f, 100.0f}, {100.0f, 100.0f});
        }
    }
}
