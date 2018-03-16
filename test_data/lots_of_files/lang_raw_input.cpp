/*
 * Experiments with language/layout stuff for win32.
 * 04.01.2016 (dd.mm.yyyy)
 * Allen Webster
 */

// TOP

#include <Windows.h>

struct Win32Vars{
    int keep_playing;
    int controls[3];
} win32;

LRESULT WinCallback(
  HWND   hwnd,
  UINT   uMsg,
  WPARAM wParam,
  LPARAM lParam
                    ){
    LRESULT result = 0;
    switch(uMsg){
    case WM_QUIT:
    case WM_DESTROY:
    {
        win32.keep_playing = 0;
    }break;
    
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYUP:
    case WM_KEYDOWN:
    {
        switch (wParam){
        case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
        case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
        case VK_MENU:case VK_LMENU:case VK_RMENU:
            break;
            
        default:
        {            
            UINT vk = (UINT)wParam;
            UINT scan = (UINT)((lParam >> 16) & 0x7F);
            BYTE bytes[256];
            WORD x;
            memset(bytes, 0, 256);
            GetKeyboardState(bytes);
            ToAscii(vk, scan, bytes, &x, 0);
            int a = 0, b = 1, c = 2;
            a = x&1 + c;
            b *= a + x;
        }break;
        }
    }break;
    
    case WM_SETFOCUS:
    {
        win32.controls[0] = 0;
        win32.controls[1] = 0;
        win32.controls[2] = 0;
    }break;
    
    case WM_INPUT:
    {
        char buffer[sizeof(RAWINPUT)] = {};
        UINT size = sizeof(RAWINPUT);
        GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
        
        RAWINPUT *rawinput = (RAWINPUT*)(buffer);
        if (rawinput->header.dwType == RIM_TYPEKEYBOARD){
            RAWKEYBOARD *raw = &rawinput->data.keyboard;
            UINT vk = raw->VKey;
            UINT flags = raw->Flags;
            UINT down = !(flags & 1);
            
            if (vk != 255){
                if (vk == VK_SHIFT){
                    win32.controls[0] = down;
                }
                else if (vk == VK_CONTROL){
                    win32.controls[1] = down;
                }
                else if (vk == VK_MENU){
                    win32.controls[2] = down;
                }
            }
        }
        
        result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }break;
    
    default:
    {
        result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }break;
    }
    return(result);
}

int WinMain(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow
            ){
    WNDCLASS winclass = {};
    winclass.style = CS_HREDRAW | CS_VREDRAW;
    winclass.lpfnWndProc = WinCallback;
    winclass.hInstance = hInstance;
    winclass.lpszClassName = "lang-test-class";
    
    ATOM winclassatom = RegisterClassA(&winclass);
    
    if (winclassatom == 0){
        return(1);
    }
    
    HWND window = CreateWindowA(
        "lang-test-class",
        "lang test window",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        400,
        400,
        0,
        0,
        hInstance,
        0);

    if (window == 0){
        return(2);
    }
    
    RAWINPUTDEVICE device;
    device.usUsagePage = 0x1;
    device.usUsage = 0x6;
    device.dwFlags = 0;
    device.hwndTarget = window;
    RegisterRawInputDevices(&device, 1, sizeof(device));
    
    ShowWindow(window, SW_SHOW);
    
    for (win32.keep_playing = 1; win32.keep_playing;){
        MSG msg;
        GetMessage(&msg, 0, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return(0);
}

// BOTTOM


