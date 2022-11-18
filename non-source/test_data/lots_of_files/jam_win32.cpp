/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Platform Layer: Win32.
 */

// TOP

// TODO(allen): memcpy sound samples for performance
// TODO(allen): migrate as much as possible to c standard lib

#include "jam_meta.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SOFTWARE 0
#define OPENGL 1

#define DISABLE_CONTROLLER 1

#define RENDER_MODE OPENGL

#if RENDER_MODE == OPENGL
#include <GL/glew.h>
#endif

#include "jam_system.h"
#include "jam_rendering_system.h"
#include "jam_rendering.h"
#include "jam.h"
#include "jam_math.cpp"
#include "jam_graphics.h"
#include "jam_graphics.cpp"
#include "jam_audio.h"
#include "jam_audio.cpp"
#include "gjCStyle.h"
#include "jam_game.h"
#include "jam.cpp"

#include <Windows.h>

#if !DISABLE_CONTROLLER
#include <XInput.h>
#endif

#include <DSound.h>

#define WINERROR(msg)

#include "jam_rendering_win32.cpp"

internal
SIG_SYS_LOAD_FILE(system_load_file){
	File result = {};
	HANDLE winfile = CreateFileA(filename, GENERIC_READ,
								 0, 0, OPEN_EXISTING,
								 FILE_ATTRIBUTE_NORMAL, 0);
	if (winfile){
		LARGE_INTEGER size;
		// TODO(allen): GetFileSizeEx is incompatible with windows store.
		if (GetFileSizeEx(winfile, &size)){
			result.size = size.QuadPart;
			result.data = VirtualAlloc(0, (i32)result.size,
									   MEM_COMMIT | MEM_RESERVE,
									   PAGE_READWRITE);
			u32 unread = (u32)result.size;
			while (unread != 0){
				DWORD read_size = Min(unread, max_u32);
				DWORD true_read_size;
				u8 *write_pos = (u8*)result.data;
				if (ReadFile(winfile, write_pos, read_size, &true_read_size, 0)){
					if (unread >= true_read_size){
						unread -= true_read_size;
						write_pos += true_read_size;
					}
					else{
						break;
					}
				}
				else{
					break;
				}
			}
			if (unread != 0){
				system_free_file(result);
				result = {};
			}
		}
		CloseHandle(winfile);
	}
	return result;
}

internal
SIG_SYS_SAVE_FILE(system_save_file){
	HANDLE winfile = CreateFileA(filename, GENERIC_WRITE,
								 0, 0, OPEN_ALWAYS,
								 FILE_ATTRIBUTE_NORMAL, 0);
	if (winfile){
		u32 unwritten = (u32)file.size;
		while (unwritten != 0){
			DWORD write_size = Min(unwritten, max_u32);
			DWORD true_write_size;
			u8 *src_pos = (u8*)file.data;
			if (ReadFile(winfile, src_pos, write_size, &true_write_size, 0)){
				if (unwritten >= true_write_size){
					unwritten -= true_write_size;
					src_pos += true_write_size;
				}
				else{
					break;
				}
			}
			else{
				break;
			}
		}
	}
}

internal
SIG_SYS_FREE_FILE(system_free_file){
	VirtualFree(file.data, 0, MEM_RELEASE);
}

#if !DISABLE_CONTROLLER
#define SIG_Input_Get_State(name) DWORD(name)(DWORD user_index, XINPUT_STATE *state)
#define SIG_Input_Set_State(name) DWORD(name)(DWORD user_index, XINPUT_VIBRATION *vibration)

typedef SIG_Input_Get_State(Input_Get_State);
typedef SIG_Input_Set_State(Input_Set_State);

struct Win32ControllerFunctions{
	bool32 is_valid;
	Input_Get_State *get_state;
	Input_Set_State *set_state;
};
#endif

#define SIG_Sound_Create(name) HRESULT(name)(LPGUID guid, LPDIRECTSOUND *ds, LPUNKNOWN unk);
typedef SIG_Sound_Create(Sound_Create);

