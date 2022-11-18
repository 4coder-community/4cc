@echo off

if not exist backup (mkdir backup)

set DT=%DATE%_%TIME%
set DT=%DT:/=.%
set DT=%DT::=.%
set DEST=backup\%DT%

mkdir %DEST%
copy %1\* %DEST%\*


REM Thanks to stackoverflow.com ~ dbenham for
REM this batch script.

set MAXBACKUPS=%2

set "delMsg="
for /f "skip=%MAXBACKUPS% delims=" %%a in (
  'dir "backup\*" /t:c /a:d /o:-d /b'
) do (
  if not defined delMsg (
    set delMsg=1
    echo More than %MAXBACKUPS% found - only the %MAXBACKUPS% most recent folders will be preserved.
  )
  rd /s /q "backup\%%a"
)

