@echo off


set LIB_DIR=C:\Users\JoshPC\projects\Random_Projects\SDLProject\SDL\lib\x64
set INCLUDE_DIR=C:\Users\JoshPC\projects\Random_Projects\SDLProject\SDL\include
set LINKER_OPTIONS=/LIBPATH:%LIB_DIR%


mkdir ..\build
pushd ..\build
cl -FC -Zi ..\code\main.cpp SDL2.lib /I %INCLUDE_DIR%\ -link %LINKER_OPTIONS%
popd

