@echo off

SET NoCTime=0
WHERE ctime >nul 2>nul
IF %ERRORLEVEL% NEQ 0 (SET NoCTime=1)

IF %NoCTime% NEQ 1 (call "ctime" -begin shmup.ctm)

set FirstError=0

IF NOT "%clset%"=="64" call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
set clset = 64

set WARNINGOPS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set WARNINGOPS=%WARNINGOPS% /wd4213 /wd4057
set WARNINGOPS=%WARNINGOPS% /GR- /EHa- /nologo /FC
set WIN_LIBS=user32.lib winmm.lib gdi32.lib
set WIN_LIBS_DD=Ole32.lib Shell32.lib
set GL_LIBS=opengl32.lib
set USE_F=-I..\code\foreign
set BUILD_DLL=/LD /link /OPT:REF

IF NOT EXIST ..\build mkdir ..\build

copy ..\code\backup.bat ..\build\backup.bat > NUL

pushd ..\build

REM remove pdbs since we use randomly generated pdb names
del *.pdb > NUL 2> NUL

set EXPORTS=/EXPORT:target_get_functions /EXPORT:bank_get_functions
cl %WARNINGOPS% %USE_F% %WIN_LIBS% %GL_LIBS% ..\code\cd_tyler_comp_unit.cpp /FeCDRenderer.dll %* %BUILD_DLL% /INCREMENTAL:NO %EXPORTS% /PDB:cd_rend_%random%.pdb
IF %ERRORLEVEL% NEQ 0 (set FirstError=1)

set EXPORTS=/EXPORT:app_step
cl %WARNINGOPS% %USE_F% %GL_LIBS% ..\code\cd_app_comp_unit.cpp /FeCDGame.dll %* %BUILD_DLL% /INCREMENTAL:NO %EXPORTS% /PDB:cd_game_%random%.pdb
IF %ERRORLEVEL% NEQ 0 (set FirstError=1)

cl %WARNINGOPS% %USE_F% %WIN_LIBS% %WIN_LIBS_DD% %GL_LIBS% lualib.lib ..\code\cd_comp_unit.cpp ..\code\foreign\DragAndDrop.c /FeCipherDrive %* /link /INCREMENTAL:NO /PDB:cd_plat_%random%.pdb
IF %ERRORLEVEL% NEQ 0 (set FirstError=1)
popd

IF %NoCTime% NEQ 1 (call "ctime" -end shmup.ctm %FirstError%)

