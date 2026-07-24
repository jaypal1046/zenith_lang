@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Game Runner (Windows)        ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

for /f "tokens=*" %%c in ('where g++ 2^>nul') do set GCC_PATH=%%c
if defined GCC_PATH (
    echo   [OK]    Toolchain: g++ - MinGW
) else (
    echo   [ERROR] g++ not found. Install MinGW and add it to PATH.
    exit /b 1
)

echo.
echo   [1/3]   Transpiling lib/main.zen -^> main_game.cpp ...
%COMPILER% ../lib/main.zen -target cpp -o main_game.cpp
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    C++ source ready

echo   [2/3]   Compiling C++17 -^> desktop/main_game.exe ...
g++ -O3 -std=c++17 main_game.cpp -I ../include -o main_game.exe -lwinhttp -lws2_32 -lopengl32 -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo   [ERROR] Native compilation failed.
    exit /b %errorlevel%
)
echo   [OK]    Build complete: desktop/main_game.exe

echo   [3/3]   Launching Zenith Game ...
echo.
.\main_game.exe

echo.
echo   [OK] Game exited successfully
echo.
endlocal
