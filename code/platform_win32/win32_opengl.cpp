
#pragma comment(lib, "OpenGL32.lib")

#include "win32_gl.h"

#include <GL/gl.h>
#include "opengl/4ed_opengl_defines.h"
#define GL_FUNC(N,R,P) typedef R (CALL_CONVENTION N##_Function)P; N##_Function *N = 0;
#include "opengl/4ed_opengl_funcs.h"
#include "opengl/4ed_opengl_render.cpp"

internal b32
win32_wgl_good(Void_Func *f){
    return(f != 0 &&
           f != (Void_Func*)1 &&
           f != (Void_Func*)2 &&
           f != (Void_Func*)3 &&
           f != (Void_Func*)-1);
}

typedef HGLRC (CALL_CONVENTION wglCreateContextAttribsARB_Function)(HDC,HGLRC,i32*);
typedef BOOL  (CALL_CONVENTION wglChoosePixelFormatARB_Function)(HDC,i32*,f32*,u32,i32*,u32*);
typedef char* (CALL_CONVENTION wglGetExtensionsStringEXT_Function)();
typedef VOID  (CALL_CONVENTION wglSwapIntervalEXT_Function)(i32);

global wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
global wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
global wglGetExtensionsStringEXT_Function *wglGetExtensionsStringEXT = 0;
global wglSwapIntervalEXT_Function *wglSwapIntervalEXT = 0;

internal LRESULT CALL_CONVENTION
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

