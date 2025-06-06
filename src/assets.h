#pragma once

#include "lib.h"

enum SpriteID{
    SPRITE_DICE,
};


struct Sprite{
    IVec2 atlasOffset;
    IVec2 size;
};

Sprite get_sprite(SpriteID spriteID){
    Sprite sprite = {};

    switch(spriteID){
        case SPRITE_DICE:
            sprite.atlasOffset = {0, 0};
            sprite.size = {16, 16};
            break;
    }

    return sprite;
}
