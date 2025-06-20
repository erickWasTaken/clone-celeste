#pragma once

#include "lib.h"

enum SpriteID{
    SPRITE_WHITE,
    SPRITE_DICE,
    SPRITE_CURSOR,
    SPRITE_PLAYER,
    SPRITE_SOLID,
};


struct Sprite{
    IVec2 atlasOffset;
    IVec2 size;
};

Sprite get_sprite(SpriteID spriteID){
    Sprite sprite = {};

    switch(spriteID){
        case SPRITE_WHITE:
            sprite.atlasOffset = {0, 0};
            sprite.size = {1, 1};
            break;
        case SPRITE_DICE:
            sprite.atlasOffset = {16, 0};
            sprite.size = {16, 16};
            break;
        case SPRITE_CURSOR:
            sprite.atlasOffset = {64, 0};
            sprite.size = {16, 16};
            break;
        case SPRITE_PLAYER:
            sprite.atlasOffset = {80, 0};
            sprite.size = {16, 18};
            break;
        case SPRITE_SOLID:
            sprite.atlasOffset = {96, 0};
            sprite.size = {16, 4};
            break;
    }

    return sprite;
}
