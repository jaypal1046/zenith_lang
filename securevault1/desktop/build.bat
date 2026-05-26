@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Desktop Runner (Windows)     ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

:: -- Detect host info --
for /f "tokens=*" %%c in ('where g++ 2^>nul') do set GCC_PATH=%%c
if defined GCC_PATH (
    echo   [OK]    Toolchain: g++ - MinGW
) else (
    echo   [ERROR] g++ not found. Install MinGW and add to PATH.
    exit /b 1
)

echo.
echo   [1/3]   Transpiling lib/main.zen -^> main.cpp ...
%COMPILER% ../lib/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    C++ source ready

echo   [2/3]   Compiling  C++17 -^> desktop/main_app.exe ...
g++ -O3 -std=c++17 main.cpp -I ../include -o main_app.exe -lwinhttp
if %errorlevel% neq 0 (
    echo   [ERROR] Native compilation failed.
    exit /b %errorlevel%
)
echo   [OK]    Build complete: desktop/main_app.exe

echo   [3/3]   Launching  Zenith Desktop App ...
echo.
echo   +-------------------------------------------------+
echo   ^|  Zenith App Output                              ^|
echo   +-------------------------------------------------+
echo.

.\main_app.exe

echo.
echo   [OK] App exited successfully
echo.

endlocal
