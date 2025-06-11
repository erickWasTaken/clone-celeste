#include "platform.h"
#include "lib.h"

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <iostream>
#include <dlfcn.h> // dll logistics
#include <unistd.h> // sleep

static constexpr int BUTTONS_KEYCODE_OFFSET = 250;

extern bool running;
static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT_ptr;
static Display* display;
static Atom wmDeleteWindow;
static Window window;

bool platform_create_window(int width, int height, const char* title){
    display = XOpenDisplay(NULL);
    window = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display),
        10, // xPos
        10, // yPos
        width,
        height,
        0, // border width
        0, // border height
        0 // background
    );

    int pixelAttribs[] = {
        GLX_DOUBLEBUFFER,   True,
        GLX_RED_SIZE,       8,
        GLX_GREEN_SIZE,     8,
        GLX_BLUE_SIZE,      8,
        None
    };

    int fbcCount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(display, DefaultScreen(display), pixelAttribs, &fbcCount);

    if(!fbc){
        SM_ASSERT(0, "glXChoseFBConfig() failed");
        return false;
    }

    PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    glXSwapIntervalEXT_ptr = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalEXT");

    int contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
        None
    };

    GLXContext rc = glXCreateContextAttribsARB(display, fbc[0], NULL, true, contextAttribs);
    if(!rc)
        std::cout<<"glx context is null!" << std::endl;


    long event_mask = 
        ExposureMask 
        | KeyPressMask | KeyReleaseMask
        | ButtonPressMask | ButtonReleaseMask;
    
    XSelectInput(display, window, event_mask);
    XMapWindow(display, window);
    glXMakeCurrent(display, window, rc);

    //
    wmDeleteWindow = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteWindow, 1);

    return true;
}

void platform_update_window(){
    Window root;
    Window child;
    int root_x;
    int root_y;
    int win_x;
    int win_y;
    unsigned int mask_return;
    XQueryPointer(display, window, &root, &child, &root_x, &root_y, &win_x, &win_y, &mask_return);

    input->mousePos = IVec2{win_x, win_y};

    input->mousePosWorld = screen_to_world_space(input->mousePos);

    while(XPending(display)){
        XEvent event;
        XNextEvent(display, &event);

        switch(event.type){
            case KeyPress:
            case KeyRelease:
            {
                bool isDown = event.type == KeyPress;
                KeyCodeID keyCode = keyCodeLookupTable[event.xkey.keycode];
                Key* key = &input->keys[keyCode];

                key->justPressed = !key->justPressed && !key->isDown && isDown;
                key->justReleased = !key->justReleased && key->isDown && !isDown;
                key->isDown = isDown;
                key->halfTransitionCount++;
                break;
            }

            case ButtonPress:
            case ButtonRelease:
            {
                bool isDown = event.type == ButtonPress;
                KeyCodeID keyCode = keyCodeLookupTable[BUTTONS_KEYCODE_OFFSET + event.xbutton.button];
                Key* key = &input->keys[keyCode];

                key->justPressed = !key->justPressed && !key->isDown && isDown;
                key->justReleased = !key->justReleased && key->isDown && !isDown;
                key->isDown = isDown;
                key->halfTransitionCount++;
                break;
            }

            default:
                break;
        }
    }
}

