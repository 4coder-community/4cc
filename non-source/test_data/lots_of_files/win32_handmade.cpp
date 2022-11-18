/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  TODO(casey):  THIS IS NOT A FINAL PLATFORM LAYER!!!
  
  - Make the right calls so Windows doesn't think we're "still loading" for a bit after we actually start
  - Saved game locations
  - Getting a handle to our own executable file
  - Asset loading path
  - Threading (launch a thread)
  - Raw Input (support for multiple keyboards)
  - ClipCursor() (for multimonitor support)
  - QueryCancelAutoplay
  - WM_ACTIVATEAPP (for when we are not the active application)
  - Blit speed improvements (BitBlt)
  - Hardware acceleration (OpenGL or Direct3D or BOTH??)
  - GetKeyboardLayout (for French keyboards, international WASD support)
  - ChangeDisplaySettings option if we detect slow fullscreen blit??

   Just a partial list of stuff!!
*/

#include "handmade_platform.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>

#include "win32_handmade.h"

// TODO(casey): This is a global for now.
global_variable bool32 GlobalRunning;
global_variable bool32 GlobalPause;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalPerfCountFrequency;
global_variable bool32 DEBUGGlobalShowCursor;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(casey): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }

    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0;
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

#if HANDMADE_INTERNAL
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    // NOTE(casey): File read successfully
                    Result.ContentsSize = FileSize32;
                }
                else
                {                    
                    // TODO(casey): Logging
                    DEBUGPlatformFreeFileMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            // NOTE(casey): File read successfully
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            // TODO(casey): Logging
        }

        CloseHandle(FileHandle);
    }
    else
    {
        // TODO(casey): Logging
    }

    return(Result);
}

DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(DEBUGExecuteSystemCommand)
{
    debug_executing_process Result = {};
    
    STARTUPINFO StartupInfo = {};
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION ProcessInfo = {};    
    if(CreateProcess(Command,
                     CommandLine,
                     0,
                     0,
                     FALSE,
                     0,
                     0,
                     Path,
                     &StartupInfo,
                     &ProcessInfo))
    {
        Assert(sizeof(Result.OSHandle) >= sizeof(ProcessInfo.hProcess));
        *(HANDLE *)&Result.OSHandle = ProcessInfo.hProcess;
    }
    else
    {
        DWORD ErrorCode = GetLastError();
        *(HANDLE *)&Result.OSHandle = INVALID_HANDLE_VALUE;
    }

    return(Result);
}

DEBUG_PLATFORM_GET_PROCESS_STATE(DEBUGGetProcessState)
{
    debug_process_state Result = {};

    HANDLE hProcess = *(HANDLE *)&Process.OSHandle;
    if(hProcess != INVALID_HANDLE_VALUE)
    {
        Result.StartedSuccessfully = true;

        if(WaitForSingleObject(hProcess, 0) == WAIT_OBJECT_0)
        {
            DWORD ReturnCode = 0;
            GetExitCodeProcess(hProcess, &ReturnCode);
            Result.ReturnCode = ReturnCode;
            CloseHandle(hProcess);
        }
        else
        {
            Result.IsRunning = true;
        }
    }

    return(Result);
}
#endif

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_game_code Result = {};

    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);

        CopyFile(SourceDLLName, TempDLLName, FALSE);
    
        Result.GameCodeDLL = LoadLibraryA(TempDLLName);
        if(Result.GameCodeDLL)
        {
            Result.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
        
            Result.GetSoundSamples = (game_get_sound_samples *)
                GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
        
            Result.DEBUGFrameEnd = (debug_game_frame_end *)
                GetProcAddress(Result.GameCodeDLL, "DEBUGGameFrameEnd");

            Result.IsValid = (Result.UpdateAndRender &&
                              Result.GetSoundSamples);
        }
    }

    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }

    return(Result);
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{    
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }

    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}

internal void
Win32LoadXInput(void)    
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}

        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}

        // TODO(casey): Diagnostic

    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(casey): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(casey): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        // TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;

            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;

                // NOTE(casey): "Create" a primary buffer
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(casey): Diagnostic
                    }
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            else
            {
                // TODO(casey): Diagnostic
            }

            // TODO(casey): In release mode, should we not specify DSBCAPS_GLOBALFOCUS?

            // TODO(casey): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