internal b32
win32_gl_create_window(HWND *wnd_out, HGLRC *context_out, DWORD style, RECT rect){
    HINSTANCE this_instance = GetModuleHandle(0);
    
    local_persist b32 srgb_support = false;
    local_persist b32 register_success = true;
    local_persist b32 first_call = true;
    if (first_call){
        log_os(" GL bootstrapping...\n");
        
        first_call = false;
        
        // NOTE(allen): Create the GL bootstrap window
        log_os(" registering bootstrap class...\n");
        WNDCLASSW wglclass = {};
        wglclass.lpfnWndProc = DefWindowProcW;
        wglclass.hInstance = this_instance;
        wglclass.lpszClassName = L"wgl-loader";
        if (RegisterClassW(&wglclass) == 0){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" creating bootstrap window...\n");
        HWND wglwindow = CreateWindowW(wglclass.lpszClassName, L"", 0, 0, 0, 0, 0,
                                       0, 0, this_instance, 0);
        if (wglwindow == 0){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Create the GL bootstrap context
        log_os(" setting bootstrap pixel format...\n");
        
        HDC wgldc = GetDC(wglwindow);
        
        PIXELFORMATDESCRIPTOR format = {};
        format.nSize = sizeof(format);
        format.nVersion = 1;
        format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        format.iPixelType = PFD_TYPE_RGBA;
        format.cColorBits = 32;
        format.cAlphaBits = 8;
        format.cDepthBits = 24;
        format.iLayerType = PFD_MAIN_PLANE;
        i32 suggested_format_index = ChoosePixelFormat(wgldc, &format);
        if (!SetPixelFormat(wgldc, suggested_format_index, &format)){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" creating bootstrap GL context...\n");
        HGLRC wglcontext = wglCreateContext(wgldc);
        if (wglcontext == 0){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" making bootstrap GL context current...\n");
        if (!wglMakeCurrent(wgldc, wglcontext)){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Load wgl extensions
        log_os(" loading wgl extensions...\n");
        
#define LoadWGL(f,l) Stmnt((f) = (f##_Function*)wglGetProcAddress(#f); \
(l) = (l) && win32_wgl_good((Void_Func*)(f));)
        
        b32 load_success = true;
        LoadWGL(wglCreateContextAttribsARB, load_success);
        LoadWGL(wglChoosePixelFormatARB, load_success);
        LoadWGL(wglGetExtensionsStringEXT, load_success);
        
        if (!load_success){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" checking wgl extensions...\n");
        char *extensions_c = wglGetExtensionsStringEXT();
        String_Const_u8 extensions = SCu8((u8*)extensions_c);
        
        {
            String_Const_u8 s = string_skip_whitespace(extensions);
            for (;s.size > 0;){
                u64 end = string_find_first_whitespace(s);
                String_Const_u8 m = string_prefix(s, end);
                if (string_match(m, string_u8_litexpr("WGL_EXT_framebuffer_sRGB")) ||
                    string_match(m, string_u8_litexpr("WGL_ARB_framebuffer_sRGB"))){
                    srgb_support = true;
                }
                else if (string_match(m, string_u8_litexpr("WGL_EXT_swap_interval"))){
                    b32 wgl_swap_interval_ext = true;
                    LoadWGL(wglSwapIntervalEXT, wgl_swap_interval_ext);
                    if (!wgl_swap_interval_ext){
                        wglSwapIntervalEXT = 0;
                    }
                }
                s = string_skip_whitespace(string_skip(s, end));
            }
        }
        
        // NOTE(allen): Load gl functions
        log_os(" loading core GL functions...\n");
        
#define GL_FUNC(f,R,P) LoadWGL(f,load_success);
#include "opengl/4ed_opengl_funcs.h"
        
        if (!load_success){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Cleanup the GL bootstrap resources
        log_os(" cleaning up boostrap resources...\n");
        
        ReleaseDC(wglwindow, wgldc);
        DestroyWindow(wglwindow);
        wglDeleteContext(wglcontext);
        
        // NOTE(allen): Register the graphics window class
        log_os(" registering graphics class...\n");
        
        WNDCLASSW wndclass = {};
        wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
        wndclass.lpfnWndProc = win32_proc;
        wndclass.hIcon = LoadIconW(GetModuleHandle(0), L"main");
        wndclass.hInstance = this_instance;
        wndclass.lpszClassName = L"GRAPHICS-WINDOW-NAME";
        if (RegisterClassW(&wndclass) == 0){
            register_success = false;
            goto fail_register;
        }
    }
    fail_register:;
    
    b32 result = false;
    if (register_success){
        // NOTE(allen): Create the graphics window
        log_os(" creating graphics window...\n");
        
        HWND wnd = CreateWindowExW(0, L"GRAPHICS-WINDOW-NAME", L"GRAPHICS", style,
                                   CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                                   0, 0, this_instance, 0);
        
        *wnd_out = 0;
        *context_out = 0;
        if (wnd != 0){
            log_os(" setting graphics pixel format...\n");
            
            HDC dc = GetDC(wnd);
            
            PIXELFORMATDESCRIPTOR format = {};
            
            i32 pixel_attrib_list[] = {
                /* 0*/WGL_DRAW_TO_WINDOW_ARB, TRUE,
                /* 2*/WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                /* 4*/WGL_SUPPORT_OPENGL_ARB, TRUE,
                /* 6*/WGL_DOUBLE_BUFFER_ARB, true,
                /* 8*/WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                /*10*/WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
                /*12*/0,
            };
            if (!srgb_support){
                pixel_attrib_list[10] = 0;
            }
            
            i32 suggested_format_index = 0;
            u32 ignore = 0;
            if (!wglChoosePixelFormatARB(dc, pixel_attrib_list, 0, 1, &suggested_format_index, &ignore)){
                goto fail_window_init;
            }
            
            DescribePixelFormat(dc, suggested_format_index, sizeof(format), &format);
            if (!SetPixelFormat(dc, suggested_format_index, &format)){
                goto fail_window_init;
            }
            
            log_os(" setting graphics attributes...\n");
            
            i32 context_attrib_list[] = {
                /*0*/WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                /*2*/WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                /*4*/WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if GL_DEBUG_MODE
                |WGL_CONTEXT_DEBUG_BIT_ARB
#endif
                ,
                /*6*/WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                /*8*/0
            };
            
            log_os(" creating graphics GL context...\n");
            HGLRC context = wglCreateContextAttribsARB(dc, 0, context_attrib_list);
            if (context == 0){
                goto fail_window_init;
            }
            
            log_os(" making graphics GL context current...\n");
            wglMakeCurrent(dc, context);
            
            
            if (wglSwapIntervalEXT != 0){
                log_os(" setting swap interval...\n");
                wglSwapIntervalEXT(1);
            }
            *wnd_out = wnd;
            *context_out = context;
            result = true;
            
            if (false){
                fail_window_init:;
                DWORD error = GetLastError();
                ReleaseDC(wnd, dc);
                DestroyWindow(wnd);
                SetLastError(error);
            } 
        }
    }
    
    return(result);
}