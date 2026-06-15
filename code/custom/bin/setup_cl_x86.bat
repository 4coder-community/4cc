@echo off

where /q cl
IF %ERRORLEVEL% == 0 (EXIT /b)

SET SCRIPTS_PATH=%~dp0
"%SCRIPTS_PATH%\setup_cl_generic.bat" amd64_x86


