@echo off
pushd %~dp0

SET ROOT=%CD%

SET FILES=
SET FILES=%FILES% %ROOT%\src\base\ftsystem.c
SET FILES=%FILES% %ROOT%\src\base\ftinit.c
SET FILES=%FILES% %ROOT%\src\base\ftdebug.c
SET FILES=%FILES% %ROOT%\src\base\ftbase.c
SET FILES=%FILES% %ROOT%\src\base\ftbitmap.c
SET FILES=%FILES% %ROOT%\src\sfnt\sfnt.c
SET FILES=%FILES% %ROOT%\src\truetype\truetype.c
SET FILES=%FILES% %ROOT%\src\cff\cff.c
SET FILES=%FILES% %ROOT%\src\smooth\smooth.c
SET FILES=%FILES% %ROOT%\src\psaux\psaux.c
SET FILES=%FILES% %ROOT%\src\psnames\psnames.c
SET FILES=%FILES% %ROOT%\src\pshinter\pshinter.c

SET BUILD_DIR=%ROOT%\..\x64
IF "%Platform%" == "X86" (
    SET BUILD_DIR=%ROOT%\..\x86
) ELSE IF "%Platform%" == "x86" (
    SET BUILD_DIR=%ROOT%\..\x86
)
IF NOT EXIST "%BUILD_DIR%" MKDIR "%BUILD_DIR%"
cd %BUILD_DIR%

del *.pdb > NUL 2> NUL
cl /c /nologo /Zi /O2 /Fd:freetype.pdb /DFT2_BUILD_LIBRARY /I%ROOT% %FILES%
lib /nologo /OUT:freetype.lib *.obj
del *.obj > NUL 2> NUL
popd
