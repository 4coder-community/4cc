/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Graphics Layer.
 */

struct Image{
	u32 *data;
	i32 width, height, pitch;
    i32 img_width, img_height;
};

#pragma pack(push, 1)
struct Bitmap_Header{
	u16 type;
	u32 size;
	u16 res1, res2;
	u32 offset;
	
	u32 struct_size;
	i32 w, h;
	u16 planes;
	u16 bits_per_pixel;
	u32 compression;
	u32 image_size;
	i32 xppm, yppm;
	u32 colors_used;
	u32 colors_important;
};
#pragma pack(pop)

struct Bitmap_File{
	File file;
	
	Bitmap_Header header;
	i32 byte_pitch;
};

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct Glyph_Data{
    bool32 exists;
    real32 advance;
};

struct Font{
	Glyph_Data glyphs[128];
    stbtt_bakedchar chardata[128];
	i32 height, ascent, descent, line_skip;
    i32 advance;
    u32 tex;
    i32 tex_width, tex_height;
};

struct Blit_Rect{
	i32 x_start, y_start;
	i32 x_end, y_end;
};

struct Sub_Rect{
	real32 x_start, y_start;
	real32 x_end, y_end;
};

