@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Web Runner                   ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

echo   [1/2]   Transpiling lib/main.zen -^> main.html ...
%COMPILER% ../lib/main.zen -target web -o main.html
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    HTML + JS output ready: web/main.html

echo   [2/3]   Starting Python HTTP server on port 8085 ...
start /b py -m http.server 8085 2>nul
if %errorlevel% neq 0 (
    echo   [WARN]  Could not start Python HTTP server automatically.
) else (
    echo   [OK]    Server running in background on port 8085
)

echo   [3/3]   Launching in Google Chrome ...
start chrome http://localhost:8085/main.html

echo.
echo   [OK] Server started and Chrome launched!
echo.
endlocal
