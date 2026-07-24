@echo off
echo.
echo   +==================================================+
echo   ^|    Zenith  *  Package Test Runner (Windows)       ^|
echo   +==================================================+
echo.
set COMPILER=zenith
if exist "zenith.exe" set COMPILER=.\zenith.exe
if exist "..\zenith.exe" set COMPILER=..\zenith.exe
%COMPILER% lib/main.zen -target cpp -o main.cpp
g++ -O3 -std=c++17 main.cpp -o main_app.exe -lws2_32
if %errorlevel% equ 0 (
    echo.
    echo   [OK] Build Succeeded! Running package tests:
    echo   --------------------------------------------------
    .\main_app.exe
    echo   --------------------------------------------------
)
