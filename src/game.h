#pragma once
#include "lib.h"
#include "render_interface.h"

constexpr int UPDATES_PER_SECOND = 60;
constexpr double UPDATE_DELAY = 1.0 / UPDATES_PER_SECOND;
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

enum AnimationState{
    ANIM_IDLE,
    ANIM_RUN,
    ANIM_JUMP,

    ANIM_COUNT
};

struct KeyMapping{
    Array<KeyCodeID, 3> keys;
};

struct Tile{
    int neighbourMask;
    bool visible;
};

struct Player{
    IVec2 pos;
    IVec2 prevPos;
    Vec2 speed;
    Vec2 solidSpeed;

    int renderOptions;

    float animTimer;
    AnimationState animationState;
    SpriteID animations[ANIM_COUNT];
};

struct Solid{
    SpriteID spriteID;
    IVec2 pos;
    IVec2 prevPos;
    Vec2 remainder;
    Vec2 speed;
    int currentKeyFrame;

    Array<IVec2, 2> keyFrames;
};

struct GameState{
    float updateTimer;
    bool isInitialized = false;

    Player player;
    Array<Solid, 2> solids;

    Array<IVec2, 21> tileCoords;
    Tile worldGrid[WORLD_GRID.x][WORLD_GRID.y];
    KeyMapping keyMappings[GAME_INPUT_COUNT];
};

static GameState* gameState;

extern "C"{
    EXPORT_FN void update_game(RenderData* renderDataIn, GameState* gameStateIn, Input* inputIn, float deltaTime);
}

