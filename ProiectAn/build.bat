@echo off
rem Compiles everything in src\ and starts the program. Run it from MSYS2 / any shell where g++ is on the PATH.
cd /d "%~dp0src"
g++ main.cpp Object.cpp Spatial.cpp Collision.cpp UI.cpp Modes.cpp -o ..\main.exe -IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++17 -O2 -pthread
if errorlevel 1 (
    echo.
    echo Build FAILED
    pause
    exit /b 1
)
cd /d "%~dp0"
echo Build OK
main.exe