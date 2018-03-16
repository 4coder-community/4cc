/*
 * General render target operations.
 */

// TOP

Bank_Begin_Setup_Sig(begin_setup){
    Asset_Bank_Setup setup = {0};
    setup.bank = bank;
    setup.trans_part = trans_part;
    setup.system = system;
    return(setup);
}

Bank_End_Setup_Sig(end_setup){}

Bank_Bind_Font_Sig(bind_font){
    System_API *system = setup->system;
    Asset_Bank *bank = setup->bank;
    Partition *part = setup->trans_part;
    Font *font = 0;
    i32 i = 0;
    
    font = get_font(bank, font_id);
    if (font){
		// TODO(tyler): Implementation of distance field text rendering for size-agnostic fonts
        // TODO(allen): Pick a good size for the texture based
        // on the size we want the font to be.
        // TODO(allen): Background asset streaming.
        Temp_Memory temp = begin_temp(part);
        u8 *temp_bitmap = push_array(part, u8, 512*512);
        File_Dump dump = system->DBG_dump_begin(file_name);
        u8 *ttf_buffer = push_array(part, u8, dump.size);
        
        if (system->DBG_dump_end(dump, ttf_buffer)){
            stbtt_fontinfo fontinfo;
            i32 ascent;
            f32 scale;
            f32 pixel_height = 16.f;
            
            stbtt_InitFont(&fontinfo, ttf_buffer, 0);
            scale = stbtt_ScaleForPixelHeight(&fontinfo, pixel_height);
            stbtt_GetFontVMetrics(&fontinfo, &ascent, 0, 0);
            
            font->texture_w = 512;
            font->texture_h = 512;
            font->pull_up = ascent*scale;
            font->id = font_id;
            
            stbtt_BakeFontBitmap(ttf_buffer, 0, pixel_height,
                                 temp_bitmap, font->texture_w, font->texture_h,
                                 0, 128, font->chardata, part);
            
            for (i = 0; i < 128; ++i){
                font->advance[i] = font->chardata[i].xadvance;
            }
            
            glGenTextures(1, &font->texture_id);
            glBindTexture(GL_TEXTURE_2D, font->texture_id);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512,
                         0, GL_ALPHA, GL_UNSIGNED_BYTE,
                         temp_bitmap);
            
#ifdef DEVELOPER
            {
                GLenum error = glGetError();
                GLenum copy = error;
                AllowLocal(copy);
            }
#endif
        }
        
        end_temp(temp);
    }
}

struct Bmp_Data{
	i32 w, h, byte_pitch;
	u16 bpp;
	u8 *data;
};

struct Bmp_File_Info{
	u16 type;
	u32 file_size;
	u16 res1, res2;
	u32 off_bits;

	u32 struct_size;
	i32 w, h;
	u16 planes;
	u16 bpp;
	u32 compression;
	u32 image_size;
	i32 xppm;
	i32 yppm;
	u32 colors_used;
	u32 colors_important;
};

b32
bmp_read(u8 *src_buffer, i32 size, Bmp_Data *bmp, Bmp_File_Info *bmp_info){
    u8 *ptr = src_buffer;
    b32 result = 0;
    
    bmp_info->type = PtrRead(ptr, u16);
    bmp_info->file_size = PtrRead(ptr, u32);
    bmp_info->res1 = PtrRead(ptr, u16);
    bmp_info->res2 = PtrRead(ptr, u16);
    bmp_info->off_bits = PtrRead(ptr, u32);
    
    if (bmp_info->type == 0x4D42){
        bmp_info->struct_size = PtrRead(ptr, u32);
        bmp_info->w = PtrRead(ptr, i32);
        bmp_info->h = PtrRead(ptr, i32);
        bmp_info->planes = PtrRead(ptr, u16);
        bmp_info->bpp = PtrRead(ptr, u16);
        bmp_info->compression = PtrRead(ptr, u32);
        bmp_info->image_size = PtrRead(ptr, u32);
        bmp_info->xppm = PtrRead(ptr, i32);
        bmp_info->yppm = PtrRead(ptr, i32);
        bmp_info->colors_used = PtrRead(ptr, u32);
        bmp_info->colors_important = PtrRead(ptr, u32);
        
        bmp->w = bmp_info->w;
        bmp->h = bmp_info->h;
        bmp->byte_pitch = bmp_info->image_size / bmp->h;
        bmp->bpp = bmp_info->bpp;
        
        if (bmp->byte_pitch == bmp->w*4){
            bmp->data = src_buffer + bmp_info->off_bits;
            result = 1;
        }
    }
    
    return(result);
}

