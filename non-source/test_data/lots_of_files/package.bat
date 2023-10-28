@echo off

pushd ..\

set DEST=packaged

if not exist %DEST% (mkdir %DEST%)
if not exist %DEST%\data (mkdir %DEST%\data)

copy build\CipherDrive.exe packaged\CipherDrive.exe
copy build\*.dll packaged\*.dll
copy build\backup.bat packaged\backup.bat
del packaged\*temp*

del /s /q packaged\data\*
copy code\data\* packaged\data\*

set DEST=C:\Users\Allen\ownCloud\CipherDriveDevBuild

if not exist %DEST% (mkdir %DEST%)
if not exist %DEST%\data (mkdir %DEST%\data)

copy packaged\* %DEST%\*
del /s /q %DEST%\data\*
copy packaged\data\* %DEST%\data\*

popd