void platform_fill_keycode_lookup_table(){
    // Mouse
    keyCodeLookupTable[BUTTONS_KEYCODE_OFFSET + Button1] = KEY_MOUSE_LEFT;
    keyCodeLookupTable[BUTTONS_KEYCODE_OFFSET + Button2] = KEY_MOUSE_MIDDLE;
    keyCodeLookupTable[BUTTONS_KEYCODE_OFFSET + Button3] = KEY_MOUSE_RIGHT;
    
    // Keyboard A - Z
    keyCodeLookupTable[XKeysymToKeycode(display, XK_A)] = KEY_A;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_B)] = KEY_B;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_C)] = KEY_C;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_D)] = KEY_D;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_E)] = KEY_E;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F)] = KEY_F;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_G)] = KEY_G;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_H)] = KEY_H;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_I)] = KEY_I;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_J)] = KEY_J;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_K)] = KEY_K;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_L)] = KEY_L;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_M)] = KEY_M;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_N)] = KEY_N;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_O)] = KEY_O;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_P)] = KEY_P;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Q)] = KEY_Q;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_R)] = KEY_R;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_S)] = KEY_S;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_T)] = KEY_T;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_U)] = KEY_U;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_V)] = KEY_V;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_W)] = KEY_W;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_X)] = KEY_X;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Y)] = KEY_Y;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Z)] = KEY_Z;

    // Keyboard a - z
    keyCodeLookupTable[XKeysymToKeycode(display, XK_a)] = KEY_A;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_b)] = KEY_B;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_c)] = KEY_C;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_d)] = KEY_D;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_e)] = KEY_E;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_f)] = KEY_F;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_g)] = KEY_G;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_h)] = KEY_H;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_i)] = KEY_I;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_j)] = KEY_J;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_k)] = KEY_K;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_l)] = KEY_L;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_m)] = KEY_M;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_n)] = KEY_N;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_o)] = KEY_O;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_p)] = KEY_P;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_q)] = KEY_Q;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_r)] = KEY_R;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_s)] = KEY_S;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_t)] = KEY_T;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_u)] = KEY_U;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_v)] = KEY_V;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_w)] = KEY_W;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_x)] = KEY_X;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_y)] = KEY_Y;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_z)] = KEY_Z;

    // Keyboard numrow
    keyCodeLookupTable[XKeysymToKeycode(display, XK_0)] = KEY_0;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_1)] = KEY_1;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_2)] = KEY_2;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_3)] = KEY_3;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_4)] = KEY_4;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_5)] = KEY_5;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_6)] = KEY_6;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_7)] = KEY_7;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_8)] = KEY_8;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_9)] = KEY_9;

    // Keyboard symbols
    keyCodeLookupTable[XKeysymToKeycode(display, XK_space)] = KEY_SPACE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_grave)] = KEY_TICK;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_minus)] = KEY_MINUS;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_equal)] = KEY_EQUAL;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_bracketleft)] = KEY_LEFT_BRACKET;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_bracketright)] = KEY_RIGHT_BRACKET;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_semicolon)] = KEY_SEMICOLON;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_quotedbl)] = KEY_QUOTE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_comma)] = KEY_COMMA;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_period)] = KEY_PERIOD;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_slash)] = KEY_FORWARD_SLASH;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_backslash)] = KEY_BACKWARD_SLASH;

    // Keyboard 
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Tab)] = KEY_TAB;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Escape)] = KEY_ESCAPE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Pause)] = KEY_PAUSE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Up)] = KEY_UP;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Down)] = KEY_DOWN;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Left)] = KEY_LEFT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Right)] = KEY_RIGHT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_BackSpace)] = KEY_BACKSPACE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Return)] = KEY_RETURN;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Delete)] = KEY_DELETE;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Insert)] = KEY_INSERT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Home)] = KEY_HOME;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_End)] = KEY_END;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Page_Up)] = KEY_PAGE_UP;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Page_Down)] = KEY_PAGE_DOWN;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Caps_Lock)] = KEY_CAPS_LOCK;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Num_Lock)] = KEY_NUM_LOCK;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Scroll_Lock)] = KEY_SCROLL_LOCK;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Menu)] = KEY_MENU;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Shift_L)] = KEY_SHIFT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Shift_R)] = KEY_SHIFT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Control_L)] = KEY_CONTROL;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Control_R)] = KEY_CONTROL;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Alt_L)] = KEY_ALT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_Alt_R)] = KEY_ALT;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F1)] = KEY_F1;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F2)] = KEY_F2;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F3)] = KEY_F3;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F4)] = KEY_F4;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F5)] = KEY_F5;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F6)] = KEY_F6;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F7)] = KEY_F7;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F8)] = KEY_F8;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F9)] = KEY_F9;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F10)] = KEY_F10;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F11)] = KEY_F11;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_F12)] = KEY_F12;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_0)] = KEY_NUMPAD_0;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_1)] = KEY_NUMPAD_1;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_2)] = KEY_NUMPAD_2;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_3)] = KEY_NUMPAD_3;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_4)] = KEY_NUMPAD_4;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_5)] = KEY_NUMPAD_5;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_6)] = KEY_NUMPAD_6;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_7)] = KEY_NUMPAD_7;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_8)] = KEY_NUMPAD_8;
    keyCodeLookupTable[XKeysymToKeycode(display, XK_KP_9)] = KEY_NUMPAD_9;
}

void* platform_load_gl_function(char* funcName){
    void* proc = (void*)glXGetProcAddress((const GLubyte*)funcName);
    if(!proc){
        SM_ASSERT(0, "Failed to load function: %s", funcName);
    }
    return proc;
}

void platform_swap_buffers(){
    glXSwapBuffers(display, window);
}

void* platform_load_dynamic_library(const char* dll){
    char path[256] = {};
    sprintf(path, "./%s", dll);
    void* lib = dlopen(path, RTLD_NOW);
    char *errstr = dlerror();
    if(errstr != NULL){
        SM_ASSERT(false, "A dynamic library linking error occurred: (%s)\n", errstr);
    }
    SM_ASSERT(lib, "Failed to load lib: %s", dll);

    return lib;
}

void* platform_load_dynamic_function(void* dll, const char* funcName){
    void* proc = dlsym(dll, funcName);
    SM_ASSERT(proc, "Failed to load function: %s", funcName);
    return proc;
}

bool platform_free_dynamic_library(void* dll){
    SM_ASSERT(dll, "No lib provided!");
    int freeResult = dlclose(dll);
    SM_ASSERT(!freeResult, "Fialed to dlclose");
    // SM_TRACE("%s", freeResult);

    return (bool)freeResult;
}

void platform_sleep(unsigned int ms){
    sleep(ms);
}


