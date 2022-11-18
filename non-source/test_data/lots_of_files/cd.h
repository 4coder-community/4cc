/*
 * Main game interface to platform layer.
 */

// TOP

#ifndef CD_H
#define CD_H

struct Memory{
    void *logical;
    u32 logical_size;
    void *transient;
    u32 transient_size;
    
#ifdef DEVELOPER
    void *developer;
    u32 developer_size;
#endif
};

struct Key_State{
    b8 down, prev_down;
};

struct Input_State{
    f32 dt;
    
    i32 mx, my;
    Key_State left_button;
    Key_State right_button;
    
    Key_State *letter;
    Key_State letter_[26];
    Key_State number[10];
    
    Key_State up;
    Key_State down;
    Key_State left;
    Key_State right;
    
    Key_State esc;
};

#ifdef DEVELOPER

#include "4tech_keycodes.h"

struct Key_Events{
    char events[32];
    i32 count;
};

struct Dev_File_Drop{
    char name[1024];
};

struct Dev_Input_State{
    Input_State input;
    
    Key_State *fkeys;
    Key_State fkeys_[12];
    
    u32 drop_count;
    Dev_File_Drop drops[128];
    f32 drop_x, drop_y;
    
    Key_Events keys;
    
};
#endif

struct Render_Target;

#define App_Step_Sig(n) void n(   \
System_API *system,               \
Memory memory,                    \
Render_Target *main_target,       \
Render_Target *dev_target,        \
struct Asset_Bank *bank,          \
Input_State *input,               \
Dev_Input_State *dev_input,       \
i32 loop_mode)

typedef App_Step_Sig(App_Step_Function);

struct App_Functions{
    App_Step_Function *step;
};

#endif

// BOTTOM


