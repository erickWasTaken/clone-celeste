#include "lib.h"

#define APIENTRY
#define GL_GLEXT_PROTOTYPES
#include "glcorearb.h"

#include "platform.h"
#ifdef __linux__
#include "linux_platform.cpp"
#endif

#include "gl_renderer.cpp"

#include <chrono>
#include <iostream>

double get_delta_time();

int main(){
    get_delta_time();

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