struct Win32SoundFunctions{
	Sound_Create *create;
};

struct Win32Vars{
	Game_Memory memory;
	
	HWND winhandle;
	
	Win32RenderingVars render_vars;
	i32 min_width, min_height;
	i32 min_game_width, min_game_height;
	i32 pref_game_width, pref_game_height;
	
	bool32 is_fullscreen;
	bool32 has_prev_winrect;
	RECT prev_winrect;
	
	LPDIRECTSOUNDBUFFER sound_buffer;
	i32 samples_per_second;
	i32 bytes_per_sample;
	i32 sound_buffer_size;
	u32 sound_sample_index;
	i16* samples;
	i32 sound_safety_margin;
	
	i16 mx, my;
	
	i64 msecond_per_frame;
	real32 update_hz;
	bool32 keep_playing;
	bool32 first;
	
	Game_Input keyboard_input, controller_input;
    
#if 0
	Win32ControllerFunctions xin;
#endif
    
	bool32 toggle_full_screen;
    
    char btn1, btn2;
};

globalvar Win32Vars win32;

internal
SIS_SYS_SET_KEYBIND(system_set_keybind){
    Assert(key >= 'A' && key <= 'Z');
    if (btn_id == 1){
        win32.btn1 = key;
    }
    else if (btn_id == 2){
        win32.btn2 = key;
    }
}

#if 0
internal void
Win32InputSetup(){
	HMODULE xinput = 0;
	xinput = LoadLibrary("Xinput1_4.dll");
	if (!xinput){
		xinput = LoadLibrary("Xinput9_1_0.dll");
	}
	if (!xinput){
		xinput = LoadLibrary("Xinput1_3.dll");
	}
	if (xinput){
		win32.xin.get_state = (Input_Get_State*)GetProcAddress(xinput, "XInputGetState");
		win32.xin.set_state = (Input_Set_State*)GetProcAddress(xinput, "XInputSetState");
		
		if (!win32.xin.get_state || !win32.xin.set_state){
			win32.xin = {};
		}
		else{
			win32.xin.is_valid = 1;
		}
	}
}

struct Win32ControllerData{
    real32 lstick_x, lstick_y;
	real32 rstick_x, rstick_y;
	bool8 plugged_in;
	bool8 up, down, left, right;
	bool8 start, back;
	bool8 left_thumb, right_thumb;
	bool8 left_shoulder, right_shoulder;
	bool8 a, b, x, y;
	bool8 ltrigger, rtrigger;
};

inline internal void
Win32RealStick(real32 *stick, i16 input, i16 deadzone){
	if (input > deadzone){
		*stick = (input - deadzone) / (32767.f - deadzone);
	}
	else if (input < -deadzone){
		*stick = (input + deadzone) / (32768.f - deadzone);
	}
	else{
		*stick = 0;
	}
}

