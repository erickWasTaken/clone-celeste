#pragma once
#include "lib.h"
#include "render_interface.h"

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
constexpr IVec2 WORLD_GRID = {WORLD_WIDTH / TILESIZE, WORLD_HEIGHT / TILESIZE};

extern "C"{
    EXPORT_FN void update_game(RenderData* renderDataIn);
}

