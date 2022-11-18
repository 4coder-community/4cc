/*
 * Win32 Layer for CipherDrive
 */

// TOP

#define LOGICAL_SIZE Mbytes(8)
#define TRANSIENT_SIZE Gbytes(1)

extern "C"{
#include "DragAndDrop.h"
}

#define FTECH_STRING_IMPLEMENTATION
#include "4tech_string.h"

#include "cd_windows_render_vars.h"

struct DLL_Reload{
    HMODULE handle;
    FILETIME time;
    i32 counter;
};

struct Win32{
    Win32_Render_Vars render_vars;
    
    b32 keep_playing;
    
    u64 perf_frequency;
    u64 perf_start;
    
    Memory memory;
    Render_Target target;
    Render_Target dbg_target;
    Asset_Bank bank;
    Input_State input;
    Dev_Input_State dev_input;
    Key_Events keys;
    
    System_API system;
    
    
    App_Functions app;
    
    i32 w, h, out_w, out_h;
    
#ifdef DEVELOPER
    void *logical_backup;
    void *logical_backup_stopped;
    Input_State *past_inputs;
    i32 max_inputs;
    i32 loop_i, loop_end;
    i32 loop_mode;
    
    Input_State input_stopped;
    i32 loop_stopped;
    b32 file_drop_lock;
    
    DLL_Reload renderer_reload;
    DLL_Reload game_reload;
#endif
};

Win32 win32;

static HANDLE
convert_handle(Platform_Handle handle){
    HANDLE result;
    Assert(sizeof(HANDLE) <= sizeof(Platform_Handle));
    result = *(HANDLE*)(&handle);
    return(result);
}

static Platform_Handle
convert_handle(HANDLE handle){
    Platform_Handle result = {0};
    Assert(sizeof(HANDLE) <= sizeof(Platform_Handle));
    *(HANDLE*)(&result) = handle;
    return(result);
}

static u64
win32_get_time(){
    u64 result = 0;
    LARGE_INTEGER time;
    if (QueryPerformanceCounter(&time)){
        result = (time.QuadPart - win32.perf_start) * 1000000 / win32.perf_frequency;
    }
    return(result);
}

static void
win32_init_gl(){
    Assert(win32.target.init != 0);
    win32.target.init(&win32.render_vars);
}

static void
win32_set_size(i32 w, i32 h, i32 out_w, i32 out_h){
    Assert(win32.target.set_screen != 0);
    win32.target.set_screen(w, h, out_w, out_h);
}

static u8 keycode_lookup_table[255];

static void
win32_keycode_init(){
    cd_memset(keycode_lookup_table, 0, sizeof(keycode_lookup_table));
    
    keycode_lookup_table[VK_BACK] = key_back;
    keycode_lookup_table[VK_DELETE] = key_del;
    keycode_lookup_table[VK_UP] = key_up;
    keycode_lookup_table[VK_DOWN] = key_down;
    keycode_lookup_table[VK_LEFT] = key_left;
    keycode_lookup_table[VK_RIGHT] = key_right;
    keycode_lookup_table[VK_INSERT] = key_insert;
    keycode_lookup_table[VK_HOME] = key_home;
    keycode_lookup_table[VK_END] = key_end;
    keycode_lookup_table[VK_PRIOR] = key_page_up;
    keycode_lookup_table[VK_NEXT] = key_page_down;
    keycode_lookup_table[VK_ESCAPE] = key_esc;
    
    keycode_lookup_table[VK_F1] = key_f1;
    keycode_lookup_table[VK_F2] = key_f2;
    keycode_lookup_table[VK_F3] = key_f3;
    keycode_lookup_table[VK_F4] = key_f4;
    keycode_lookup_table[VK_F5] = key_f5;
    keycode_lookup_table[VK_F6] = key_f6;
    keycode_lookup_table[VK_F7] = key_f7;
    keycode_lookup_table[VK_F8] = key_f8;
    keycode_lookup_table[VK_F9] = key_f9;
    
    keycode_lookup_table[VK_F10] = key_f10;
    keycode_lookup_table[VK_F11] = key_f11;
    keycode_lookup_table[VK_F12] = key_f12;
    keycode_lookup_table[VK_F13] = key_f13;
    keycode_lookup_table[VK_F14] = key_f14;
    keycode_lookup_table[VK_F15] = key_f15;
    keycode_lookup_table[VK_F16] = key_f16;
}

