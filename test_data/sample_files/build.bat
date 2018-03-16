@echo off

pushd ..\build
cl "..\test\With (Parens).cpp" /FC
popd

del ..\build\* /F /Q

REM do this to prevent the directory from becoming empty which leads to issues
copy "..\test\With (Parens).cpp" ..\build\*

