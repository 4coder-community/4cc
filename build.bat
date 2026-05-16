@echo off

rem Make sure we are at the root of the project
cd /D "%~dp0"

set src_root=%cd%\code
set custom_root=%src_root%\custom
set build_root=%cd%\build

if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
    call %custom_root%\bin\setup_cl_x64.bat
) else (
    call %custom_root%\bin\setup_cl_x86.bat
)

set opts=/nologo /FC /Zi /I%src_root% /I%custom_root%

if not exist "%build_root%" mkdir %build_root%
pushd %build_root%
call cl %opts% %src_root%\4ed_build.cpp /Febuild
if %ERRORLEVEL% neq 0 exit /b
popd

pushd %src_root%
%build_root%\build.exe %*
popd
