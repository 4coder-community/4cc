/*
 * 4tech reusable win32 layer
 *   by Allen Webster
 * 08.20.2015
 *
 */

// TOP

#include <Windows.h>
#include <Windowsx.h>

struct Win32{
    HWND hwnd;
    HDC dc;
    bool keep_playing;
    
    Memory memory;
    Render_Target render_target;
    Mouse_State mouse, previous_mouse;
    
	HCURSOR cursor_ibeam;
	HCURSOR cursor_arrow;
	HCURSOR cursor_leftright;
	HCURSOR cursor_updown;
    
#if RENDER_MODE == OPENGL_RENDER
    HGLRC glcontext;
#endif
};

global Win32 win32;

internal void
win32_debug(char *msg){
    MessageBox(win32.hwnd,
               msg,
               "debug out",
               MB_OK);
}

#if RENDER_MODE == OPENGL_RENDER

internal void
win32_screen_init(){
    PIXELFORMATDESCRIPTOR format;
    int format_id;
    BOOL success;
    
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
    format.cDepthBits = 16;
    format.cStencilBits = 0;
    format.cAuxBuffers = 0;
    format.iLayerType = PFD_MAIN_PLANE;
    format.bReserved = 0;
    format.dwLayerMask = 0;
    format.dwVisibleMask = 0;
    format.dwDamageMask = 0;
    
    win32.dc = GetDC(win32.hwnd);
    _Assert(win32.dc);
    format_id = ChoosePixelFormat(win32.dc, &format);
    _Assert(format_id != 0);
    success = SetPixelFormat(win32.dc, format_id, &format);
    _Assert(success == TRUE);
    
    win32.glcontext = wglCreateContext(win32.dc);
    wglMakeCurrent(win32.dc, win32.glcontext);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0,0,0,0);
    glFlush();
    SwapBuffers(win32.dc);
    ShowWindow(win32.hwnd, SW_SHOW);
}

internal void
win32_resize_screen(i32 w, i32 h){
    if (w > 0 && h > 0){
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, w, h, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glScissor(0, 0, w, h);
        
        win32.render_target.w = w;
        win32.render_target.h = h;
    }
}

internal void
win32_redraw_screen(){
    glFlush();
    SwapBuffers(win32.dc);
}

internal void
win32_screen_free(){
    wglMakeCurrent(0, 0);
    wglDeleteContext(win32.glcontext);
}

#endif

internal void
win32_track_leave(){
    TRACKMOUSEEVENT track;
    track.cbSize = sizeof(track);
    track.dwFlags = TME_LEAVE;
    track.hwndTrack = win32.hwnd;
    TrackMouseEvent(&track);
}

internal u64
win32_get_time(){
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);
    return time.QuadPart;
}

LRESULT ColorWindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam){
    LRESULT result = {};
    switch (uMsg){
    case WM_SIZE:
    {
        i32 w = LOWORD(lParam);
        i32 h = HIWORD(lParam);
        win32_resize_screen(w, h);
    }break;
        
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        
        App_Step_Data step;
        step.memory = win32.memory;
        step.render_target = win32.render_target;
        step.mouse = win32.mouse;
        App_Step_Out step_out = {};
        app_step(&step, &step_out);
        win32_redraw_screen();
        
        EndPaint(hwnd, &ps);
    }break;
    
    case WM_CLOSE:
    case WM_DESTROY:
    {
        win32.keep_playing = 0;
    }break;
    
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        
    }break;
    
	case WM_LBUTTONDOWN:
	{
		win32.mouse.l = 1;
	}break;
	case WM_RBUTTONDOWN:
	{
		win32.mouse.r = 1;
	}break;
	
	case WM_LBUTTONUP:
	{
		win32.mouse.l = 0;
	}break;
	case WM_RBUTTONUP:
	{
		win32.mouse.r = 0;
	}break;
	
    case WM_MOUSEMOVE:
    {
        win32.mouse.x = GET_X_LPARAM(lParam);
        win32.mouse.y = GET_Y_LPARAM(lParam);
        win32.mouse.in_window = 1;
        win32_track_leave();
    }break;
    
    case WM_MOUSEWHEEL:
    {
        win32.mouse.wheel = GET_WHEEL_DELTA_WPARAM(wParam);
    }break;
    
    case WM_MOUSELEAVE:
    {
        win32.mouse.in_window = 0;
    }break;
    
    case WM_KILLFOCUS:
    {
        win32.mouse = {};
    }break;
    
    default:
    {
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }break;
    }
    return result;
}

