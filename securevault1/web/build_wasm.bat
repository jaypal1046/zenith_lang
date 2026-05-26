@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  WASM Runner                  ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

echo   [1/3]   Transpiling lib/main.zen -^> main_wasm.html ...
%COMPILER% ../lib/main.zen -target wasm -o main_wasm.html
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    WASM wrapper output ready: web/main_wasm.html

echo   [2/3]   Starting Python HTTP server on port 8000 ...
start /b python -m http.server 8000 2>nul
if %errorlevel% neq 0 (
    echo   [WARN]  Could not start Python HTTP server automatically.
) else (
    echo   [OK]    Server running in background on port 8000
)

echo   [3/3]   Launching  in default browser ...
start http://localhost:8000/main_wasm.html

echo.
echo   [OK] Browser launched to http://localhost:8000/main_wasm.html
echo.

endlocal
