@echo off

set prev_cwd=%cd%

rem Make sure we are at the root of the project
cd /D "%~dp0"

set src_root=%cd%\code
set custom_root=%src_root%\custom
set build_root=%cd%\build

rem If the user want's 32 bits they need to setup MSVC before calling the script.
call %custom_root%\bin\setup_cl_x64.bat

set opts=/nologo /FC /Zi /I%src_root% /I%custom_root%

if not exist "%build_root%" mkdir %build_root%
pushd %build_root%
call cl %opts% %src_root%\4ed_build.cpp /Febuild
popd

if %ERRORLEVEL% neq 0 goto END

%build_root%\build.exe %*

:END
cd /D "%prev_cwd%"