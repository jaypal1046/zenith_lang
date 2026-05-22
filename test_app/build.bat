@echo off
echo ===================================================
echo   Zenith App Builder: Desktop and Web compiler
echo ===================================================
echo.

:: Check if compiler is available
where zenith >nul 2>nul
if %errorlevel% neq 0 (
    :: Fallback to local workspace relative path if available
    if exist "..\zenith.exe" (
        set COMPILER=..\zenith.exe
    ) else if exist "zenith.exe" (
        set COMPILER=zenith.exe
    ) else (
        echo [ERROR] 'zenith' compiler executable not found in PATH or parent directories.
        exit /b 1
    )
) else (
    set COMPILER=zenith
)

echo Using compiler: %COMPILER%

echo 1. Transpiling main.zen to C++ target...
%COMPILER% main.zen -target cpp
if %errorlevel% neq 0 exit /b %errorlevel%

echo 2. Transpiling main.zen to Web Target...
%COMPILER% main.zen -target web
if %errorlevel% neq 0 exit /b %errorlevel%

echo 3. Transpiling main.zen to WASM Target...
%COMPILER% main.zen -target wasm
if %errorlevel% neq 0 exit /b %errorlevel%

echo.
echo 4. Compiling C++ code into Desktop binary...
g++ -O3 -std=c++17 main.cpp -I include -o main_app.exe -lwinhttp
if %errorlevel% equ 0 (
    echo [OK] Native Windows application built: main_app.exe
) else (
    echo [WARNING] Native compiler g++ failed or winhttp not found.
)

echo.
echo ===================================================
echo   Zenith Build Succeeded!
echo ===================================================
echo To run Desktop: .\main_app.exe
echo To run Web: Launch server on directory and open main.html or main_wasm.html
echo.
