/*
 * Interface between game and graphics module
 */

// TOP

#ifndef CD_RENDER_TARGET_H
#define CD_RENDER_TARGET_H

#define Render_Get_Functions_Sig(n) void n(Render_Target *target)
typedef Render_Get_Functions_Sig(Render_Get_Functions);

#define Render_Execute_Sig(n) void n(Render_Target *target, struct Asset_Bank *bank)
typedef Render_Execute_Sig(Render_Execute_Function);

#define Init_Sig(n) void n(void *render_vars)
typedef Init_Sig(Init_Function);

#define Display_Sig(n) void n(void *render_vars)
typedef Display_Sig(Display_Function);

#define Set_Screen_Sig(n) void n(i32 w, i32 h, i32 out_w, i32 out_h)
typedef Set_Screen_Sig(Set_Screen_Function);


struct Asset_Bank_Setup{
    struct Asset_Bank *bank;
    Partition *asset_part;
    System_API *system;
};

#define Bank_Get_Functions_Sig(n) void n(Asset_Bank *bank)
typedef Bank_Get_Functions_Sig(Bank_Get_Functions);

#define Bank_Begin_Setup_Sig(n) Asset_Bank_Setup n(struct Asset_Bank *bank, Partition *trans_part, System_API *system)
typedef Bank_Begin_Setup_Sig(Begin_Setup_Function);

#define Bank_End_Setup_Sig(n) void n(Asset_Bank_Setup *setup)
typedef Bank_End_Setup_Sig(End_Setup_Function);

#define Bank_Bind_Font_Sig(n) void n(Asset_Bank_Setup *setup, char *file_name, i32 font_id)
typedef Bank_Bind_Font_Sig(Bind_Font_Function);

#define Bank_Bind_BMP_Sig(n) void n(Asset_Bank_Setup *setup, char *file_name, i32 image_id)
typedef Bank_Bind_BMP_Sig(Bind_BMP_Function);

#define Bank_Replace_BMP_Sig(n) void n(System_API *system, struct Asset_Bank *bank, Partition *part, char *file_name, i32 image_id)
typedef Bank_Replace_BMP_Sig(Replace_BMP_Function);


struct Font{
    stbtt_bakedchar chardata[128];
    f32 advance[128];
    f32 pull_up;
    i32 texture_w, texture_h;
    u32 texture_id;
    i32 id;
};

struct Image{
    f32 tex_x, tex_y;
    f32 width, height;
    u32 texture_id;
    i32 id;
};

struct Layer
{
	f32 resolution; // 1 = 'default', 2 = 2*default,  etc.
	u32 fbo_id;
	u32 color_tex, normal_tex, depth_tex; // more? -1 means not-assigned
	i32 id;
};


struct Asset_Bank{
    // Asset management
    Begin_Setup_Function *begin_setup;
    End_Setup_Function *end_setup;
    Bind_Font_Function *bind_font;
    Bind_BMP_Function *bind_bmp;
    Replace_BMP_Function *replace_bmp;
    
    // TODO(allen): Turn fonts into just a set of bitmaps.
    Font *fonts;
    i32 font_max_id;
    
    // TODO(allen): Allow for growth in the images array durring development.
    Image *images;
    i32 image_max_id;
};

struct Render_Target{
    // Immediate execution render commands
    Init_Function *init;
    Display_Function *display;
    Set_Screen_Function *set_screen;
    
    Partition push_part;
    
    vec2 dim;
    
    Transform me_to_pixel_transform;
    Transform transform;
    
    Render_Execute_Function *execute;
    
    void *memory;
    i32 memory_size;
};

enum Render_Command_Type{
    render_null,
    
    render_dbg_rectangle,
    render_dbg_parallelagram,
    render_glyph, // TODO(allen): render_string would be more awesome
    render_image,
    
    // NOTE(allen): game specific stuff
    render_lane,
    render_obstacle,
};
struct Render_Command_Header{
    Render_Command_Type type;
    i32 size;
};

struct Render_Dbg_Rectangle{
    f32_rect rect;
    vec4 color;
};

struct Render_Dbg_Parallelagram{
    Transform transform;
    vec4 color;
};

struct Render_Glyph{
    vec4 color;
    vec2 pos;
    i32 font_id;
    char c;
    // NOTE(allen): pad out to multiples of 8
    char filler[3];
};

struct Render_Image{
    Transform transform;
    i32 image_id;
    char filler[4];
};

struct Render_Lane{
    i32 lane_index;
    f32 y_position;
};

struct Render_Obstacle{
    u32 type;
    u32 seed;
    f32 x;
    i32 lane;
    char filler[4];
};

#define NextHeader(h) (Render_Command_Header*)((char*)h + h->size)

#endif

// BOTTOM