#if HANDMADE_INTERNAL
            BufferDescription.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(casey): Diagnostic
        }
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return(Result);
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(casey): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;

    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;

    // NOTE(casey): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    // NOTE(casey): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    Buffer->Pitch = Align16(Width*BytesPerPixel);
    int BitmapMemorySize = (Buffer->Pitch*Buffer->Height);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    // TODO(casey): Probably clear this to black
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO(casey): Centering / black bars?
    
    if((WindowWidth >= Buffer->Width*2) &&
       (WindowHeight >= Buffer->Height*2))
    {
        StretchDIBits(DeviceContext,
                      0, 0, 2*Buffer->Width, 2*Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
#if 0
        int OffsetX = 10;
        int OffsetY = 10;

        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);
#else
        int OffsetX = 0;
        int OffsetY = 0;
#endif
    
        // NOTE(casey): For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!
        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{       
    LRESULT Result = 0;

    switch(Message)
    {
        case WM_CLOSE:
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;

        case WM_SETCURSOR:
        {
            if(DEBUGGlobalShowCursor)
            {
                Result = DefWindowProcA(Window, Message, WParam, LParam);
            }
            else
            {
                SetCursor(0);
            }
        } break;
        
        case WM_ACTIVATEAPP:
        {
#if 0
            if(WParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;

        case WM_DESTROY:
        {
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                       Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        } break;

        default:
        {
//            OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    // TODO(casey): More strenuous test!
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid

        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;

    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }

    return(Result);
}

internal void
Win32GetInputFileLocation(win32_state *State, bool32 InputStream,
                          int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildEXEPathFileName(State, Temp, DestCount, Dest);
}

internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *State, int unsigned Index)
{
    Assert(Index > 0);
    Assert(Index < ArrayCount(State->ReplayBuffers));
    win32_replay_buffer *Result = &State->ReplayBuffers[Index];
    return(Result);
}

internal void
Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputRecordingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputRecordingIndex = InputRecordingIndex;

        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
        State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(ReplayBuffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndRecordingInput(win32_state *State)
{
    CloseHandle(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
Win32BeginInputPlayBack(win32_state *State, int InputPlayingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputPlayingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputPlayingIndex = InputPlayingIndex;

        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
        State->PlaybackHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);

#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(State->GameMemoryBlock, ReplayBuffer->MemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndInputPlayBack(win32_state *State)
{
    CloseHandle(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)    
{
    DWORD BytesWritten;
    WriteFile(State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlayBackInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesRead = 0;
    if(ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
    {
        if(BytesRead == 0)
        {
            // NOTE(casey): We've hit the end of the stream, go back to the beginning
            int PlayingIndex = State->InputPlayingIndex;
            Win32EndInputPlayBack(State);
            Win32BeginInputPlayBack(State, PlayingIndex);
            ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
        }
    }
}

internal void
ToggleFullscreen(HWND Window)
{
    // NOTE(casey): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;

                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    else if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if(VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    else if(VKCode == 'L')
                    {
                        if(IsDown)
                        {
                            if(State->InputPlayingIndex == 0)
                            {
                                if(State->InputRecordingIndex == 0)
                                {
                                    Win32BeginRecordingInput(State, 1);
                                }
                                else
                                {
                                    Win32EndRecordingInput(State);
                                    Win32BeginInputPlayBack(State, 1);
                                }
                            }
                            else
                            {
                                Win32EndInputPlayBack(State);
                            }
                        }
                    }
#endif
                    if(IsDown)
                    {
                        bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if(Message.hwnd)
                            {
                                ToggleFullscreen(Message.hwnd);
                            }
                        }
                    }
                }

            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

#if 0
internal void
HandleDebugCycleCounters(game_memory *Memory)
{
#if HANDMADE_INTERNAL
    OutputDebugStringA("DEBUG CYCLE COUNTS:\n");
    for(int CounterIndex = 0;
        CounterIndex < ArrayCount(Memory->Counters);
        ++CounterIndex)
    {
        debug_cycle_counter *Counter = Memory->Counters + CounterIndex;

        if(Counter->HitCount)
        {
            char TextBuffer[256];
            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                        "  %d: %I64ucy %uh %I64ucy/h\n",
                        CounterIndex,
                        Counter->CycleCount,
                        Counter->HitCount,
                        Counter->CycleCount / Counter->HitCount);
            OutputDebugStringA(TextBuffer);
            Counter->HitCount = 0;
            Counter->CycleCount = 0;
        }
    }
#endif
}
#endif

#if 0

internal void
Win32DebugDrawVertical(win32_offscreen_buffer *Backbuffer,
                       int X, int Top, int Bottom, uint32 Color)
{
    if(Top <= 0)
    {
        Top = 0;
    }

    if(Bottom > Backbuffer->Height)
    {
        Bottom = Backbuffer->Height;
    }
    
    if((X >= 0) && (X < Backbuffer->Width))
    {
        uint8 *Pixel = ((uint8 *)Backbuffer->Memory +
                        X*Backbuffer->BytesPerPixel +
                        Top*Backbuffer->Pitch);
        for(int Y = Top;
            Y < Bottom;
            ++Y)
        {
            *(uint32 *)Pixel = Color;
            Pixel += Backbuffer->Pitch;
        }
    }
}

inline void
Win32DrawSoundBufferMarker(win32_offscreen_buffer *Backbuffer,
                           win32_sound_output *SoundOutput,
                           real32 C, int PadX, int Top, int Bottom,
                           DWORD Value, uint32 Color)
{
    real32 XReal32 = (C * (real32)Value);
    int X = PadX + (int)XReal32;
    Win32DebugDrawVertical(Backbuffer, X, Top, Bottom, Color);
}

internal void
Win32DebugSyncDisplay(win32_offscreen_buffer *Backbuffer,
                      int MarkerCount, win32_debug_time_marker *Markers,
                      int CurrentMarkerIndex,
                      win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;

    int LineHeight = 64;
    
    real32 C = (real32)(Backbuffer->Width - 2*PadX) / (real32)SoundOutput->SecondaryBufferSize;
    for(int MarkerIndex = 0;
        MarkerIndex < MarkerCount;
        ++MarkerIndex)
    {
        win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);

        DWORD PlayColor = 0xFFFFFFFF;
        DWORD WriteColor = 0xFFFF0000;
        DWORD ExpectedFlipColor = 0xFFFFFF00;
        DWORD PlayWindowColor = 0xFFFF00FF;

        int Top = PadY;
        int Bottom = PadY + LineHeight;
        if(MarkerIndex == CurrentMarkerIndex)
        {
            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            int FirstTop = Top;
            
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColor);

            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);

            Top += LineHeight+PadY;
            Bottom += LineHeight+PadY;

            Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, FirstTop, Bottom, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
        }        
        
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor + 480*SoundOutput->BytesPerSample, PlayWindowColor);
        Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColor);
    }
}

#endif

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    uint32 volatile CompletionGoal;
    uint32 volatile CompletionCount;
    
    uint32 volatile NextEntryToWrite;
    uint32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;

    platform_work_queue_entry Entries[256];
};

internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal bool32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;

    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {        
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }

    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }

    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    platform_work_queue *Queue = (platform_work_queue *)lpParameter;

    u32 TestThreadID = GetThreadID();
    Assert(TestThreadID == GetCurrentThreadId());
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(Queue))
        {
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }

//    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}

internal void
Win32MakeQueue(platform_work_queue *Queue, uint32 ThreadCount)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;

    uint32 InitialCount = 0;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0,
                                               InitialCount,
                                               ThreadCount,
                                               0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

struct win32_platform_file_handle
{
    HANDLE Win32Handle;
};

struct win32_platform_file_group
{
    HANDLE FindHandle;
    WIN32_FIND_DATAW FindData;
};

internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(Win32GetAllFilesOfTypeBegin)
{
    platform_file_group Result = {};
    
    // TODO(casey): If we want, someday, make an actual arena used by Win32
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)VirtualAlloc(
        0, sizeof(win32_platform_file_group),
        MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Result.Platform = Win32FileGroup;

    wchar_t *WildCard = L"*.*";
    switch(Type)
    {
        case PlatformFileType_AssetFile:
        {
            WildCard = L"*.hha";
        } break;

        case PlatformFileType_SavedGameFile:
        {
            WildCard = L"*.hhs";
        } break;

        InvalidDefaultCase;
    }
    
    Result.FileCount = 0;

    WIN32_FIND_DATAW FindData;
    HANDLE FindHandle = FindFirstFileW(WildCard, &FindData);
    while(FindHandle != INVALID_HANDLE_VALUE)
    {
        ++Result.FileCount;
        
        if(!FindNextFileW(FindHandle, &FindData))
        {
            break;
        }
    }
    FindClose(FindHandle);

    Win32FileGroup->FindHandle = FindFirstFileW(WildCard, &Win32FileGroup->FindData);
    
    return(Result);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(Win32GetAllFilesOfTypeEnd)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    if(Win32FileGroup)
    {
        FindClose(Win32FileGroup->FindHandle);

        VirtualFree(Win32FileGroup, 0, MEM_RELEASE);
    }
}

