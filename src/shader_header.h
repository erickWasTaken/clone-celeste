#ifdef ENGINE
#pragma once

#include "lib.h"
#define vec2 Vec2
#define ivec2 IVec2

#else
#define BIT(i) 1 << i

#endif
int FLIP_X = BIT(0);
int FLIP_Y = BIT(1);

struct Transform{
    vec2 pos;
    vec2 size;
    ivec2 atlasOffset;
    ivec2 spriteSize;
    int renderOptions;
    int materialIdx;
    float layer;
    int padding;
};