static LRESULT
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    
    switch (uMsg){
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32.keep_playing = false;
        }break;
        
        case WM_SIZE:
        {
            win32.w = LOWORD(lParam);
            win32.h = HIWORD(lParam);
            win32_set_size(win32.w, win32.h, win32.out_w, win32.out_h);
        }break;
        
        case WM_MENUCHAR:
        case WM_SYSCHAR:break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            switch (wParam){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:break;
                
                default:
                {
                    b8 current_state = ((lParam & Bit_31)?(0):(1));
                    if (current_state){
                        char key = keycode_lookup_table[(u8)wParam];
                        
                        if (!key){
                            BYTE state[256];
                            GetKeyboardState(state);
                            state[VK_CONTROL] = 0;
                            
                            UINT vk = (UINT)wParam;
                            UINT scan = (UINT)((lParam >> 16) & 0x7F);
                            WORD c = 0;
                            i32 result = ToAscii(vk, scan, state, &c, 0);
                            
                            if (result < 0){
                                ToAscii(vk, scan, state, &c, 0);
                            }
                            else if (result == 1){
                                key = (char)c;
                                if (key == '\r'){
                                    key = '\n';
                                }
                            }
                        }
                        
                        if (key){
                            i32 max = ArrayCount(win32.keys.events);
                            if (win32.keys.count < max){
                                win32.keys.events[win32.keys.count++] = key;
                            }
                        }
                    }
                }break;
            }
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    
    return(result);
}

#if 0
static char*
win32_cf_type(CLIPFORMAT cf){
    char *which = "";
    switch (cf){
        case CF_TEXT: which = "CF_TEXT"; break;
        case CF_BITMAP: which = "CF_BITMAP"; break;
        case CF_METAFILEPICT: which = "CF_METAFILEPICT"; break;
        case CF_SYLK: which = "CF_SYLK"; break;
        case CF_DIF: which = "CF_DIF"; break;
        case CF_TIFF: which = "CF_TIFF"; break;
        case CF_OEMTEXT: which = "CF_OEMTEXT"; break;
        case CF_DIB: which = "CF_DIB"; break;
        case CF_PALETTE: which = "CF_PALETTE"; break;
        case CF_PENDATA: which = "CF_PENDATA"; break;
        case CF_RIFF: which = "CF_RIFF"; break;
        case CF_WAVE: which = "CF_WAVE"; break;
        case CF_UNICODETEXT: which = "CF_UNICODETEXT"; break;
        case CF_ENHMETAFILE: which = "CF_ENHMETAFILE"; break;
        case CF_HDROP: which = "CF_HDROP"; break;
        case CF_LOCALE: which = "CF_LOCALE"; break;
        case CF_MAX: which = "CF_MAX"; break;
        case CF_OWNERDISPLAY: which = "CF_OWNERDISPLAY"; break;
        case CF_DSPTEXT: which = "CF_DSPTEXT"; break;
        case CF_DSPBITMAP: which = "CF_DSPBITMAP"; break;
        case CF_DSPMETAFILEPICT: which = "CF_DSPMETAFILEPICT"; break;
        case CF_DSPENHMETAFILE: which = "CF_DSPENHMETAFILE"; break;
        case CF_PRIVATEFIRST: which = "CF_PRIVATEFIRST"; break;
        case CF_PRIVATELAST: which = "CF_PRIVATELAST"; break;
        case CF_GDIOBJFIRST: which = "CF_GDIOBJFIRST"; break;
        case CF_GDIOBJLAST: which = "CF_GDIOBJLAST"; break;
    }
    return(which);
}
#endif