internal PLATFORM_OPEN_FILE(Win32OpenNextFile)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    platform_file_handle Result = {};

    if(Win32FileGroup->FindHandle != INVALID_HANDLE_VALUE)
    {    
        // TODO(casey): If we want, someday, make an actual arena used by Win32
        win32_platform_file_handle *Win32Handle = (win32_platform_file_handle *)VirtualAlloc(
            0, sizeof(win32_platform_file_handle),
            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        Result.Platform = Win32Handle;

        if(Win32Handle)
        {
            wchar_t *FileName = Win32FileGroup->FindData.cFileName;
            Win32Handle->Win32Handle = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            Result.NoErrors = (Win32Handle->Win32Handle != INVALID_HANDLE_VALUE);
        }
        
        if(!FindNextFileW(Win32FileGroup->FindHandle, &Win32FileGroup->FindData))
        {
            FindClose(Win32FileGroup->FindHandle);
            Win32FileGroup->FindHandle = INVALID_HANDLE_VALUE;
        }
    }
    
    return(Result);
}

internal PLATFORM_FILE_ERROR(Win32FileError)
{
#if HANDMADE_INTERNAL
    OutputDebugString("WIN32 FILE ERROR: ");
    OutputDebugString(Message);
    OutputDebugString("\n");
#endif
    
    Handle->NoErrors = false;
}

internal PLATFORM_READ_DATA_FROM_FILE(Win32ReadDataFromFile)
{
    if(PlatformNoFileErrors(Source))
    {
        win32_platform_file_handle *Handle = (win32_platform_file_handle *)Source->Platform;
        OVERLAPPED Overlapped = {};
        Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
        Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
    
        uint32 FileSize32 = SafeTruncateUInt64(Size);
    
        DWORD BytesRead;
        if(ReadFile(Handle->Win32Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
           (FileSize32 == BytesRead))
        {
            // NOTE(casey): File read succeeded!
        }
        else
        {
            Win32FileError(Source, "Read file failed.");
        }
    }
}

/*

internal PLATFORM_FILE_ERROR(Win32CloseFile)
{
    CloseHandle(FileHandle);
}

*/

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    return(Result);
}

PLATFORM_DEALLOCATE_MEMORY(Win32DeallocateMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

#if HANDMADE_INTERNAL
global_variable debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
#endif

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{    
    win32_state Win32State = {};

    platform_work_queue HighPriorityQueue = {};
    Win32MakeQueue(&HighPriorityQueue, 6);
    
    platform_work_queue LowPriorityQueue = {};
    Win32MakeQueue(&LowPriorityQueue, 2);

#if 0
    Win32AddEntry(&Queue, DoWorkerWork, "String A0");
    Win32AddEntry(&Queue, DoWorkerWork, "String A1");
    Win32AddEntry(&Queue, DoWorkerWork, "String A2");
    Win32AddEntry(&Queue, DoWorkerWork, "String A3");
    Win32AddEntry(&Queue, DoWorkerWork, "String A4");
    Win32AddEntry(&Queue, DoWorkerWork, "String A5");
    Win32AddEntry(&Queue, DoWorkerWork, "String A6");
    Win32AddEntry(&Queue, DoWorkerWork, "String A7");
    Win32AddEntry(&Queue, DoWorkerWork, "String A8");
    Win32AddEntry(&Queue, DoWorkerWork, "String A9");
    
    Win32AddEntry(&Queue, DoWorkerWork, "String B0");
    Win32AddEntry(&Queue, DoWorkerWork, "String B1");
    Win32AddEntry(&Queue, DoWorkerWork, "String B2");
    Win32AddEntry(&Queue, DoWorkerWork, "String B3");
    Win32AddEntry(&Queue, DoWorkerWork, "String B4");
    Win32AddEntry(&Queue, DoWorkerWork, "String B5");
    Win32AddEntry(&Queue, DoWorkerWork, "String B6");
    Win32AddEntry(&Queue, DoWorkerWork, "String B7");
    Win32AddEntry(&Queue, DoWorkerWork, "String B8");
    Win32AddEntry(&Queue, DoWorkerWork, "String B9");

    Win32CompleteAllWork(&Queue);
#endif
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

    Win32GetEXEFileName(&Win32State);

    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmade.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "handmade_temp.dll",
                              sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);

    char GameCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.tmp",
                              sizeof(GameCodeLockFullPath), GameCodeLockFullPath);

    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    Win32LoadXInput();

#if HANDMADE_INTERNAL
    DEBUGGlobalShowCursor = true;
#endif
    WNDCLASSA WindowClass = {};
        
    /* NOTE(casey): 1080p display mode is 1920x1080 -> Half of that is 960x540
       1920 -> 2048 = 2048-1920 -> 128 pixels
       1080 -> 2048 = 2048-1080 -> pixels 968
       1024 + 128 = 1152
    */
