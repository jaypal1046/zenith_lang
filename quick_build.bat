@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|       BUILDING ZENITH COMPILER (Quick Build)     ^|
echo   +==================================================+
echo.

set GCC=
if exist "C:\TDM-GCC-64\bin\g++.exe" set GCC=C:\TDM-GCC-64\bin\g++.exe
if not defined GCC if exist "C:\Users\jaypr\scoop\apps\gcc\current\bin\g++.exe" set GCC=C:\Users\jaypr\scoop\apps\gcc\current\bin\g++.exe
if not defined GCC set GCC=g++

echo   Using compiler: %GCC%
echo.

"%GCC%" -O3 -std=c++17 ^
    src/main.cpp ^
    src/frontend/lexer.cpp ^
    src/frontend/parser.cpp ^
    src/frontend/semantic.cpp ^
    src/frontend/formatter.cpp ^
    src/lsp/lsp.cpp ^
    src/backend/codegen.cpp ^
    src/backend/js_codegen.cpp ^
    src/backend/wasm_codegen.cpp ^
    -I include ^
    -o zenith.exe ^
    -lws2_32 -lwinhttp -lopengl32 -lpthread

if %errorlevel% neq 0 (
    echo.
    echo   [ERROR] Build failed!
    exit /b %errorlevel%
)

echo.
echo   [OK] zenith.exe built successfully in project root!

:: Copy to user PATH location if present
if exist "%USERPROFILE%\.zenith\bin" (
    copy /Y zenith.exe "%USERPROFILE%\.zenith\bin\zenith.exe" >nul
    echo   [OK] Copied new zenith.exe to %USERPROFILE%\.zenith\bin\zenith.exe
)

echo.
echo   TEST: Run with the newly built zenith binary:
echo.
echo     cd examples\fantasy_survival
echo     ..\..\zenith.exe run desktop
echo.

endlocal
