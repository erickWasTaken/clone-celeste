#pragma once

#include "lib.h"

enum SpriteID{
    SPRITE_WHITE,
    SPRITE_DICE,
    SPRITE_CURSOR,
    SPRITE_SOLID,
    PLAYER_IDLE,
    PLAYER_RUN,
    PLAYER_JUMP,
};


struct Sprite{
    IVec2 atlasOffset;
    IVec2 size;
    IVec2 offset;
    int frameCount = 1;
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
        case SPRITE_SOLID:
            sprite.atlasOffset = {80, 0};
            sprite.size = {16, 4};
            break;
        case PLAYER_IDLE:
            sprite.atlasOffset = {96, 0};
            sprite.size = {16, 18};
            sprite.offset = {8, 8};
            break;
        case PLAYER_RUN:
            sprite.atlasOffset = {112, 0};
            sprite.size = {16, 18};
            sprite.offset = {8, 8};
            sprite.frameCount = 12;
            break;
        case PLAYER_JUMP:
            sprite.atlasOffset = {208, 0};
            sprite.size = {16, 18};
            sprite.offset = {8, 8};
            sprite.frameCount = 2;
            break;
    }

    return sprite;
}