DWORD
win32_drop_callback(CLIPFORMAT cf, HGLOBAL hData, HWND hWnd,
                    DWORD dwKeyState, POINTL pt,
                    void *pUserData){
    DWORD effect = DROPEFFECT_NONE;
    
    Assert(win32.file_drop_lock);
    
    u32 count = DragQueryFile((HDROP)hData, 0xFFFFFFFF, 0, 0);
    u32 max = ArrayCount(win32.dev_input.drops);
    max -= win32.dev_input.drop_count;
    
    if (count > max){
        count = max;
    }
    
    for (u32 i = 0; i < count; ++i){
        TCHAR file_path[1024];
        
        DWORD len = DragQueryFile((HDROP)hData, i,
                                  file_path, ArrayCount(file_path));
        
        if (len < 1024){
            Dev_File_Drop *drop = &win32.dev_input.drops[win32.dev_input.drop_count++];
            cd_memcpy(drop->name, file_path, len);
            drop->name[len] = 0;
        }
        else{
            // TODO(allen): Issue warning to developer person who has long file name.
        }
    }
    
    win32.dev_input.drop_x = (f32)(pt.x);
    win32.dev_input.drop_y = (f32)(pt.y);
    
    return(effect);
}

#ifdef DEVELOPER

File_Dump
DBG_dump_begin(char *filename){
    File_Dump dump = {0};
    HANDLE file = 0;
    LARGE_INTEGER size = {0};
    
    file = CreateFile(filename, GENERIC_READ, 0, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file != INVALID_HANDLE_VALUE){
        if (GetFileSizeEx(file, &size)){
            if (size.HighPart == 0){
                dump.handle = convert_handle(file);
                dump.size = size.LowPart;
            }
            else{
                CloseHandle(file);
            }
        }
        else{
            CloseHandle(file);
        }
    }
    
    return(dump);
}

b32
DBG_dump_end(File_Dump dump, void *buffer){
    b32 result = false;
    HANDLE file = convert_handle(dump.handle);
    DWORD total_unread = dump.size;
    DWORD read_amount = 0;
    
    if (file != 0){
        if (buffer){
            while (total_unread > 0){
                if (ReadFile(file, buffer, total_unread, &read_amount, 0)){
                    buffer = (char*)buffer + read_amount;
                    total_unread -= read_amount;
                }
                else{
                    break;
                }
            }
            if (total_unread == 0){
                result = true;
            }
        }
        CloseHandle(file);
    }
    return(result);
}

b32
DBG_dump_out(char *file_name, void *buffer, i32 size){
    b32 result = false;
    
    if (buffer){
        HANDLE file = 
            CreateFile(file_name,
                       GENERIC_WRITE,
                       0,
                       0,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       0);
        DWORD size_written = 0;
        
        if (file != INVALID_HANDLE_VALUE){
            WriteFile(file, buffer, size, &size_written, 0);
            result = true;
            CloseHandle(file);
        }
    }
    else{
        DeleteFile(file_name);
    }
    
    return(result);
}

b32
DBG_copy(char *source, char *name){
    b32 result = CopyFile(source, name, false);
    return(result);
}

i32
DBG_module_path(char *out, i32 capacity){
    i32 result = 0;
    i32 len = GetModuleFileName(0, out, capacity);
    
    if (len < capacity-1){
        String str = make_string(out, len, len);
        remove_last_folder(&str);
        if (str.str[str.size-1] == '\\'){
            str.size-=1;
        }
        terminate_with_null(&str);
        result = str.size;
    }
    
    return(result);
}

i32
DBG_working_path(char *out, i32 capacity){
    i32 result = 0;
    i32 len = GetCurrentDirectory(capacity, out);
    
    if (len < capacity-1){
        result = len;
    }
    
    return(result);
}

b32
DBG_call_script(char *script){
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    char command_line[2048];
    
    String s = make_fixed_width_string(command_line);
    
    copy(&s, make_lit_string("/C "));
    append_partial(&s, script);
    b32 success = terminate_with_null(&s);
    
    if (success){
        success = false;
        
        char path[2048];
        i32 path_len = DBG_module_path(path, sizeof(path));
        
        if (path_len > 0){
            STARTUPINFO startup = {};
            startup.cb = sizeof(STARTUPINFO);
            
            PROCESS_INFORMATION info = {};
            
            if (CreateProcess(cmd, command_line,
                              0, 0, FALSE, CREATE_NO_WINDOW,
                              env_variables, path,
                              &startup, &info)){
                
                CloseHandle(info.hThread);
                CloseHandle(info.hProcess);
                
                success = true;
            }
        }
    }
    
    return(success);
}

