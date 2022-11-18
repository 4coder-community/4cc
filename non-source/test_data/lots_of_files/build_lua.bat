@echo off

IF NOT "%clset%"=="64" call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set clset = 64

set WARNINGOPS=/GR- /EHa- /nologo /FC

pushd ..\build
cl %WARNINGOPS% -c -Zi ..\code\foreign\allen_lua_build_target.c
lib /nologo allen_lua_build_target.obj -OUT:lualib.lib
popd