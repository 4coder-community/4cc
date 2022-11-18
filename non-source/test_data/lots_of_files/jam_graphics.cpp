/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Graphics Layer.
 */

// TOP

internal Blit_Rect
rect_from_target(Game_Render_Target *target){
    Blit_Rect result;
    result.x_start = 0;
    result.y_start = 0;
    result.x_end = target->width;
    result.y_end = target->height;
    
    return result;
}

internal void
bitmap_open_file(char *filename, Bitmap_File *bmp_file){
	File file = system_load_file(filename);
	if (file.data && file.size > sizeof(Bitmap_Header)){
		*bmp_file = {};
		bmp_file->file = file;
		bmp_file->header = *(Bitmap_Header*)file.data;
		bmp_file->byte_pitch =
			bmp_file->header.image_size / bmp_file->header.h;
		
		Assert(bmp_file->header.type == 0x4D42);
		Assert(bmp_file->header.compression == 0);
	}
}

internal i32
bitmap_data_requirement(Bitmap_File *bmp_file){
    i32 w, h;
    w = round_up_POT(bmp_file->header.w);
    h = round_up_POT(bmp_file->header.h);
    
	return 4*(w * h);
}

internal void
bitmap_fill_image(Bitmap_File *bmp_file, Image *image){
	image->width = round_up_POT(bmp_file->header.w);
    image->height = round_up_POT(bmp_file->header.h);
	image->pitch = 4*image->width;
    
    i32 true_width, true_height;
    true_width = bmp_file->header.w;
    true_height = bmp_file->header.h;
    
    image->img_width = true_width;
    image->img_height = true_height;
    
	i32 byte_per_pixel = bmp_file->header.bits_per_pixel >> 3;
	u8 *pixel_line = (u8*)bmp_file->file.data + bmp_file->header.offset;
	u8 *dest_line = (u8*)image->data;
	if (byte_per_pixel == 3){
		for (i32 y = 0; y < true_height; ++y){
			u8 *pixel = pixel_line;
			u32 *dest = (u32*)dest_line;
			for (i32 x = 0; x < true_width; ++x){
#if 0
				*dest = (0xFF << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0]);
#else
				*dest = (0xFF << 24) | (pixel[0] << 16) | (pixel[1] << 8) | (pixel[2]);
#endif
				pixel += 3;
				++dest;
			}
			pixel_line += bmp_file->byte_pitch;
			dest_line += image->pitch;
		}
	}
	else{
		for (i32 y = 0; y < true_height; ++y){
			u32 *pixel = (u32*)pixel_line;
			u32 *dest = (u32*)dest_line;
			for (i32 x = 0; x < true_width; ++x){
#if 0
				*dest =
					(*pixel & 0xFF00FF00) |
					((*pixel & 0x00FF0000) >> 16) |
					((*pixel & 0x000000FF) << 16);
#else
				*dest = *pixel;
#endif
				++pixel;
				++dest;
			}
			pixel_line += bmp_file->byte_pitch;
			dest_line += image->pitch;
		}
	}
}

internal void
bitmap_free_file(Bitmap_File *bmp_file){
	system_free_file(bmp_file->file);
}

// TODO(allen): eliminate this?
internal i32
font_init(){
    return 0;
}

inline internal i32
font_predict_size(i32 pt_size){
	return pt_size*pt_size*128;
}

