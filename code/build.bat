@echo off


set LIB_DIR=W:\SDLProject\SDL\lib\x64
set INCLUDE_DIR=W:\SDLProject\SDL\include
set LINKER_OPTIONS=/LIBPATH:%LIB_DIR%


mkdir ..\build
pushd ..\build
cl -FC -Zi ..\code\main.cpp SDL2.lib /I %INCLUDE_DIR%\ -link %LINKER_OPTIONS%
popd