//    Win32ResizeDIBSection(&GlobalBackbuffer, 960, 540);
    Win32ResizeDIBSection(&GlobalBackbuffer, 1920, 1080);
//    Win32ResizeDIBSection(&GlobalBackbuffer, 1279, 719);
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
//    WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                0, // WS_EX_TOPMOST|WS_EX_LAYERED,
                WindowClass.lpszClassName,
                "Handmade Hero",
                WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                Instance,
                0);
        if(Window)
        {
            win32_sound_output SoundOutput = {};

            // TODO(casey): How do we reliably query on this on Windows?
            int MonitorRefreshHz = 60;
            HDC RefreshDC = GetDC(Window);
            int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            ReleaseDC(Window, RefreshDC);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            real32 GameUpdateHz = (real32)(MonitorRefreshHz / 2.0f);
            real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;

            // TODO(casey): Make this like sixty seconds?
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            // TODO(casey): Actually compute this variance and see
            // what the lowest reasonable value is.
            SoundOutput.SafetyBytes = (int)(((real32)SoundOutput.SamplesPerSecond*(real32)SoundOutput.BytesPerSample / GameUpdateHz)/3.0f);
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;

#if 0
            // NOTE(casey): This tests the PlayCursor/WriteCursor update frequency
            // On the Handmade Hero machine, it was 480 samples.
            while(GlobalRunning)
            {
                DWORD PlayCursor;
                DWORD WriteCursor;
                GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor);

                char TextBuffer[256];
                _snprintf_s(TextBuffer, sizeof(TextBuffer),
                            "PC:%u WC:%u\n", PlayCursor, WriteCursor);
                OutputDebugStringA(TextBuffer);
            }
#endif
            
            // TODO(casey): Pool with bitmap VirtualAlloc
            // TODO(casey): Remove MaxPossibleOverrun?
            u32 MaxPossibleOverrun = 2*8*sizeof(u16);
            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize + MaxPossibleOverrun,
                                                   MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

            
#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(256);
            GameMemory.TransientStorageSize = Gigabytes(1);
            GameMemory.DebugStorageSize = Megabytes(64);
            GameMemory.HighPriorityQueue = &HighPriorityQueue;
            GameMemory.LowPriorityQueue = &LowPriorityQueue;
            GameMemory.PlatformAPI.AddEntry = Win32AddEntry;
            GameMemory.PlatformAPI.CompleteAllWork = Win32CompleteAllWork;
            
            GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = Win32GetAllFilesOfTypeBegin;
            GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = Win32GetAllFilesOfTypeEnd;
            GameMemory.PlatformAPI.OpenNextFile = Win32OpenNextFile;
            GameMemory.PlatformAPI.ReadDataFromFile = Win32ReadDataFromFile;
            GameMemory.PlatformAPI.FileError = Win32FileError;

            GameMemory.PlatformAPI.AllocateMemory = Win32AllocateMemory;
            GameMemory.PlatformAPI.DeallocateMemory = Win32DeallocateMemory;

#if HANDMADE_INTERNAL
            GameMemory.PlatformAPI.DEBUGFreeFileMemory = DEBUGPlatformFreeFileMemory;
            GameMemory.PlatformAPI.DEBUGReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.PlatformAPI.DEBUGWriteEntireFile = DEBUGPlatformWriteEntireFile;
            GameMemory.PlatformAPI.DEBUGExecuteSystemCommand = DEBUGExecuteSystemCommand;
            GameMemory.PlatformAPI.DEBUGGetProcessState = DEBUGGetProcessState;