// TODO(allen): switch to 0 = FAIL, 1 = SUCCESS
internal i32
font_load(char *filename, Font *font_out, i32 pt_size,
          void *font_block, i32 font_block_size){
    
    i32 result = 0;
    
    File file;
    file = system_load_file(filename);
    
    if (!file.data){
        result = 1;
    }
    
    else{
        stbtt_fontinfo font;
        if (!stbtt_InitFont(&font, (u8*)file.data, 0)){
            result = 1;
        }
        else{
            i32 ascent, descent, line_gap;
            real32 scale;
            
            stbtt_GetFontVMetrics(&font, &ascent, &descent, &line_gap);
            scale = stbtt_ScaleForPixelHeight(&font, (real32)pt_size);
            
            real32 scaled_ascent, scaled_descent, scaled_line_gap;
            
            scaled_ascent = scale*ascent;
            scaled_descent = scale*descent;
            scaled_line_gap = scale*line_gap;
            
            font_out->height = (i32)(scaled_ascent - scaled_descent + scaled_line_gap);
            font_out->ascent = (i32)(scaled_ascent);
            font_out->descent = (i32)(scaled_descent);
            font_out->line_skip = (i32)(scaled_line_gap);
            
            u8 *memory_cursor = (u8*)font_block;
            Assert(pt_size*pt_size*128 <= font_block_size);
            
            i32 tex_width, tex_height;
            tex_width = pt_size*128;
            tex_height = pt_size*2;
            
            font_out->tex_width = tex_width;
            font_out->tex_height = tex_height;
            
            if (stbtt_BakeFontBitmap((u8*)file.data, 0, (real32)pt_size,
                                     memory_cursor, tex_width, tex_height, 0, 128, font_out->chardata) <= 0){
                result = 0;
            }
            
            else{
                GLuint font_tex;
                glGenTextures(1, &font_tex);
                glBindTexture(GL_TEXTURE_2D, font_tex);
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, tex_width, tex_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, memory_cursor);
                
                font_out->tex = font_tex;
            }
            
            i32 max_advance = 0;
            
            for (u16 code_point = 0; code_point < 128; ++code_point){
                if (stbtt_FindGlyphIndex(&font, code_point) != 0){
                    font_out->glyphs[code_point].exists = 1;
                    font_out->glyphs[code_point].advance = font_out->chardata[code_point].xadvance;
                    i32 advance = Ceil(font_out->chardata[code_point].xadvance);
                    if (max_advance < advance){
                        max_advance = advance;
                    }
                }
            }
            font_out->advance = max_advance;
            
        }
        system_free_file(file);
    }
    
    return result;
}

internal void
font_draw_glyph_clipped(Game_Render_Target *target,
                        Font *font, u16 character,
                        real32 x, real32 y, u32 color,
                        Blit_Rect clip_box){
    
    if (clip_box.x_start < clip_box.x_end && clip_box.y_start < clip_box.y_end){
        real32 x_shift, y_shift;
        x_shift = font->chardata[character].xoff;
        y_shift = (real32)font->ascent;// + font->chardata[character].yoff;
        
        x += x_shift;
        y += y_shift;
        
        glScissor(clip_box.x_start,
                  target->height - clip_box.y_end,
                  clip_box.x_end - clip_box.x_start,
                  clip_box.y_end - clip_box.y_start);
        
        stbtt_aligned_quad q;
        stbtt_GetBakedQuadUnrounded(font->chardata, font->tex_width, font->tex_height, character, &x, &y, &q, 1);
        
        Vec3 c = unpack_color3(color);
        glColor3f(c.r, c.g, c.b);
        glBindTexture(GL_TEXTURE_2D, font->tex);
        glBegin(GL_QUADS);
        {
            glTexCoord2f(q.s0, q.t1); glVertex2f(q.x0, q.y1);
            glTexCoord2f(q.s1, q.t1); glVertex2f(q.x1, q.y1);
            glTexCoord2f(q.s1, q.t0); glVertex2f(q.x1, q.y0);
            glTexCoord2f(q.s0, q.t0); glVertex2f(q.x0, q.y0);
        }
        glEnd();
        
        glScissor(0, 0, target->width, target->height);
    }
}

internal void
font_draw_glyph(Game_Render_Target *target,
                Font *font, u16 character,
                real32 x, real32 y, u32 color){
    Blit_Rect screen = rect_from_target(target);
    font_draw_glyph_clipped(target, font, character, x, y, color, screen);
}