internal void
Win32ControllerGet(Win32ControllerData *data, i32 index){
	XINPUT_STATE state;
	DWORD success = win32.xin.get_state(index, &state);
	
	data->plugged_in = (success == ERROR_SUCCESS);
	if (success == ERROR_SUCCESS){
		data->up = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
		data->down = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
		data->left = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		data->right = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
		
		data->start = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_START);
		data->back = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_BACK);
		
		data->left_thumb = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
		data->right_thumb = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
		
		data->left_shoulder = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
		data->right_shoulder = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
		
		data->a = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_A);
		data->b = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_B);
		data->x = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_X);
		data->y = CheckFlag(state.Gamepad.wButtons, XINPUT_GAMEPAD_Y);
		
		if (state.Gamepad.bLeftTrigger >= 224){
			data->ltrigger = 1;
		}
		else{
			data->ltrigger = 0;
		}
		
		if (state.Gamepad.bRightTrigger >= 224){
			data->rtrigger = 1;
		}
		else{
			data->rtrigger = 0;
		}
		
		Win32RealStick(&data->lstick_x, state.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		Win32RealStick(&data->lstick_y, state.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		
		Win32RealStick(&data->rstick_x, state.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		Win32RealStick(&data->rstick_y, state.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		
	}
}

internal bool32
Win32ControllerRumble(i32 index, real32 left, real32 right){
	XINPUT_VIBRATION vals;
	vals.wLeftMotorSpeed = (u16)(left*max_u16);
	vals.wRightMotorSpeed = (u16)(right*max_u16);
	DWORD success = win32.xin.set_state(index, &vals);
	return (success == ERROR_SUCCESS);
}
#endif

internal Win32SoundFunctions
Win32SoundSetup(i32 samples_per_second, i32 buffer_size){
	Win32SoundFunctions funcs = {};
	HMODULE dsound = LoadLibrary("dsound.dll");
	if (dsound){
		funcs.create = (Sound_Create*)GetProcAddress(dsound, "DirectSoundCreate");
		if (funcs.create){
			LPDIRECTSOUND dsound;
			if (SUCCEEDED(funcs.create(0, &dsound, 0))){
				WAVEFORMATEX wave_format = {};
				wave_format.wFormatTag = WAVE_FORMAT_PCM;
				wave_format.nChannels = 2;
				wave_format.nSamplesPerSec = samples_per_second;
				wave_format.wBitsPerSample = 16;
				wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
				wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec*wave_format.nBlockAlign;
				wave_format.cbSize = 0;
				
				if (SUCCEEDED(dsound->SetCooperativeLevel(win32.winhandle, DSSCL_PRIORITY))){
					DSBUFFERDESC buffer_description = {};
					buffer_description.dwSize = sizeof(buffer_description);
					buffer_description.dwFlags = DSBCAPS_PRIMARYBUFFER;
					
					LPDIRECTSOUNDBUFFER buffer;
					if (SUCCEEDED(dsound->CreateSoundBuffer(&buffer_description, &buffer, 0))){
						if (buffer->SetFormat(&wave_format)){
							// NOTE(allen): SUCCESS - set primary buffer format
						}
						else{
							// NOTE(allen): FAIL - set primary buffer format
						}
					}
					else{
						// NOTE(allen): FAIL - create primary buffer
					}
				}
				else{
					// NOTE(allen): FAIL - set cooperative level
				}
				
				DSBUFFERDESC buffer_description = {};
				buffer_description.dwSize = sizeof(buffer_description);
				buffer_description.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
				buffer_description.dwBufferBytes = buffer_size;
				buffer_description.lpwfxFormat = &wave_format;
				if (SUCCEEDED(dsound->CreateSoundBuffer(&buffer_description, &win32.sound_buffer, 0))){
					// NOTE(allen): SUCCESS - create secondary buffer
				}
				else{
					// NOTE(allen): FAIL - create secondary buffer
				}
			}
			else{
				// NOTE(allen): FAIL - create direct sound interface
			}
		}
		else{
			// NOTE(allen): FAIL - load direct sound creator function
		}
	}
	else{
		// NOTE(allen): FAIL - load direct sound library
	}
	
	return funcs;
}

internal void
Win32ClearSound(){
	VOID *chunk1, *chunk2;
	DWORD size1, size2;
	if (SUCCEEDED(win32.sound_buffer->Lock(0, win32.sound_buffer_size,
										   &chunk1, &size1, &chunk2, &size2,
										   0))){
		u8 *dest = (u8*)chunk1;
		if (dest){
			for (DWORD i = 0; i < size1; ++i){
				*dest = 0;
				++dest;
			}
		}
		
		dest = (u8*)chunk2;
		
		if (dest){
			for (DWORD i = 0; i < size1; ++i){
				*dest = 0;
				++dest;
			}
		}
		
		win32.sound_buffer->Unlock(chunk1, size1, chunk2, size2);
	}
}

internal void
Win32FillSound(DWORD write_start, DWORD write_size, i16 *source){
	VOID *chunk1, *chunk2;
	DWORD size1, size2;
	if (SUCCEEDED(win32.sound_buffer->Lock(write_start, write_size,
										   &chunk1, &size1, &chunk2, &size2,
										   0))){
		DWORD samples = size1/win32.bytes_per_sample;
		i16 *dest = (i16*)chunk1;
		if (dest){
			for (DWORD i = 0; i < samples; ++i){
				*dest++ = *source++;
				*dest++ = *source++;
				++win32.sound_sample_index;
			}
		}
		
		samples = size2/win32.bytes_per_sample;
		dest = (i16*)chunk2;
		if (dest){
			for (DWORD i = 0; i < samples; ++i){
				*dest++ = *source++;
				*dest++ = *source++;
				++win32.sound_sample_index;
			}
		}
		
		win32.sound_buffer->Unlock(chunk1, size1, chunk2, size2);
	}
}

inline internal void
Win32UpdateKey(bool32 press, bool32 release, bool8 *key){
	if (press){
		*key = 1;
	}
	if (release){
		*key = 0;
	}
}

internal LRESULT
Win32Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	LRESULT result = {};
	switch (uMsg){
        
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			bool32 prev, current;
			prev = ((lParam & Bit_30)?(1):(0));
			current = ((lParam & Bit_31)?(0):(1));
            
			bool32 press = current && !prev;
			bool32 release = prev && !current;
            
			switch (wParam){
            case VK_UP:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.digital.up);
            }break;
            case VK_DOWN:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.digital.down);
            }break;
            
            case VK_LEFT:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.digital.left);
            }break;
            case VK_RIGHT:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.digital.right);
            }break;
            
            case VK_SPACE:
            case VK_RETURN:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.button[0]);
                
                if (uMsg == WM_KEYDOWN){
                    win32.keyboard_input.key_input = 1;
                    win32.keyboard_input.key_code = 1;
                }
            }break;
            
            case VK_BACK:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.back);
                
                if (uMsg == WM_KEYDOWN){
                    win32.keyboard_input.key_input = 1;
                    win32.keyboard_input.key_code = 0;
                }
            }break;
            case VK_ESCAPE:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.pause);
            }break;
            case VK_F2:
            {
                Win32UpdateKey(press, release, &win32.keyboard_input.toggle_edit_mode);
            }break;
            case VK_SHIFT: break;
            case VK_CONTROL:
            {
#if 0
                if (press){
                    win32.toggle_full_screen = 1;
                }
#endif
            }break;
            case VK_MENU:
            {
                win32.first = 1;
            }break; /*ALT*/
            
            default:
            {
                if ((char)wParam == win32.btn1){
                    Win32UpdateKey(press, release, &win32.keyboard_input.button[1]);
                }
                
                if ((char)wParam == win32.btn2){
                    Win32UpdateKey(press, release, &win32.keyboard_input.button[2]);
                }
                
                if (uMsg == WM_KEYDOWN){
                    if ((wParam >= '0' && wParam <= '9') ||
                        (wParam >= 'A' && wParam <= 'Z')){
                        win32.keyboard_input.key_input = 1;
                        win32.keyboard_input.key_code = (char)wParam;
                    }
                }
            }break;
			}
		}break;
		
		case WM_LBUTTONDOWN:
		{
			win32.mx = LOWORD(lParam);
			win32.my = HIWORD(lParam);
		}break;
		
		case WM_LBUTTONUP:
		{
			win32.mx = LOWORD(lParam);
			win32.my = HIWORD(lParam);
		}break;
		
		case WM_RBUTTONDOWN:
		{
			win32.mx = LOWORD(lParam);
			win32.my = HIWORD(lParam);
		}break;
		
		case WM_RBUTTONUP:
		{
			win32.mx = LOWORD(lParam);
			win32.my = HIWORD(lParam);
		}break;
		
		case WM_MOUSEMOVE:
		{
			win32.mx = LOWORD(lParam);
			win32.my = HIWORD(lParam);
		}break;
		
		case WM_SIZE:
		{
			i32 w, h;
			w = LOWORD(lParam);
			h = HIWORD(lParam);
			render_set_screen_size(&win32.render_vars, w, h, win32.pref_game_width, win32.pref_game_height);
		}break;
		
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = win32.min_width;
			mmi->ptMinTrackSize.y = win32.min_height;
			return 0;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps); AllowLocal(hdc);
			//win32.render_vars.hdc = hdc;
			render_redraw_screen(&win32.render_vars);
			EndPaint(hwnd, &ps);
		}break;
		
		case WM_CLOSE:
		case WM_DESTROY:
		{
			win32.keep_playing = 0;
		}break;
		
		case WM_NCLBUTTONDOWN:
		{
			DWORD status;
			win32.sound_buffer->GetStatus(&status);
			if(status & DSBSTATUS_PLAYING){
				win32.sound_buffer->Stop();
			}
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}break;
		
		case WM_ENTERSIZEMOVE:
		{
			DWORD status;
			win32.sound_buffer->GetStatus(&status);
			if(status & DSBSTATUS_PLAYING){
				win32.sound_buffer->Stop();
			}
		}break;
		
		case WM_EXITSIZEMOVE:
		{
			DWORD status;
			win32.sound_buffer->GetStatus(&status);
			if(!(status & DSBSTATUS_PLAYING)){
				win32.sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
			}
		}break;
		
		default:
		{
			result = DefWindowProc(hwnd, uMsg, wParam, lParam);
		}break;
	}
	return result;
}