#endif

            // TODO(casey): Handle various memory footprints (USING
            // SYSTEM METRICS)

            // TODO(casey): Use MEM_LARGE_PAGES and
            // call adjust token privileges when not on Windows XP?

            // TODO(casey): TransientStorage needs to be broken up
            // into game transient and cache transient, and only the
            // former need be saved for state playback.
            Win32State.TotalSize = (GameMemory.PermanentStorageSize +
                                    GameMemory.TransientStorageSize +
                                    GameMemory.DebugStorageSize);
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize,
                                                      MEM_RESERVE|MEM_COMMIT,
                                                      PAGE_READWRITE);
            GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
            GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage +
                                           GameMemory.PermanentStorageSize);
            GameMemory.DebugStorage = ((u8 *)GameMemory.TransientStorage +
                                       GameMemory.TransientStorageSize);

            for(int ReplayIndex = 1;
                ReplayIndex < ArrayCount(Win32State.ReplayBuffers);
                ++ReplayIndex)
            {
                win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];

                // TODO(casey): Recording system still seems to take too long
                // on record start - find out what Windows is doing and if
                // we can speed up / defer some of that processing.
                
                Win32GetInputFileLocation(&Win32State, false, ReplayIndex,
                                          sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);

                ReplayBuffer->FileHandle =
                    CreateFileA(ReplayBuffer->FileName,
                                GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);

                LARGE_INTEGER MaxSize;
                MaxSize.QuadPart = Win32State.TotalSize;
                ReplayBuffer->MemoryMap = CreateFileMapping(
                    ReplayBuffer->FileHandle, 0, PAGE_READWRITE,
                    MaxSize.HighPart, MaxSize.LowPart, 0);

                ReplayBuffer->MemoryBlock = MapViewOfFile(
                    ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
                if(ReplayBuffer->MemoryBlock)
                {
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }

            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
    
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();

                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[30] = {0};

                DWORD AudioLatencyBytes = 0;
                real32 AudioLatencySeconds = 0;
                bool32 SoundIsValid = false;

                win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                                         TempGameCodeDLLFullPath,
                                                         GameCodeLockFullPath);
                while(GlobalRunning)
                {
                    //
                    //
                    //
                    
                    BEGIN_BLOCK(ExecutableRefresh);
                    NewInput->dtForFrame = TargetSecondsPerFrame;

                    GameMemory.ExecutableReloaded = false;                    
                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                    if(CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0)
                    {
                        Win32CompleteAllWork(&HighPriorityQueue);
                        Win32CompleteAllWork(&LowPriorityQueue);

#if HANDMADE_INTERNAL
                        GlobalDebugTable = &GlobalDebugTable_;
#endif
                        Win32UnloadGameCode(&Game);
                        Game = Win32LoadGameCode(SourceGameCodeDLLFullPath,
                                                 TempGameCodeDLLFullPath,
                                                 GameCodeLockFullPath);
                        GameMemory.ExecutableReloaded = true;
                    }
                    END_BLOCK(ExecutableRefresh);

                    //
                    //
                    //

                    BEGIN_BLOCK(InputProcessing);
                    
                    // TODO(casey): Zeroing macro
                    // TODO(casey): We can't zero everything because the up/down state will
                    // be wrong!!!
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for(int ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }

                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController);

                    if(!GlobalPause)
                    {
                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->MouseX = (r32)MouseP.x;
                        NewInput->MouseY = (r32)((GlobalBackbuffer.Height - 1) - MouseP.y);
                        NewInput->MouseZ = 0; // TODO(casey): Support mousewheel?

                        NewInput->ShiftDown = (GetKeyState(VK_SHIFT) & (1 << 15));
                        NewInput->AltDown = (GetKeyState(VK_MENU) & (1 << 15));
                        NewInput->ControlDown = (GetKeyState(VK_CONTROL) & (1 << 15));

                        DWORD WinButtonID[PlatformMouseButton_Count] =
                        {
                            VK_LBUTTON,
                            VK_MBUTTON,
                            VK_RBUTTON,
                            VK_XBUTTON1,
                            VK_XBUTTON2,
                        };
                        for(u32 ButtonIndex = 0;
                            ButtonIndex < PlatformMouseButton_Count;
                            ++ButtonIndex)
                        {
                            NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
                            NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                            Win32ProcessKeyboardMessage(&NewInput->MouseButtons[ButtonIndex],
                                                        GetKeyState(WinButtonID[ButtonIndex]) & (1 << 15));
                        }
                        
                        // TODO(casey): Need to not poll disconnected controllers to avoid
                        // xinput frame rate hit on older libraries...
                        // TODO(casey): Should we poll this more frequently
                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
                        if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
                        {
                            MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
                        }
                
                        for (DWORD ControllerIndex = 0;
                             ControllerIndex < MaxControllerCount;
                             ++ControllerIndex)
                        {
                            DWORD OurControllerIndex = ControllerIndex + 1;
                            game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                            game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                    
                            XINPUT_STATE ControllerState;
                            if(XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                            {
                                NewController->IsConnected = true;
                                NewController->IsAnalog = OldController->IsAnalog;
                           
                                // NOTE(casey): This controller is plugged in
                                // TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
                                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                                // TODO(casey): This is a square deadzone, check XInput to
                                // verify that the deadzone is "round" and show how to do
                                // round deadzone processing.
                                NewController->StickAverageX = Win32ProcessXInputStickValue(
                                    Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                NewController->StickAverageY = Win32ProcessXInputStickValue(
                                    Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                if((NewController->StickAverageX != 0.0f) ||
                                   (NewController->StickAverageY != 0.0f))
                                {
                                    NewController->IsAnalog = true;
                                }

                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                                {
                                    NewController->StickAverageY = 1.0f;
                                    NewController->IsAnalog = false;
                                }
                            
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                                {
                                    NewController->StickAverageY = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                            
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                                {
                                    NewController->StickAverageX = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                            
                                if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                                {
                                    NewController->StickAverageX = 1.0f;
                                    NewController->IsAnalog = false;
                                }

                                real32 Threshold = 0.5f;
                                Win32ProcessXInputDigitalButton(
                                    (NewController->StickAverageX < -Threshold) ? 1 : 0,
                                    &OldController->MoveLeft, 1,
                                    &NewController->MoveLeft);
                                Win32ProcessXInputDigitalButton(
                                    (NewController->StickAverageX > Threshold) ? 1 : 0,
                                    &OldController->MoveRight, 1,
                                    &NewController->MoveRight);
                                Win32ProcessXInputDigitalButton(
                                    (NewController->StickAverageY < -Threshold) ? 1 : 0,
                                    &OldController->MoveDown, 1,
                                    &NewController->MoveDown);
                                Win32ProcessXInputDigitalButton(
                                    (NewController->StickAverageY > Threshold) ? 1 : 0,
                                    &OldController->MoveUp, 1,
                                    &NewController->MoveUp);

                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionDown, XINPUT_GAMEPAD_A,
                                                                &NewController->ActionDown);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionRight, XINPUT_GAMEPAD_B,
                                                                &NewController->ActionRight);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionLeft, XINPUT_GAMEPAD_X,
                                                                &NewController->ActionLeft);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->ActionUp, XINPUT_GAMEPAD_Y,
                                                                &NewController->ActionUp);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                                &NewController->LeftShoulder);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                                &NewController->RightShoulder);

                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->Start, XINPUT_GAMEPAD_START,
                                                                &NewController->Start);
                                Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                &OldController->Back, XINPUT_GAMEPAD_BACK,
                                                                &NewController->Back);
                            }
                            else
                            {
                                // NOTE(casey): The controller is not available
                                NewController->IsConnected = false;
                            }
                        }
                    }
                    END_BLOCK(InputProcessing);

                    //
                    //
                    //

                    BEGIN_BLOCK(GameUpdate);

                    game_offscreen_buffer Buffer = {};
                    Buffer.Memory = GlobalBackbuffer.Memory;
                    Buffer.Width = GlobalBackbuffer.Width; 
                    Buffer.Height = GlobalBackbuffer.Height;
                    Buffer.Pitch = GlobalBackbuffer.Pitch;
                    if(!GlobalPause)
                    {
                        if(Win32State.InputRecordingIndex)
                        {
                            Win32RecordInput(&Win32State, NewInput);
                        }

                        if(Win32State.InputPlayingIndex)
                        {
                            game_input Temp = *NewInput;
                            Win32PlayBackInput(&Win32State, NewInput);
                            for(u32 MouseButtonIndex = 0;
                                MouseButtonIndex < PlatformMouseButton_Count;
                                ++MouseButtonIndex)
                            {
                                NewInput->MouseButtons[MouseButtonIndex] = Temp.MouseButtons[MouseButtonIndex];
                            }
                            NewInput->MouseX = Temp.MouseX;
                            NewInput->MouseY = Temp.MouseY;
                            NewInput->MouseZ = Temp.MouseZ;
                        }
                        if(Game.UpdateAndRender)
                        {
                            Game.UpdateAndRender(&GameMemory, NewInput, &Buffer);
//                            HandleDebugCycleCounters(&GameMemory);
                        }
                    }

                    END_BLOCK(GameUpdate);

                    //
                    //
                    //

                    BEGIN_BLOCK(AudioUpdate);
                    
                    if(!GlobalPause)
                    {
                        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                        {
                            /* NOTE(casey):

                               Here is how sound output computation works.

                               We define a safety value that is the number
                               of samples we think our game update loop
                               may vary by (let's say up to 2ms)
                       
                               When we wake up to write audio, we will look
                               and see what the play cursor position is and we
                               will forecast ahead where we think the play
                               cursor will be on the next frame boundary.

                               We will then look to see if the write cursor is
                               before that by at least our safety value.  If
                               it is, the target fill position is that frame
                               boundary plus one frame.  This gives us perfect
                               audio sync in the case of a card that has low
                               enough latency.

                               If the write cursor is _after_ that safety
                               margin, then we assume we can never sync the
                               audio perfectly, so we will write one frame's
                               worth of audio plus the safety margin's worth
                               of guard samples.
                            */
                            if(!SoundIsValid)
                            {
                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }

                            DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
                                                SoundOutput.SecondaryBufferSize);

                            DWORD ExpectedSoundBytesPerFrame =
                                (int)((real32)(SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample) /
                                      GameUpdateHz);
                            real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
                            DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame)*(real32)ExpectedSoundBytesPerFrame);

                            DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
                        
                            DWORD SafeWriteCursor = WriteCursor;
                            if(SafeWriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor);
                            SafeWriteCursor += SoundOutput.SafetyBytes;
                        
                            bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);                        

                            DWORD TargetCursor = 0;
                            if(AudioCardIsLowLatency)
                            {
                                TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                            }
                            else
                            {
                                TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame +
                                                SoundOutput.SafetyBytes);
                            }
                            TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);
                        
                            DWORD BytesToWrite = 0;
                            if(ByteToLock > TargetCursor)
                            {
                                BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                                BytesToWrite += TargetCursor;
                            }
                            else
                            {
                                BytesToWrite = TargetCursor - ByteToLock;
                            }

                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = Align8(BytesToWrite / SoundOutput.BytesPerSample);
                            BytesToWrite = SoundBuffer.SampleCount*SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;
                            if(Game.GetSoundSamples)
                            {
                                Game.GetSoundSamples(&GameMemory, &SoundBuffer);
                            }

