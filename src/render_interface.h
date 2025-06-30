#pragma once
#include "shader_header.h"
#include "assets.h"
#include <iostream>
#include "input.h"

struct OrthographicCamera{
    float zoom = 1.0f;
    Vec2 size;
    Vec2 pos;
};

struct RenderData{
    OrthographicCamera camera;
    Array<Transform, 1000> transforms;
};

struct DrawData{
    int frameIndex = 0;
    int renderOptions = 0;
};

static RenderData* renderData;

void draw_quad(Vec2 pos, Vec2 size){
    Transform transform = {};
    transform.pos = pos;
    transform.size = size;
    transform.atlasOffset = {0, 0};
    transform.spriteSize = {1, 1};

    renderData->transforms.add(transform);
}

void draw_quad(Transform transform){
    renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, Vec2 pos, DrawData drawData = {}){
    Sprite sprite = get_sprite(spriteID);
    
    Transform transform = {};

    transform.pos = pos - toVec2(sprite.offset);
    transform.size = toVec2(sprite.size);
    transform.spriteSize = sprite.size;
    transform.atlasOffset = sprite.atlasOffset;
    transform.atlasOffset.x += drawData.frameIndex * sprite.size.x;
    transform.renderOptions = drawData.renderOptions;

    renderData->transforms.add(transform);
}

void draw_sprite(SpriteID spriteID, IVec2 pos, DrawData drawData = {}){
    return draw_sprite(spriteID, toVec2(pos), drawData);
}

IVec2 screen_to_world_space(IVec2 coord){
    OrthographicCamera cam = renderData->camera;
    
    int xPos = (float)coord.x / (float)input->screenSize.x * cam.size.x;
    xPos += -cam.size.x / 2.0f + cam.pos.x;

    int yPos = (float)coord.y / (float)input->screenSize.y * cam.size.y;
    yPos += cam.size.y / 2.0f + cam.pos.y;

    return {xPos, yPos};
}

int animate(float* timer, int frameCount, float duration){
    int frameID = (int)((*timer / duration) * frameCount);
    frameID %= frameCount;
    return frameID;
}
