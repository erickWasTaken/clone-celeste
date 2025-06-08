#pragma once
#include "shader_header.h"
#include "assets.h"
#include <iostream>

struct RenderData{
    Array<Transform, 1000> transforms;
};

static RenderData* renderData;

void draw_sprite(SpriteID spriteID, Vec2 pos, Vec2 size){
    Sprite sprite = get_sprite(spriteID);
    if(sprite.size.x != 16){
        SM_ASSERT(false, "No sprite!");
    }
    Transform transform = {};

    transform.pos = pos;
    transform.size = size;
    transform.spriteSize = sprite.size;
    transform.atlasOffset = sprite.atlasOffset;

    renderData->transforms.add(transform);
}
