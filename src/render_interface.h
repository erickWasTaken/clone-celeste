#pragma once
#include "shader_header.h"

struct RenderData{
    Array<Transform, 1000> transforms;
};

static RenderData* renderData;