int
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow){
    win32 = {};
    
    WNDCLASSEX winclass;
    winclass.cbSize = sizeof(WNDCLASSEX);
    winclass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    winclass.lpfnWndProc = ColorWindowProc;
    winclass.cbClsExtra = 0;
    winclass.cbWndExtra = 0;
    winclass.hInstance = hInstance;
    winclass.hIcon = 0;
    winclass.hCursor = 0;
    winclass.hbrBackground = 0;
    winclass.lpszMenuName = 0;
    winclass.lpszClassName = "color-class";
    winclass.hIconSm = 0;
    
    ATOM register_result = RegisterClassEx(&winclass);
    _Assert(register_result);
    
    i32 w, h;
    w = 800;
    h = 600;
    
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = w;
    window_rect.bottom = h;
    
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, 0);
    
    HWND hwnd =
        CreateWindowEx(0,
                       "color-class",
                       "4tech colors",
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       window_rect.right - window_rect.left,
                       window_rect.bottom - window_rect.top,
                       0,
                       0,
                       hInstance,
                       0);
    _Assert(hwnd);
    
    win32.hwnd = hwnd;
    win32.dc = GetDC(win32.hwnd);
    win32_screen_init();
    
    GetClientRect(win32.hwnd, &window_rect);
    
    win32_resize_screen(window_rect.right - window_rect.left,
                        window_rect.bottom - window_rect.top);
    
    win32.memory.size = Mbytes(1);
    win32.memory.mem = VirtualAlloc((LPVOID)Tbytes(2),
                                    win32.memory.size,
                                    MEM_RESERVE|MEM_COMMIT,
                                    PAGE_READWRITE);
    _Assert(win32.memory.mem);
    
    win32.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    
    u64 frame_target = MILLION / 30;
    u64 frame_start, frame_end;
    u64 frame_used;
    
    timeBeginPeriod(1);
    win32.keep_playing = 1;
    frame_start = win32_get_time();
    while (win32.keep_playing){
        MSG msg;
        win32.previous_mouse = win32.mouse;
        while (PeekMessage(&msg,
                           0,0,0,
                           PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        win32.mouse.pl = win32.mouse.l && !win32.previous_mouse.l;
        win32.mouse.pr = win32.mouse.r && !win32.previous_mouse.r;
        
        App_Step_Data step;
        step.memory = win32.memory;
        step.render_target = win32.render_target;
        step.mouse = win32.mouse;
        
        App_Step_Out step_out = {};
        app_step(&step, &step_out);
        win32_redraw_screen();

        switch (step_out.mouse_cursor){
        case APP_MOUSE_CURSOR_IBEAM:
            SetCursor(win32.cursor_ibeam);
            break;
            
        case APP_MOUSE_CURSOR_ARROW:
            SetCursor(win32.cursor_arrow);
            break;
            
        case APP_MOUSE_CURSOR_LEFTRIGHT:
            SetCursor(win32.cursor_leftright);
            break;
            
        case APP_MOUSE_CURSOR_UPDOWN:
            SetCursor(win32.cursor_updown);
            break;
        }
        
        frame_end = win32_get_time();
        frame_used = frame_end - frame_start;
        if (frame_used < frame_target){
            Sleep((DWORD)(frame_target - frame_used) / 1000);
        }
        frame_start = win32_get_time();
    }
    
    return 0;
}

// BOTTOM