struct POT_Texture{
    void *data;
    u32 w, h;
};

u32
round_up_pot(u32 x){
    --x;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    ++x;
    return(x);
}

POT_Texture
pot_bmp_texture(Partition *part, void *data, i32 w, i32 h, i32 pitch){
    POT_Texture result = {0};
    char *src_line = 0;
    char *dst_line = 0;
    char *pixel = 0;
    char r,g,b,a;
    i32 dst_pitch = 0;
    i32 y = 0, x = 0;
    
    result.w = round_up_pot((u32)w);
    result.h = round_up_pot((u32)h);
    dst_pitch = result.w*4;
    align_partition(part, 4);
    result.data = push_array(part, char, result.h*dst_pitch);
    memset(result.data, 0, result.h*dst_pitch);
    
    src_line = (char*)data;
    dst_line = (char*)result.data;
    
    for (y = 0; y < h; ++y){
        memcpy(dst_line, src_line, pitch);
        
        pixel = dst_line;
        for (x = 0; x < w; ++x){
            b = pixel[0];
            g = pixel[1];
            r = pixel[2];
            a = pixel[3];
            
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = a;
            
            pixel += 4;
        }
        
        src_line += pitch;
        dst_line += dst_pitch;
    }
    
    return(result);
}

u32
load_bmp(System_API *system, Partition *part, Image *image, i32 image_id, char *file_name){
    u32 prev_tex_id = 0;
    if (image){
        // TODO(allen): Background asset streaming.
        Bmp_Data bmp;
        Bmp_File_Info bmp_info;
        
        Temp_Memory temp = begin_temp(part);
        File_Dump dump = system->DBG_dump_begin(file_name);
        u8 *bmp_buffer = push_array(part, u8, dump.size);
        if (system->DBG_dump_end(dump, bmp_buffer)){
            if (bmp_read(bmp_buffer, dump.size, &bmp, &bmp_info)){
                POT_Texture texture =
                    pot_bmp_texture(part, bmp.data, bmp.w, bmp.h, bmp.byte_pitch);
                
                image->tex_x = (f32)bmp.w/(f32)texture.w;
                image->tex_y = (f32)bmp.h/(f32)texture.h;
                image->width = (f32)bmp.w;
                image->height = (f32)bmp.h;
                image->id = image_id;
                
                prev_tex_id = image->texture_id;
                
                glGenTextures(1, &image->texture_id);
                glBindTexture(GL_TEXTURE_2D, image->texture_id);
                
                
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             texture.w, texture.h,
                             0, GL_RGBA, GL_UNSIGNED_BYTE,
                             texture.data);
            }
        }
        end_temp(temp);
    }
    return(prev_tex_id);
}

Bank_Bind_BMP_Sig(bind_bmp){
    Asset_Bank *bank = setup->bank;
    Partition *part = setup->trans_part;
    Image *image = get_image(bank, image_id);
    load_bmp(setup->system, part, image, image_id, file_name);
}

Bank_Replace_BMP_Sig(replace_bmp){
    u32 prev_tex_id = 0;
    Image *image = get_image(bank, image_id);
    prev_tex_id = load_bmp(system, part, image, image_id, file_name);
    if (prev_tex_id){
        glDeleteTextures(1, &prev_tex_id);
    }
}

#include <windows.h>
#include "cd_windows_render_vars.h"

#include "cd_gl_defines.h"

static void*
win32_load_gl_always(char *name, HMODULE module){
    void *p = (void *)wglGetProcAddress(name), *r = 0;
    if(p == 0 ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) ){
        r = (void *)GetProcAddress(module, name);
    }
    else{
        r = p;
    }
    return(r);
}

