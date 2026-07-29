@echo off
setlocal
taskkill /IM fantasy_survival.exe /F 2>nul

echo.
echo   +==================================================+
echo   ^|    Zenith Game SDK - Fantasy Survival Builder    ^|
echo   +==================================================+
echo.

set "COMPILER=%~dp0..\..\zenith.exe"
set "INC_DIR=%~dp0..\..\include"

if not exist "%COMPILER%" (
    echo Building Zenith Compiler...
    call ..\..\build.bat
)

echo 1. Transpiling main.zen to C++...
"%COMPILER%" main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo [ERROR] Transpilation failed.
    exit /b %errorlevel%
)

echo 2. Compiling native executable fantasy_survival.exe...
g++ -O3 -std=c++17 main.cpp -I "%INC_DIR%" -o fantasy_survival.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo [ERROR] C++ Compilation failed.
    exit /b %errorlevel%
)

echo.
echo   [OK] Build Successful! Launching fantasy_survival.exe...
start fantasy_survival.exe
endlocal
