@echo off
echo "initializing PROJECT workspace"

rem set VSCMD_START_DIR=%CD%
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall" x64
set path=C:\Users\JoshPC\projects\Random_Projects\SDLProject\misc;%path%

cd C:\Users\JoshPC\projects\Random_Projects\SDLProject