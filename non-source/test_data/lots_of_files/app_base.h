/*
 * 4tech reusable platform layer interface
 *   by Allen Webster
 * 20.08.2015
 *
 */

// TOP

#if FTECH_ALLOW_ASSERT
#include <assert.h>
#define _Assert assert
#else
#define _Assert(x)
#endif

#define SOFTWARE_RENDER 0
#define OPENGL_RENDER 1

#if FTECH_GL_RENDER
#define RENDER_MODE OPENGL_RENDER
#endif

#if FTECH_SOFT_RENDER
#define RENDER_MODE SOFTWARE_RENDER
#endif

#ifndef RENDER_MODE
#error render mode not set
#endif

struct Mouse_State{
    bool32 in_window;
    i32 x, y;
    bool32 l, r;
    bool32 pl, pr;
    i16 wheel;
};

struct Memory{
    void *mem;
    i32 size;
};

struct Render_Target{
    i32 w, h;
};

struct App_Step_Data{
    Memory memory;
    Render_Target render_target;
    Mouse_State mouse;
};

enum App_Mouse_Cursor{
    APP_MOUSE_CURSOR_UNSET,
    APP_MOUSE_CURSOR_IBEAM,
    APP_MOUSE_CURSOR_ARROW,
    APP_MOUSE_CURSOR_LEFTRIGHT,
    APP_MOUSE_CURSOR_UPDOWN
};

struct App_Step_Out{
    App_Mouse_Cursor mouse_cursor;
};

#define APP_STEP_SIG(name) void name(App_Step_Data *step, App_Step_Out *out)
APP_STEP_SIG(app_step);

// BOTTOM

