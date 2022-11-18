@echo off

set location=%cd%
set me="%~dp0"

REM 4cc\code\bin
cd %me%

REM 4cc
cd ..\..
set repo_root=%cd%
set src_root=%repo_root%\code
set build_root=%repo_root%\build
set bin_root=%repo_root%\code\bin
set custom_root=%repo_root%\code\custom
set custom_bin=%custom_root\bin

cd %location%

set mode=%1
if "%mode%" == "" (set mode="/DDEV_BUILD")

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /wd4189 /WX
set opts=%opts% /GR- /EHa- /nologo /FC /Zi
set opts=%opts% /I%src_root% /I%custom_root%
set opts=%opts% %mode%

set FirstError=0

if not exist "%build_root%" mkdir %build_root%
pushd %build_root%
call cl %opts% kernel32.lib %bin_root%\4ed_build.cpp /Febuild
if %ERRORLEVEL% neq 0 (set FirstError=1)
if %ERRORLEVEL% neq 0 (goto END)
popd

%build_root%\build
:END
if %ERRORLEVEL% neq 0 (set FirstError=1)