// TODO(allen): Need to move anything that couples
// rendering to platform into a separate layer.
Init_Sig(init){
    Win32_Render_Vars *vars = (Win32_Render_Vars*)render_vars;
    
    // GL context initialization
    {
        PIXELFORMATDESCRIPTOR format;
        int format_id;
        BOOL success;
        HDC dc;
        
        format.nSize = sizeof(format);
        format.nVersion = 1;
        format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        format.iPixelType = PFD_TYPE_RGBA;
        format.cColorBits = 32;
        format.cRedBits = 0;
        format.cRedShift = 0;
        format.cGreenBits = 0;
        format.cGreenShift = 0;
        format.cBlueBits = 0;
        format.cBlueShift = 0;
        format.cAlphaBits = 0;
        format.cAlphaShift = 0;
        format.cAccumBits = 0;
        format.cAccumRedBits = 0;
        format.cAccumGreenBits = 0;
        format.cAccumBlueBits = 0;
        format.cAccumAlphaBits = 0;
        format.cDepthBits = 24;
        format.cStencilBits = 8;
        format.cAuxBuffers = 0;
        format.iLayerType = PFD_MAIN_PLANE;
        format.bReserved = 0;
        format.dwLayerMask = 0;
        format.dwVisibleMask = 0;
        format.dwDamageMask = 0;
        
        dc = GetDC(vars->hwnd);
        Assert(dc);
        format_id = ChoosePixelFormat(dc, &format);
        Assert(format_id != 0);
        success = SetPixelFormat(dc, format_id, &format);
        Assert(success == TRUE);
        
        HGLRC glcontext = wglCreateContext(dc);
        wglMakeCurrent(dc, glcontext);
        
        {
            HMODULE module = LoadLibraryA("opengl32.dll");
            
            wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
            wglCreateContextAttribsARB = (wglCreateContextAttribsARB_Function*)
                win32_load_gl_always("wglCreateContextAttribsARB", module);
            
            wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
            wglChoosePixelFormatARB = (wglChoosePixelFormatARB_Function*)
                win32_load_gl_always("wglChoosePixelFormatARB", module);
            
            if (wglCreateContextAttribsARB != 0 && wglChoosePixelFormatARB != 0){
                const int choosePixel_attribList[] =
                {
                    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                    WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
                    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                    WGL_COLOR_BITS_ARB, 32,
                    WGL_DEPTH_BITS_ARB, 24,
                    WGL_STENCIL_BITS_ARB, 8,
                    0,
                };
                
                int extended_format_id;
                UINT num_formats = 0;
                BOOL result = 0;
                
                result = wglChoosePixelFormatARB(dc,
                                                 choosePixel_attribList,
                                                 0,
                                                 1,
                                                 &extended_format_id,
                                                 &num_formats);
                
                if (result != 0 && num_formats > 0){
                    const int createContext_attribList[] = {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0
                    };
                    
                    if (extended_format_id == format_id){
                        HGLRC extended_context =
                            wglCreateContextAttribsARB(dc,
                                                       0,
                                                       createContext_attribList);
                        if (extended_context){
                            wglMakeCurrent(dc, extended_context);
                            wglDeleteContext(glcontext);
                            glcontext = extended_context;
                            
                            MessageBoxA(0,(char*)glGetString(GL_VERSION), "OPENGL VERSION",0);
                        }
                    }
                }
            }
        }
        
        ReleaseDC(vars->hwnd, dc);
    }
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0,0,0,0);
}

Display_Sig(display){
    Win32_Render_Vars *vars = (Win32_Render_Vars*)render_vars;
    glFlush();
    HDC dc = GetDC(vars->hwnd);
    SwapBuffers(dc);
    ReleaseDC(vars->hwnd, dc);
}

Set_Screen_Sig(set_screen){
    if (w > 0 && h > 0){
        glViewport(0, h - out_h, out_w, out_h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, out_w, 0, out_h, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScissor(0, h - out_h, out_w, out_h);
    }
}

// BOTTOM

