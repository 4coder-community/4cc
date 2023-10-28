/*
 * Experiments with language/layout stuff for win32.
 * 04.01.2016 (dd.mm.yyyy)
 * Allen Webster
 */

// TOP

// NOTE(allen): This is a bare bones window opening and message pumping
// application. I created it to search for a way to get the language bar
// to behave correctly when my window has focus. As of the 4th of January,
// I have not yet found the solution.
//
// Log of Insanity
// DAY 2:
// Thanks to "insofaras" from the handmade_hero chat for finding the solution
// to this problem. Turns out the messages for the language bar were being blocked
// by my process because my reading of some MSDN documentation was too hasty. Go
// to GetMessage to see a full description of the problem I was having.
//
//
// DAY 1:
// I looked around MSDN for a while, reading about lots of layout manipulating
// functions. I also found the WM_INPUTLANGECHANGE message, but that message is
// apparently passed AFTER the language is changed, and the problem is that I can't
// change the languge using the GUI, so it doesn't seem like the right direction.
// 
// I also looked around the emacs source and tried matching some of the settings
// used there including WS_CLIPCHILDREN, and I tried using SetCapture/ReleaseCapture
// similarly to emacs code but none of that fixed the problem either.

#include <Windows.h>

struct Win32Vars{
    int keep_playing;
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

    // NOTE(allen): This was here to investigate the effects of capturing and especially
    // releasing mouse input, I have blocked them out for now because they didn't help
    // and I might want other messages of the same type in the future.
#if 0
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    {
        SetCapture(hwnd);
    }break;
    
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    {
        ReleaseCapture();
    }break;
#endif
    
    default:
    {
        result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
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
    
    ShowWindow(window, SW_SHOW);

    // NOTE(allen): This is here to investigate what happens when the layout
    // is changed in code. On Windows 7 I observed the language bar updating what
    // it displayed, but only if I dragged the window over the bar, or did something else
    // to force it to redraw. If you only have one layout loaded, nothing will happen here.
    HKL locales[2];
    int locale_count = GetKeyboardLayoutList(2, locales);
    if (locale_count > 1){
        ActivateKeyboardLayout(locales[1], 0);
    }
    
    for (win32.keep_playing = 1; win32.keep_playing;){
        MSG msg;
        // NOTE(allen): This turns out to be the crucial point. The second parameter filters out
        // messages by a window handle, and it turns out the language bar is one of those things
        // that needs to process input through the active process but the messages aren't tied
        // to your main window, so those messages get blocked if you put the handle there. Long
        // story short: put a 0 there.
        GetMessage(&msg, 0, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return(0);
}

// BOTTOM


