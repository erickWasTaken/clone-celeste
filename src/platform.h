#pragma once
#include "input.h"
#include "render_interface.h"

static bool running = true;

static KeyCodeID keyCodeLookupTable[KEY_COUNT];

bool platform_create_window(int width, int height, const char* title);
void platform_update_window();
void platform_fill_keycode_lookup_table();
void* platform_load_gl_function(char* funcName);
void platform_swap_buffers();
void* platform_load_dynamic_lib(const char* dll);
void* platform_load_dynamic_function(void* dll, const char* funcName);
bool platform_free_dynamic_library(void* dll);
void platform_sleep(unsigned int ms);