#endif

static void*
win32_alloc(i32 size){
    void *result = VirtualAlloc(0, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return(result);
}

static void
win32_free(void *ptr){
    VirtualFree(ptr, 0, MEM_RELEASE);
}

static char*
game_temp_name(i32 counter){
    char *temp = "game_temp0.dll";
    switch (counter % 4){
        case 1: temp = "game_temp1.dll"; break;
        case 2: temp = "game_temp2.dll"; break;
        case 3: temp = "game_temp3.dll"; break;
    }
    return(temp);
}

static char*
renderer_temp_name(i32 counter){
    char *temp = "renderer_temp0.dll";
    switch (counter % 4){
        case 1: temp = "renderer_temp1.dll"; break;
        case 2: temp = "renderer_temp2.dll"; break;
        case 3: temp = "renderer_temp3.dll"; break;
    }
    return(temp);
}

// TODO(allen): Rewrite using CopyFile dumbass.
static HMODULE
win32_copy_load(String path, char *file_name, char *temp_name){
    HMODULE module = 0;
    
    append(&path, file_name);
    terminate_with_null(&path);
    
    File_Dump dump = DBG_dump_begin(path.str);
    if (dump.size > 0){
        void *buffer = win32_alloc(dump.size);
        if (buffer){
            if (DBG_dump_end(dump, buffer)){
                remove_last_folder(&path);
                append(&path, temp_name);
                terminate_with_null(&path);
                DBG_dump_out(path.str, buffer, dump.size);
                module = LoadLibraryA(path.str);
            }
            win32_free(buffer);
        }
    }
    
    remove_last_folder(&path);
    terminate_with_null(&path);
    
    return(module);
}

static HMODULE
win32_try_reload(DLL_Reload *reload, String path, char *file_name, char *temp_name){
    HMODULE module = 0;
    FILETIME file_time_now;
    LONG updated = 0;
    HANDLE file = 0;
    
    append(&path, file_name);
    terminate_with_null(&path);
    
    file = CreateFile(path.str,
                      GENERIC_READ,
                      0,
                      0,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      0);
    
    remove_last_folder(&path);
    
    if (file != INVALID_HANDLE_VALUE){
        GetFileTime(file, 0, 0, &file_time_now);
        CloseHandle(file);
        
        updated = (CompareFileTime(&reload->time, &file_time_now) < 0);
        
        if (reload->handle == 0 || updated){
            if (reload->handle != 0){
                FreeLibrary(reload->handle);
            }
            
            reload->time = file_time_now;
            reload->handle = win32_copy_load(path, file_name, temp_name);
            
            module = reload->handle;
        }
    }
    
    return(module);
}

static void
win32_reload_renderer(String path_string){
    HMODULE game_render_module = 0;
    Render_Get_Functions *target_get_functions = 0;
    Bank_Get_Functions *bank_get_functions = 0;
    
    game_render_module =
        win32_try_reload(&win32.renderer_reload,
                         path_string,
                         "CDRenderer.dll",
                         renderer_temp_name(win32.renderer_reload.counter++));
    
    if (game_render_module != 0){
        target_get_functions = (Render_Get_Functions*)
            GetProcAddress(game_render_module, "target_get_functions");
        Assert(target_get_functions != 0);
        
        bank_get_functions = (Bank_Get_Functions*)
            GetProcAddress(game_render_module, "bank_get_functions");
        Assert(target_get_functions != 0);
        
        target_get_functions(&win32.target);
        bank_get_functions(&win32.bank);
        
#ifdef DEVELOPER
        target_get_functions(&win32.dbg_target);
        win32.dbg_target.execute = dbg_render_execute;
#endif
    }
}

static void
win32_reload_game(String path_string){
    HMODULE application_module = 0;
    App_Step_Function *app_step = 0;
    
    application_module =
        win32_try_reload(&win32.game_reload,
                         path_string,
                         "CDGame.dll",
                         game_temp_name(win32.game_reload.counter++));
    
    if (application_module != 0){
        app_step = (App_Step_Function*)
            GetProcAddress(application_module, "app_step");
        Assert(app_step != 0);
        
        win32.app.step = app_step;
    }
}

int
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow){
    
    {
        char dir_space[1024];
        int len = GetCurrentDirectory(sizeof(dir_space), dir_space);
        String dir = make_string(dir_space, len, sizeof(dir_space));
        append(&dir, "\\data");
        terminate_with_null(&dir);
        
        if (SetCurrentDirectory(dir.str) == FALSE){
            exit(1);
        }
    }
    
    // Window initialization
    win32 = {0};
    win32_keycode_init();
    
    {
        LARGE_INTEGER lpf;
        QueryPerformanceFrequency(&lpf);
        win32.perf_frequency = lpf.QuadPart;
        QueryPerformanceCounter(&lpf);
        win32.perf_start = lpf.QuadPart;
    }
    
    // Memory initialization
    {
        u64 offset = 0;
        LPVOID ptr;
        
        ptr = (LPVOID)Gbytes(1);
        win32.memory.logical_size = LOGICAL_SIZE;
        win32.memory.logical = VirtualAlloc(ptr, LOGICAL_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
#ifdef DEVELOPER
        offset += LOGICAL_SIZE;
        ptr = (LPVOID)(Gbytes(1) + offset);
        win32.logical_backup = VirtualAlloc(ptr, LOGICAL_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
        offset += LOGICAL_SIZE;
        ptr = (LPVOID)(Gbytes(1) + offset);
        win32.logical_backup_stopped = VirtualAlloc(ptr, LOGICAL_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
        offset += LOGICAL_SIZE;
        ptr = (LPVOID)(Gbytes(1) + offset);
        win32.past_inputs = (Input_State*)
            VirtualAlloc(ptr, LOGICAL_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        win32.max_inputs = LOGICAL_SIZE/sizeof(Input_State);
        
        offset += LOGICAL_SIZE;
        ptr = (LPVOID)(Gbytes(1) + offset);
        win32.memory.developer = VirtualAlloc(ptr, LOGICAL_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        win32.memory.developer_size = LOGICAL_SIZE;
#endif
        
        ptr = (LPVOID)Gbytes(2);
        win32.memory.transient_size = TRANSIENT_SIZE;
        win32.memory.transient = VirtualAlloc(ptr, TRANSIENT_SIZE, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    }
    
    // Linkage initialization
    char file_name[1024];
    DWORD len = 
        GetModuleFileName(0,
                          file_name,
                          sizeof(file_name));
    
    String path_string = make_string(file_name, len, sizeof(file_name));
    remove_last_folder(&path_string);
    
    {
        win32.system.DBG_dump_begin = DBG_dump_begin;
        win32.system.DBG_dump_end = DBG_dump_end;
        win32.system.DBG_dump_out = DBG_dump_out;
        win32.system.DBG_copy = DBG_copy;
        win32.system.DBG_call_script = DBG_call_script;
        win32.system.DBG_module_path = DBG_module_path;
        win32.system.DBG_working_path = DBG_working_path;
        win32.system.DBG_memory_allocate = win32_alloc;
        win32.system.DBG_memory_free = win32_free;
        
        win32_reload_renderer(path_string);
        win32_reload_game(path_string);
    }
    
    WNDCLASSEX winclass;
    winclass.cbSize = sizeof(WNDCLASSEX);
    winclass.style = CS_HREDRAW | CS_VREDRAW;
    winclass.lpfnWndProc = win32_proc;
    winclass.cbClsExtra = 0;
    winclass.cbWndExtra = 0;
    winclass.hInstance = hInstance;
    winclass.hIcon = 0;
    winclass.hCursor = 0;
    winclass.hbrBackground = 0;
    winclass.lpszMenuName = 0;
    winclass.lpszClassName = "cipher-drive-class";
    winclass.hIconSm = 0;
    
    ATOM register_result = RegisterClassEx(&winclass);
    Assert(register_result);
    
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = DEFAULT_WIDTH;
    window_rect.bottom = DEFAULT_HEIGHT;
    
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, 0);
    
    HWND hwnd =
        CreateWindowEx(0,
                       "cipher-drive-class",
                       "Cipher Drive - Dev",
                       WS_OVERLAPPEDWINDOW,
                       CW_USEDEFAULT,
                       CW_USEDEFAULT,
                       window_rect.right - window_rect.left,
                       window_rect.bottom - window_rect.top,
                       0,
                       0,
                       hInstance,
                       0);
    
    Assert(hwnd);
    
    win32.render_vars.hwnd = hwnd;
    
    GetClientRect(win32.render_vars.hwnd, &window_rect);
    
    win32.w = window_rect.right - window_rect.left;
    win32.h = window_rect.bottom - window_rect.top;
    win32.out_w = DEFAULT_WIDTH;
    win32.out_h = DEFAULT_HEIGHT;
    
    win32.target.dim = v2((f32)win32.out_w, (f32)win32.out_h);
    win32.dbg_target.dim = v2((f32)win32.out_w, (f32)win32.out_h);
    
    
    // GL initialization
    {
        win32_init_gl();
        win32_set_size(win32.w, win32.h, win32.out_w, win32.out_h);
    }
    
#ifdef DEVELOPER
    {
        // Begin drag&drop
        MyDragDropInit(0);
        CLIPFORMAT formats[] = {
            CF_HDROP
        };
        MyRegisterDragDrop(win32.render_vars.hwnd,
                           formats, ArrayCount(formats),
                           WM_NULL, win32_drop_callback,
                           0);
    }
#endif
    
    // Main loop
    u64 FPS = 24;
    u64 frame_target = 1000000 / FPS;
    u64 frame_start = 0, frame_end = 0, frame_used = 0;
    
    timeBeginPeriod(1);
    win32.keep_playing = true;
    
    ShowCursor(FALSE);
    ShowWindow(win32.render_vars.hwnd, SW_SHOW);
    
    {
        glFlush();
        HDC dc = GetDC(win32.render_vars.hwnd);
        SwapBuffers(dc);
        ReleaseDC(win32.render_vars.hwnd, dc);
    }
    
    frame_start = win32_get_time();
    while (win32.keep_playing){
        MSG msg;
        
#ifdef DEVELOPER
        win32.dev_input.drop_count = 0;
        win32.file_drop_lock = 1;
        cd_memset(&win32.keys, 0, sizeof(win32.keys));
#endif
        
        while (PeekMessage(&msg,
                0,0,0,
                PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
#ifdef DEVELOPER
        win32.file_drop_lock = 0;
        
        win32_reload_renderer(path_string);
        win32_reload_game(path_string);
#endif
        
        Render_Target *dbg_target = 0;
        
#ifdef DEVELOPER
        dbg_target = &win32.dbg_target;
#endif
        
        BYTE keys[256];
        GetKeyboardState(keys);
        
        // NOTE(allen): MSDN says that the high order bit
        // indicates whether the key is down, other bits can
        // be set for apparently no reason just to screw you.
#define win32_down(b) (0x80 & b)
        
        win32.input.left_button.prev_down = win32.input.left_button.down;
        win32.input.right_button.prev_down = win32.input.right_button.down;
        
        win32.input.left_button.down = win32_down(keys[VK_LBUTTON]);
        win32.input.right_button.down = win32_down(keys[VK_RBUTTON]);
        
        for (i32 i = 0; i < 26; ++i){
            win32.input.letter_[i].prev_down = win32.input.letter_[i].down;
            win32.input.letter_[i].down = win32_down(keys['A' + i]);
        }
        win32.input.letter = win32.input.letter_ - 'A';
        
        for (i32 i = 0; i < 10; ++i){
            win32.input.number[i].prev_down = win32.input.number[i].down;
            win32.input.number[i].down = win32_down(keys['0' + i]);
        }
        
        win32.input.up.prev_down = win32.input.up.down;
        win32.input.down.prev_down = win32.input.down.down;
        win32.input.left.prev_down = win32.input.left.down;
        win32.input.right.prev_down = win32.input.right.down;
        
        win32.input.up.down = win32_down(keys[VK_UP]);
        win32.input.down.down = win32_down(keys[VK_DOWN]);
        win32.input.left.down = win32_down(keys[VK_LEFT]);
        win32.input.right.down = win32_down(keys[VK_RIGHT]);
        
        win32.input.esc.prev_down = win32.input.esc.down;
        win32.input.esc.down = win32_down(keys[VK_ESCAPE]);
        
        POINT cursor_point;
        GetCursorPos(&cursor_point);
        ScreenToClient(win32.render_vars.hwnd, &cursor_point);
        
        win32.input.mx = cursor_point.x;
        win32.input.my = win32.out_h - cursor_point.y;
        
        win32.input.dt = 1.f / FPS;
        
        Input_State input = win32.input;
        
#ifdef DEVELOPER
        win32.dev_input.input = input;
        
        for (i32 i = 0; i < 12; ++i){
            win32.dev_input.fkeys_[i].prev_down = win32.dev_input.fkeys_[i].down;
            win32.dev_input.fkeys_[i].down = win32_down(keys[VK_F1 + i]);
        }
        win32.dev_input.fkeys = win32.dev_input.fkeys_ - 1;
        
        if (win32.dev_input.fkeys[8].down && !win32.dev_input.fkeys[8].prev_down){
            if (win32.loop_stopped){
                win32.loop_stopped = 0;
            }
            else{
                cd_memcpy(win32.logical_backup_stopped, win32.memory.logical, LOGICAL_SIZE);
                win32.input_stopped = input;
                win32.loop_stopped = 1;
            }
        }
        
        // TODO(allen): replace memcpy with our own swankier version.
        if (win32.dev_input.fkeys[7].down && !win32.dev_input.fkeys[7].prev_down){
            if (win32.loop_stopped){
                win32.loop_stopped = 0;
            }
            else{
                switch (win32.loop_mode){
                    case 0:
                    cd_memcpy(win32.logical_backup, win32.memory.logical, LOGICAL_SIZE);
                    win32.loop_mode = 1;
                    break;
                    
                    case 1:
                    cd_memcpy(win32.memory.logical, win32.logical_backup, LOGICAL_SIZE);
                    win32.loop_mode = 2;
                    win32.loop_end = win32.loop_i;
                    win32.loop_i = 0;
                    break;
                    
                    case 2:
                    win32.loop_mode = 0;
                    break;
                }
            }
        }
        
        win32.dev_input.keys = win32.keys;
        
        if (win32.loop_stopped){
            cd_memcpy(win32.memory.logical, win32.logical_backup_stopped, LOGICAL_SIZE);
            input = win32.input_stopped;
        }
        else{
            if (win32.loop_mode == 1 && win32.loop_i == win32.max_inputs){
                cd_memcpy(win32.memory.logical, win32.logical_backup, LOGICAL_SIZE);
                win32.loop_mode = 2;
                win32.loop_end = win32.loop_i;
                win32.loop_i = 0;
            }
            
            switch (win32.loop_mode){
                case 1:
                win32.past_inputs[win32.loop_i] = input;
                ++win32.loop_i;
                break;
                
                case 2:
                if (win32.loop_i == win32.loop_end){
                    cd_memcpy(win32.memory.logical, win32.logical_backup, LOGICAL_SIZE);
                    win32.loop_i = 0;
                }
                input = win32.past_inputs[win32.loop_i];
                ++win32.loop_i;
                break;
            }
        }
#endif
        
        if (win32.app.step && win32.target.execute){
            win32.app.step(&win32.system,
                           win32.memory,
                           &win32.target,
                           dbg_target,
                           &win32.bank,
                           &input,
                           &win32.dev_input,
                           win32.loop_mode + win32.loop_stopped * 3);
        }
        
#ifdef DEVELOPER
        {
            GLenum error = glGetError();
            GLenum copy = error;
            AllowLocal(copy);
        }
#endif
        
        win32.target.display(&win32.render_vars);
        
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