#if HANDMADE_INTERNAL
                            win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->OutputLocation = ByteToLock;
                            Marker->OutputByteCount = BytesToWrite;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if(UnwrappedWriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                            AudioLatencySeconds =
                                (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
                                 (real32)SoundOutput.SamplesPerSecond);

#if 0
                            char TextBuffer[256];
                            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                                        "BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n",
                                        ByteToLock, TargetCursor, BytesToWrite,
                                        PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
                            OutputDebugStringA(TextBuffer);
#endif
#endif   
                            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                        }
                        else
                        {
                            SoundIsValid = false;
                        }
                    }

                    END_BLOCK(AudioUpdate);

                    //
                    //
                    //
                    
#if HANDMADE_INTERNAL
                    BEGIN_BLOCK(DebugCollation);

                    if(Game.DEBUGFrameEnd)
                    {
                        GlobalDebugTable = Game.DEBUGFrameEnd(&GameMemory, NewInput, &Buffer);
                    }
                    GlobalDebugTable_.EventArrayIndex_EventIndex = 0;

                    END_BLOCK(DebugCollation);
#endif
                    
                    //
                    //
                    //

                    // TODO(casey): Leave this off until we have actual vblank support?
#if 0
                    BEGIN_BLOCK(FramerateWait);

                    if(!GlobalPause)
                    {
                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

                        // TODO(casey): NOT TESTED YET!  PROBABLY BUGGY!!!!!
                        real32 SecondsElapsedForFrame = WorkSecondsElapsed;
                        if(SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {                        
                            if(SleepIsGranular)
                            {
                                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                                   SecondsElapsedForFrame));
                                if(SleepMS > 0)
                                {
                                    Sleep(SleepMS);
                                }
                            }
                        
                            real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                                       Win32GetWallClock());
                            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                // TODO(casey): LOG MISSED SLEEP HERE
                            }
                        
                            while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                            {                            
                                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                                Win32GetWallClock());
                            }
                        }
                        else
                        {
                            // TODO(casey): MISSED FRAME RATE!
                            // TODO(casey): Logging
                        }
                    }

                    END_BLOCK(FramerateWait);
#endif
                    
                    //
                    //
                    //

                    BEGIN_BLOCK(FrameDisplay);
                    
                    win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                    HDC DeviceContext = GetDC(Window);
                    Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                               Dimension.Width, Dimension.Height);
                    ReleaseDC(Window, DeviceContext);

                    FlipWallClock = Win32GetWallClock();

                    game_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                    // TODO(casey): Should I clear these here?

                    END_BLOCK(FrameDisplay);

                    LARGE_INTEGER EndCounter = Win32GetWallClock();                    
                    FRAME_MARKER(Win32GetSecondsElapsed(LastCounter, EndCounter));
                    LastCounter = EndCounter;
                }
            }
            else
            {
                // TODO(casey): Logging
            }
        }
        else
        {
            // TODO(casey): Logging
        }
    }
    else
    {
        // TODO(casey): Logging
    }
    
    return(0);
}
