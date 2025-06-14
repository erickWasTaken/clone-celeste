#pragma once
#include "lib.h"
#include "render_interface.h"

constexpr int WORLD_WIDTH = 320;
constexpr int WORLD_HEIGHT = 180;
constexpr int TILESIZE = 8;
constexpr IVec2 WORLD_GRID = {WORLD_WIDTH / TILESIZE, WORLD_HEIGHT / TILESIZE};

enum GameInputType{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    JUMP,

    MOUSE_LEFT,
    MOUSE_RIGHT,

    PAUSE,

    GAME_INPUT_COUNT
};

struct Tile{
    int neighbourMask;
    bool visible;
};

struct KeyMapping{
    Array<KeyCodeID, 3> keys;
};

struct GameState{
    bool isInitialized = false;
    IVec2 playerPos;

    Array<IVec2, 21> tileCoords;
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
    KeyMapping keyMappings[GAME_INPUT_COUNT];
};

static GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn, Input* inputIn);
}

