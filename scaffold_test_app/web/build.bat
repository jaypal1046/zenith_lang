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

echo   [2/2]   Launching  in default browser ...
start main.html

echo.
echo   [OK] Browser launched
echo.
echo   Tip: Serve with a local HTTP server for full AJAX support:
echo         python -m http.server 8080
echo.

endlocal
