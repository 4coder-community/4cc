@echo off

call custom\bin\build_one_time.bat 4ed_api_parser_main.cpp

call one_time.exe 4ed_api_implementation.cpp

call bin\build.bat

