@echo off
setlocal EnableDelayedExpansion

echo.
echo   +==================================================+
echo   ^|    SecureVault  *  Zenith Web Server             ^|
echo   +==================================================+
echo.

:: ---- Locate zenith.exe (always next to this bat file's parent) ----
set ZENITH=%~dp0..\zenith.exe

echo   Compiler : %ZENITH%
echo   Project  : %~dp0
echo.

:: ---- Serve the project directory (Next.js-style directory router) ----
echo   Starting SSR server on http://localhost:8080 ...
echo   Routes:
echo     /          ^<-- pages/index.zen
echo     /editor    ^<-- pages/editor.zen
echo.
echo   Press Ctrl+C to stop.
echo.

:: %~dp0 expands to the ABSOLUTE path of the bat file's directory (no trailing dot needed)
"%ZENITH%" serve "%~dp0" --port 8080 --target web

endlocal
