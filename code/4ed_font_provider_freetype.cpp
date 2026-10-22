/*
* Mr. 4th Dimention - Allen Webster
*
* 18.07.2017
*
* Freetype implementation of the font provider interface.
*
*/

// TOP

// NOTE(allen): Thanks to insofaras.  This is copy-pasted from some work he originally did to get free type working on Linux.

#undef internal
#include <ft2build.h>
#include FT_FREETYPE_H
#define internal static

internal u32
ft__load_flags(b32 use_hinting){
    u32 ft_flags = FT_LOAD_RENDER;
    if (use_hinting){
        // NOTE(inso): FT_LOAD_TARGET_LIGHT does hinting only vertically, which looks nicer imo
        // maybe it could be exposed as an option for hinting, instead of just on/off.
        ft_flags |= (FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
    }
    else{
        ft_flags |= (FT_LOAD_NO_AUTOHINT | FT_LOAD_NO_HINTING);
    }
    return(ft_flags);
}

struct Bad_Rect_Pack{
    Vec2_i32 max_dim;
    Vec3_i32 dim;
    Vec3_i32 p;
    i32 current_line_h;
};

// NOTE(long): Similar to mrmxier's ft__bad_rect_pack_next, but simplified into one function
// It could be inlined into ft__font_make_face, but I’m keeping it generic for future reuse
internal Vec3_i32 BRP_Pack(Bad_Rect_Pack* pack, Vec2_i32 dim)
{
    Vec3_i32 p = {};
    if (dim.x <= pack->max_dim.x && dim.y <= pack->max_dim.y)
    {
        if (pack->p.x + dim.x > pack->max_dim.x)
        {
            pack->p.x = 0;
            pack->p.y += pack->current_line_h;
            pack->current_line_h = 0;
        }
        
        if (pack->p.y + dim.y > pack->max_dim.y)
        {
            pack->p = V3i32(0, 0, pack->p.z+1);
            pack->current_line_h = 0;
        }
        
        p = pack->p;
        pack->p.x += dim.x;
        
        pack->current_line_h = Max(pack->current_line_h, dim.y);
        pack->dim.x = Max(pack->dim.x, pack->p.x);
        pack->dim.y = Max(pack->dim.y, pack->p.y + pack->current_line_h);
        pack->dim.z = pack->p.z + 1;
    }
    return p;
}

// NOTE(long): 4coder doesn't provide a straightforward way to load a file
// The closest option is dump_file (4coder_helper.cpp), but it relies on the CRT
// On top of that, it can't be used here because of the F****** STUPID include order
internal String8 SysOpenFile(Arena* arena, char* name)
{
    String8 result = {};
    Plat_Handle handle = {};
    
    if (system_load_handle(arena, name, &handle))
    {
        File_Attributes attributes = system_load_attributes(handle);
        u8* data = push_array(arena, u8, attributes.size);
        if (data)
        {
            if (system_load_file(handle, (char*)data, (u32)attributes.size))
            {
                system_load_close(handle);
                result = SCu8(data, attributes.size);
            }
            else pop_array(arena, u8, attributes.size);
        }
    }
    
    return result;
}

internal Face* ft__font_make_face(Arena* arena, Face_Description* description, f32 scale_factor)
{
    Arena scratch = make_arena_system();
    Temp_Memory temp = begin_temp(arena);
    String8 file_name = push_string_copy(arena, description->font.file_name);
    b32 error = 0;
    
    //- NOTE(long): FreeType Init
    FT_Library ft = {};
    FT_Error init_error = FT_Init_FreeType(&ft);
    error = init_error;
    
    FT_Face ft_face = {};
    if (!error)
    {
        FT_Open_Args args = {0};
        args.flags = FT_OPEN_MEMORY;
        
        String8 data = SysOpenFile(&scratch, (char*)file_name.str);
        args.memory_base = data.str;
        args.memory_size = (FT_Long)data.size;
        error = FT_Open_Face(ft, &args, 0, &ft_face);
    }
    
    //- NOTE(long): Sizing
    if (!error)
    {
        FT_F26Dot6 char_size = description->parameters.pt_size << 6;
        FT_UInt dpi = (FT_UInt)(scale_factor * 96.f);
        error = FT_Set_Char_Size(ft_face, char_size, char_size, dpi, dpi);
    }
    
    //- NOTE(long): Face Init
    Face* face = 0;
    if (!error)
    {
        face = push_array_zero(arena, Face, 1);
        face->description.font.file_name = file_name;
        face->description.parameters = description->parameters;
    }
    
    //- NOTE(long): Metrics Calculation
    Face_Metrics* met = &face->metrics;
    if (!error)
    {
        met->text_height =  f32_ceil32(ft_face->size->metrics.height     /64.f);
        met->ascent      =  f32_ceil32(ft_face->size->metrics.ascender   /64.f);
        met->descent     = f32_floor32(ft_face->size->metrics.descender  /64.f);
        met->max_advance =  f32_ceil32(ft_face->size->metrics.max_advance/64.f);
        met->line_skip   = clamp_bot(1.f, met->text_height - (met->ascent - met->descent));
        met->line_height = met->text_height + met->line_skip;
        
        f32 real_over_notional = met->line_height/(f32)ft_face->height;
        f32 relative_center = -1.f*real_over_notional*ft_face->underline_position;
        f32 relative_thickness = real_over_notional*ft_face->underline_thickness;
        f32 center    = f32_floor32(met->ascent + relative_center);
        f32 thickness = clamp_bot(1.f, relative_thickness);
        met->underline_yoff1 = center - thickness*0.5f;
        met->underline_yoff2 = center + thickness*0.5f;
    }
    
    struct Bitmap
    {
        FT_UInt glyph_index;
        Vec2_i32 dim;
        u8* data;
    };
    Bitmap* bitmaps = 0;
    Face_Advance_Map* advance_map = &face->advance_map;
    
    //- NOTE(long): Codepoint -> Glyph
    u16 index_count = 0;
    if (!error)
    {
        Codepoint_Index_Map map = {};
        map.zero_index = max_u16;
        map.table = make_table_u32_u16(arena->base_allocator, ft_face->num_glyphs * 2);
        // TODO(long): Small memory waste: the actual size should be index_count
        bitmaps = push_array_zero(&scratch, Bitmap, ft_face->num_glyphs);
        
        for (FT_UInt index = 0, codepoint = FT_Get_First_Char(ft_face, &index); index != 0;
             codepoint = FT_Get_Next_Char(ft_face, codepoint, &index))
        {
            u16 val = index_count++;
            bitmaps[val].glyph_index = index;
            
            // NOTE(long): Insert the current index rather than the glyph index
            if (codepoint == 0)
            {
                map.has_zero_index = 1;
                map.zero_index = val;
            }
            else table_insert(&map.table, codepoint, val);
        }
        
        advance_map->codepoint_to_index = map;
        advance_map->index_count = index_count;
        advance_map->advance = push_array(arena, f32, index_count);
        face->bounds = push_array(arena, Glyph_Bounds, index_count);
    }
    
    //- NOTE(long): Glyph Rasterization
    if (!error)
    {
        // NOTE(long): We can't use FT_LOAD_TARGET_MONO because it requires the raster1 renderer
        // Not only that, the resulting bitmap is 8 pixels per byte, which makes copying non-trivial
        u32 load_flags = ft__load_flags(description->parameters.hinting);
        b32 is_aa_1bit = description->parameters.aa_mode == FaceAntialiasingMode_1BitMono;
        
        for (i32 i = 0; i < index_count; ++i)
        {
            // NOTE(long): This is a per-glyph error; if fails, just skip to the next glyph
            if (FT_Load_Glyph(ft_face, bitmaps[i].glyph_index, load_flags))
                continue;
            
            FT_GlyphSlot ft_glyph = ft_face->glyph;
            Vec2_i32 dim = V2i32(ft_glyph->bitmap.width, ft_glyph->bitmap.rows);
            bitmaps[i].dim = dim;
            bitmaps[i].data = push_array(&scratch, u8, dim.x*dim.y);
            
            advance_map->advance[i] = f32_ceil32(ft_glyph->advance.x/64.0f);
            Glyph_Bounds* bounds = face->bounds + i;
            bounds->xy_off.x0 = (f32)(ft_face->glyph->bitmap_left);
            bounds->xy_off.y0 = (f32)(met->ascent - ft_face->glyph->bitmap_top);
            bounds->xy_off.x1 = (f32)(bounds->xy_off.x0 + dim.x);
            bounds->xy_off.y1 = (f32)(bounds->xy_off.y0 + dim.y);
            
            switch (ft_glyph->bitmap.pixel_mode)
            {
                case FT_PIXEL_MODE_GRAY:
                {
                    u8* src_line = ft_glyph->bitmap.buffer;
                    if (ft_glyph->bitmap.pitch < 0)
                        src_line = ft_glyph->bitmap.buffer + (-ft_glyph->bitmap.pitch)*(dim.y - 1);
                    
                    u8* dst = bitmaps[i].data;
                    for (i32 y = 0; y < dim.y; ++y)
                    {
                        u8* src = src_line;
                        for (i32 x = 0; x < dim.x; ++x)
                        {
                            u8 pixel = *src++;
                            if (is_aa_1bit)
                                pixel = (pixel >= 128) * 255;
                            *dst++ = pixel;
                        }
                        src_line += ft_glyph->bitmap.pitch;
                    }
                } break;
                
                default: NotImplemented; break;
            }
        }
    }
    
    //- NOTE(long): Clean up the library; from this point forward, FreeType is no longer needed
    if (!init_error)
        FT_Done_FreeType(ft);
    
    //- NOTE(long): Finish metrics calculation after all glyph advances are known
    if (!error)
    {
        met->space_advance = font_get_glyph_advance(advance_map, met, ' ', 0);
        met->decimal_digit_advance = font_get_max_glyph_advance_range(advance_map, met, '0', '9', 0);
        met->hex_digit_advance = font_get_max_glyph_advance_range(advance_map, met, 'A', 'F', 0);
        met->hex_digit_advance = Max(met->hex_digit_advance, met->decimal_digit_advance);
        
        met->byte_sub_advances[0] = font_get_glyph_advance(advance_map, met, '\\', 0);
        met->byte_sub_advances[1] = met->hex_digit_advance;
        met->byte_sub_advances[2] = met->hex_digit_advance;
        met->byte_advance = met->byte_sub_advances[0] + met->byte_sub_advances[1] + met->byte_sub_advances[2];
        
        met->normal_lowercase_advance = font_get_average_glyph_advance_range(advance_map, met, 'a', 'z', 0);
        met->normal_uppercase_advance = font_get_average_glyph_advance_range(advance_map, met, 'A', 'Z', 0);
        met->normal_advance = (26*met->normal_lowercase_advance +
                               26*met->normal_uppercase_advance +
                               10*met->decimal_digit_advance)/62.f;
    }
    
    //- NOTE(long): Texture Packing
    if (!error)
    {
        Bad_Rect_Pack pack = {V2i32(256, 256)};
        for (i32 i = 0; i < index_count; ++i)
        {
            Vec2_i32 dim = bitmaps[i].dim;
            Vec3_i32 p = BRP_Pack(&pack, dim);
            face->bounds[i].uv = Rf32((f32)p.x, (f32)p.y, (f32)dim.x, (f32)dim.y);
            face->bounds[i].w = (f32)p.z;
        }
        
        Texture_Kind texture_kind = TextureKind_Mono;
        u32 texture = graphics_get_texture(pack.dim, face->texture_kind);
        
        error = texture == 0;
        if (!error)
        {
            face->texture_kind = texture_kind;
            face->texture = texture;
            Vec3_f32 texture_dim = face->texture_dim = V3f32(pack.dim);
            
            //- NOTE(long): Upload to GPU
            for (i32 i = 0; i < index_count; ++i)
            {
                Glyph_Bounds* bounds = face->bounds + i; 
                Bitmap bitmap = bitmaps[i];
                Vec3_i32 pos = V3i32((i32)bounds->uv.x0, (i32)bounds->uv.y0, (i32)bounds->w);
                Vec3_i32 dim = V3i32(bitmap.dim.x, bitmap.dim.y, 1);
                graphics_fill_texture(face->texture_kind, face->texture, pos, dim, bitmap.data);
                
                bounds->uv.x1 = (bounds->uv.x0 + bounds->uv.x1)/texture_dim.x;
                bounds->uv.y1 = (bounds->uv.y0 + bounds->uv.y1)/texture_dim.y;
                bounds->uv.x0 = (bounds->uv.x0 +             0)/texture_dim.x;
                bounds->uv.y0 = (bounds->uv.y0 +             0)/texture_dim.y;
            }
        }
    }
    
    //- NOTE(long): Clear scratch arena (the bitmaps array is not needed anymore)
    linalloc_clear(&scratch);
    
    //- NOTE(long): If any error occurs, restore the arena to its previous state
    if (error)
    {
        face = 0;
        end_temp(temp);
    }
    
    return face;
}

// BOTTOM

