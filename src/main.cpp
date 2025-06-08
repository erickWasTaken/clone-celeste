#include "lib.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

#include "platform.h"
#ifdef __linux__
#include "linux_platform.cpp"
const char* gameLibName = "game.so";
const char* gameLoadLibName = "game_load.so";
#endif

#include "gl_renderer.cpp"
#include "game.h"

// Code hot reload
typedef decltype(update_game) update_game_type;
static update_game_type* update_game_ptr;

#include <chrono>
#include <iostream>

double get_delta_time();
void reload_game_dll(BumpAllocator* transientStorage);

int main(){
    BumpAllocator transientStorage = make_bump_allocator(MB(50));
    BumpAllocator persistentStorage = make_bump_allocator(MB(256));

    input = (Input*)bump_alloc(&persistentStorage, sizeof(Input));
    if(!input){
        SM_ASSERT(false, "Failed to allocate memory for input!");
        return -1;
    }

    renderData = (RenderData*)bump_alloc(&persistentStorage, sizeof(RenderData));
    if(!renderData){
        SM_ASSERT(false, "Failed to allocate memory for render data!");
        return -1;
    }

    const char* str = "Linux window test, plz work X(";

    platform_create_window(1280, 720, str);
    gl_init(&transientStorage);

    while(running){
        get_delta_time();
        reload_game_dll(&transientStorage);

        update_game(renderData);
        
        gl_render(&transientStorage);
        platform_swap_buffers();

        transientStorage.used = 0;
    }

    return 0;
}

double get_delta_time(){
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    double delta = std::chrono::duration<double>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    return delta;
}

void update_game(RenderData* renderDataIn){
    update_game_ptr(renderDataIn);
}

void reload_game_dll(BumpAllocator* transientStorage){
    static void* gameDLL;
    static long long gameDLLTimestamp;

    long long currentTimestamp = get_timestamp(gameLibName);
    if(currentTimestamp > gameDLLTimestamp){
        if(gameDLL){
            bool freeResult = platform_free_dynamic_library(gameDLL);
            // SM_ASSERT(freeResult, "Failed to free: %s", gameLibName);
            gameDLL = nullptr;
            SM_TRACE("Freed %s", gameLibName);
        }

        while(!copy_file(gameLibName, gameLoadLibName, transientStorage)){
            platform_sleep(10);
        }
        SM_TRACE("Copied into %s", gameLibName, gameLoadLibName);

        gameDLL = platform_load_dynamic_library(gameLoadLibName);
        SM_ASSERT(gameDLL, "Failed to load: %s", gameLoadLibName);

        update_game_ptr = (update_game_type*)platform_load_dynamic_function(gameDLL, "update_game");
        SM_ASSERT(update_game_ptr, "Failed to load update_game function");
        gameDLLTimestamp = currentTimestamp;
    }
}
