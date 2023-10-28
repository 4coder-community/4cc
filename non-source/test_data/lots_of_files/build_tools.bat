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
set USE_F=-I..\code\foreign

IF NOT EXIST ..\meta mkdir ..\meta

pushd ..\meta
cl %WARNINGOPS% %USE_F% ..\code\tool_ttf_to_bmp.c /Fettf_to_bmp %*
IF %ERRORLEVEL% NEQ 0 (set FirstError=1)
popd

IF %NoCTime% NEQ 1 (call "ctime" -end shmup.ctm %FirstError%)