internal i64
Win32GetTime(){
	i64 result = 0;
	LARGE_INTEGER time;
	if (QueryPerformanceCounter(&time)){
		result = (i64)time.QuadPart;
	}
	return result;
}

internal bool32
Win32GoFullscreen(){
	HMONITOR monitor = MonitorFromWindow(win32.winhandle, MONITOR_DEFAULTTOPRIMARY);
	
	if (!monitor){
		WINERROR("Failed to get monitor handle");
		return 0;
	}
	
	MONITORINFO monitor_info;
	monitor_info.cbSize = sizeof(monitor_info);
	
	if (!GetMonitorInfo(monitor, &monitor_info)){
		WINERROR("Failed to get monitor information");
		return 0;
	}
	
	if (!GetWindowRect(win32.winhandle, &win32.prev_winrect)){
		WINERROR("Failed to save previous window size");
	}
	else{
		win32.has_prev_winrect = 1;
	}
	
	SetWindowLongPtr(win32.winhandle, GWL_STYLE,
					 WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE);
	MoveWindow(win32.winhandle,
			   monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
			   monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
			   monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
			   TRUE);
	
	win32.is_fullscreen = 1;
	
	return 1;
}

internal bool32
Win32GoWindowed(){
	SetWindowLongPtr(win32.winhandle, GWL_STYLE,
					 WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	if (win32.has_prev_winrect){
		MoveWindow(win32.winhandle,
				   win32.prev_winrect.left, win32.prev_winrect.top,
				   win32.prev_winrect.right - win32.prev_winrect.left,
				   win32.prev_winrect.bottom - win32.prev_winrect.top,
				   TRUE);
	}
	else{
		
		RECT winrect = {};
		
		winrect.right = win32.min_game_width;
		winrect.bottom = win32.min_game_height;
		
		if (!AdjustWindowRect(&winrect, WS_OVERLAPPEDWINDOW, 0)){
			return 0;
		}
		
		i32 win_x, win_y;
		win_x = 0;
		win_y = 0;
		
		// TODO(allen): This is now duplicated... so fix that?
		HMONITOR monitor = MonitorFromWindow(win32.winhandle, MONITOR_DEFAULTTOPRIMARY);
	
		if (!monitor){
			WINERROR("Failed to get monitor handle");
		}
		else{
	
			MONITORINFO monitor_info;
			monitor_info.cbSize = sizeof(monitor_info);
	
			if (!GetMonitorInfo(monitor, &monitor_info)){
				WINERROR("Failed to get monitor information");
			}
			else{
				// TODO: This will be easier when the get monitor info stuff
				// is pulled into it's own function so that it is only a matter
				// of seeing the success or failure of that one call.
				win_x =
					monitor_info.rcMonitor.right -
					monitor_info.rcMonitor.left -
					(winrect.right - winrect.left);
				
				win_y =
					monitor_info.rcMonitor.bottom -
					monitor_info.rcMonitor.top -
					(winrect.bottom - winrect.top);
				
				win_x /= 2;
				win_y /= 2;
			}
		}
		
		MoveWindow(win32.winhandle,
				   win_x, win_y,
				   winrect.right - winrect.left,
				   winrect.bottom - winrect.top,
				   TRUE);
	}
	return 1;
}

int __stdcall 
WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow){
	win32 = {};
    
    win32.btn1 = 'Z';
    win32.btn2 = 'X';
    
	win32.update_hz = 30.f;
	win32.msecond_per_frame = (i64)(1000.f/win32.update_hz);
	win32.min_game_width = 800;
	win32.min_game_height = 600;
	win32.pref_game_width = 800;
	win32.pref_game_height = 600;
	
	WNDCLASSEX winclass = {};
	winclass.cbSize = sizeof(WNDCLASSEX);
#if RENDER_MODE == SOFTWARE
	winclass.style = CS_HREDRAW | CS_VREDRAW;
#elif RENDER_MODE == OPENGL
	winclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
#endif
	winclass.lpfnWndProc = (WNDPROC)&Win32Proc;
	winclass.hInstance = hInstance;
	winclass.hIcon = 0; // TODO(allen): Setup this icon!
	winclass.hCursor = 0;
	winclass.lpszClassName = "TMUND-win-class";
	winclass.hIconSm = 0;

#if !DISABLE_CONTROLLER
	Win32InputSetup();
#endif
	
	ATOM classatom;
	classatom = RegisterClassEx(&winclass);
	
	if (!classatom){
		WINERROR("Failed to setup window class");
		return 0;
	}
	
	RECT winrect = {};
	
	winrect.right = win32.min_game_width;
	winrect.bottom = win32.min_game_height;
	
	if (!AdjustWindowRect(&winrect, WS_OVERLAPPEDWINDOW, 0)){
		WINERROR("Failed to compute window size");
		return 0;
	}
	
	HWND winhandle;
	winhandle = CreateWindow(winclass.lpszClassName,
							 "THE MOST UNDEAD", //NOTE(allen): Title
							 WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE
							 ,
							 CW_USEDEFAULT, CW_USEDEFAULT,
							 winrect.right - winrect.left,
							 winrect.bottom - winrect.top,
							 0, 0, hInstance, 0
							 );
	
	win32.min_width = winrect.right - winrect.left;
	win32.min_height = winrect.bottom - winrect.top;
	
	if (!winhandle){
		WINERROR("Failed to setup window");
		return 0;
	}
	
	win32.winhandle = winhandle;
	win32.render_vars.hdc = GetDC(win32.winhandle);
    
	render_startup(&win32.render_vars);
	//ReleaseDC(win32.winhandle, win32.render_vars.hdc);
	
	GetClientRect(winhandle, &winrect);
	
	render_set_screen_size(&win32.render_vars,
						   winrect.right - winrect.left,
						   winrect.bottom - winrect.top,
						   win32.min_game_width, win32.min_game_height);

#if RENDER_MODE == SOFTWARE
	ShowWindow(win32.winhandle, SW_SHOW);
	UpdateWindow(win32.winhandle);
#endif
	
	win32.samples_per_second = 48000;
	win32.bytes_per_sample = sizeof(i16)*2;
	win32.sound_buffer_size = win32.samples_per_second*win32.bytes_per_sample;
	win32.sound_safety_margin = (i32)
		(win32.samples_per_second*win32.bytes_per_sample/(win32.update_hz*3.f));
    win32.sound_safety_margin = LargeRoundUp(win32.sound_safety_margin, win32.bytes_per_sample);
	Win32SoundSetup(win32.samples_per_second, win32.sound_buffer_size);
	Win32ClearSound();
	win32.sound_buffer->Play(0, 0, DSBPLAY_LOOPING);
	
	win32.samples = (i16*)VirtualAlloc(0, win32.sound_buffer_size,
									   MEM_COMMIT | MEM_RESERVE,
									   PAGE_READWRITE);
	
	if (!win32.samples){
		WINERROR("Failed to setup audio buffer memory");
		return 0;
	}
	
	Game_Memory memory;
	memory.size = Mbytes(256);
	memory.data = VirtualAlloc(0, memory.size,
							   MEM_COMMIT | MEM_RESERVE,
							   PAGE_READWRITE);
	win32.memory = memory;
	
	bool32 good_sound = 0;
	win32.keep_playing = 1;
	timeBeginPeriod(1);
	win32.first = 1;
	MSG msg;
	i64 prev_work_time = Win32GetTime();
	i64 flip_time = prev_work_time;
	i64 start_time = prev_work_time;
	i64 end_time = prev_work_time;
    AllowLocal(start_time);
    AllowLocal(end_time);
    
	while (win32.keep_playing){
		win32.toggle_full_screen = 0;
        win32.keyboard_input.key_input = 0;
		while (PeekMessageA(&msg, winhandle, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

#if !DISABLE_CONTROLLER
		Win32ControllerData controller = {};
		if (win32.xin.is_valid){
			// TODO(allen): Multiple controller support?
			Win32ControllerGet(&controller, 0);
		}
		if (controller.plugged_in){
			if (controller.lstick_x != 0.f ||
				controller.lstick_y != 0.f){
				win32.controller_input.is_analog = 1;
				win32.controller_input.analog.stick_x = controller.lstick_x;
				win32.controller_input.analog.stick_y = -controller.lstick_y;
			}
			else{
				win32.controller_input.is_analog = 0;
				win32.controller_input.digital.right = controller.right;
				win32.controller_input.digital.left = controller.left;
				win32.controller_input.digital.up = controller.up;
				win32.controller_input.digital.down = controller.down;
			}
			
			win32.controller_input.button[0] = controller.a;
			win32.controller_input.button[1] = controller.b;
			win32.controller_input.button[2] = controller.x;
			win32.controller_input.button[3] = controller.y;
			win32.controller_input.button[4] = controller.left_shoulder;
			win32.controller_input.button[5] = controller.right_shoulder;
			win32.controller_input.button[6] = controller.ltrigger;
			win32.controller_input.button[7] = controller.rtrigger;
			
			win32.controller_input.pause = controller.start;
			win32.controller_input.back = controller.back;
		}
#endif
        
		Game_Step_Target step_target;
		step_target.render = render_get_render_target(&win32.render_vars);
		step_target.can_fullscreen = 1;
		step_target.is_fullscreen = &win32.is_fullscreen;
		step_target.audio_samples_per_second = win32.samples_per_second;
		step_target.second_per_frame = 1.f/win32.update_hz;
        step_target.btn1 = win32.btn1;
        step_target.btn2 = win32.btn2;
        
		bool32 was_fullscreen = win32.is_fullscreen;
		
#define ComposeControls(name) input.name = win32.keyboard_input.name || win32.controller_input.name
		
		Game_Input input;
		if (win32.controller_input.is_analog){
			input.is_analog = 1;
			input.analog.stick_x = win32.controller_input.analog.stick_x;
			input.analog.stick_y = win32.controller_input.analog.stick_y;
		}
		else{
			input.is_analog = 0;
			ComposeControls(digital.up);
			ComposeControls(digital.down);
			ComposeControls(digital.left);
			ComposeControls(digital.right);
		}
		
		ComposeControls(button[0]);
		ComposeControls(button[1]);
		ComposeControls(button[2]);
		ComposeControls(button[3]);
		ComposeControls(button[4]);
		ComposeControls(button[5]);
		ComposeControls(button[6]);
		ComposeControls(button[7]);
        
		ComposeControls(pause);
		ComposeControls(back);
        
		ComposeControls(toggle_edit_mode);
        
        input.key_input = win32.keyboard_input.key_input;
        input.key_code = win32.keyboard_input.key_code;
        
		if (game_step(step_target, input, memory, win32.first)){
			win32.keep_playing = 0;
		}
		
		if (win32.toggle_full_screen){
			win32.is_fullscreen = !was_fullscreen;
		}
		
		win32.first = 0;
		
		if (win32.is_fullscreen && !was_fullscreen){
			if (!Win32GoFullscreen()){
				win32.is_fullscreen = 0;
			}
		}
		else if (!win32.is_fullscreen && was_fullscreen){
			Win32GoWindowed();
		}
		
		i64 audio_time = Win32GetTime();
		i64 time_ellapsed_before_audio = audio_time - flip_time;
		
		DWORD play_cursor, write_cursor;
		if (win32.sound_buffer->GetCurrentPosition(&play_cursor, &write_cursor) == DS_OK){
			if (!good_sound){
				win32.sound_sample_index = write_cursor / win32.bytes_per_sample;
				good_sound = 1;
			}
			
			DWORD write_start =
				((win32.sound_sample_index*win32.bytes_per_sample) % win32.sound_buffer_size);
			
			DWORD expected_bytes_per_frame = (DWORD)
				(win32.bytes_per_sample*win32.samples_per_second*win32.msecond_per_frame/1000);
			
			i64 useconds_til_flip = (win32.msecond_per_frame*1000 - time_ellapsed_before_audio);
			DWORD expected_bytes_til_flip = (DWORD)
				(win32.bytes_per_sample*win32.samples_per_second*useconds_til_flip/1000000);
			
			DWORD expected_frame_bounary = play_cursor + expected_bytes_til_flip;
			
			DWORD safe_cursor = write_cursor;
			if (safe_cursor < play_cursor){
				safe_cursor += win32.sound_buffer_size;
			}
			
			safe_cursor += win32.sound_safety_margin;
			
			bool32 low_latency = (safe_cursor < expected_frame_bounary);
			
			DWORD target = 0;
			if (low_latency){
				target = (expected_frame_bounary + expected_bytes_per_frame);
			}
			else{
				target = (write_cursor + expected_bytes_per_frame + win32.sound_safety_margin);
			}
			target = target % win32.sound_buffer_size;
			
			DWORD write_size = 0;
			if (write_start > target){
				write_size = win32.sound_buffer_size - write_start + target;
			}
			else{
				write_size = target - write_start;
			}
			
			Game_Audio_Target audio_target;
			audio_target.samples = win32.samples;
			audio_target.count_per_channel = write_size / win32.bytes_per_sample;
            audio_target.channels = 2;
			audio_target.samples_per_second = win32.samples_per_second;
			audio_target.second_per_frame = 1.f/win32.update_hz;
			
			game_fill_samples(audio_target, memory);
			
			Win32FillSound(write_start, write_size, win32.samples);
		}
		else{
			good_sound = 0;
		}
		
		i64 work_time = Win32GetTime();
		i64 run_time = ((work_time - prev_work_time)/1000);
		if (run_time < win32.msecond_per_frame){
			Sleep((DWORD)(win32.msecond_per_frame - run_time));
			
			work_time = Win32GetTime();
			if ((DWORD)((work_time - prev_work_time)/1000) < win32.msecond_per_frame){
				// NOTE(allen): MINOR - sleep did not work
			}
			while ((DWORD)((work_time - prev_work_time)/1000) < win32.msecond_per_frame){
				work_time = Win32GetTime();
			}
		}
		else{
			// NOTE(allen): FAIL - missed frame rate
            int x = 0;
            AllowLocal(x);
		}
		prev_work_time = Win32GetTime();
		
		//win32.render_vars.hdc = GetDC(win32.winhandle);
		render_redraw_screen(&win32.render_vars);
		//ReleaseDC(win32.winhandle, win32.render_vars.hdc);
		
		flip_time = Win32GetTime();
	}
	
	// TODO(allen): Must we do this?  We know we're ending, seems like
	// this might be handled by OS just fine.
	//win32.render_vars.hdc = GetDC(win32.winhandle);
	render_shutdown(&win32.render_vars);
	//ReleaseDC(win32.winhandle, win32.render_vars.hdc);
	
	return 0;
}

// BOTTOM

