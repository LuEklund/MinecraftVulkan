@echo off
if not exist build mkdir build

cd build

cmake -S ../ -B . -G "MinGW Makefiles"

if exist C:\msys64\ucrt64\bin\mingw32-make.exe (
    echo Using mingw32-make.exe
    call C:\msys64\ucrt64\bin\mingw32-make.exe
) else (
    echo mingw32-make.exe not found. Please add it to your PATH or specify its full path.
    exit /b 1
)
