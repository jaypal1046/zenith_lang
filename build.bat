@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Building Zenith Compiler               ^|
echo   +==================================================+
echo.

:: Try TDM-GCC first, then scoop gcc, then plain g++
set GCC=
if exist "C:\TDM-GCC-64\bin\g++.exe"                    set GCC=C:\TDM-GCC-64\bin\g++.exe
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
    -lws2_32 -lpthread

if %errorlevel% neq 0 (
    echo [ERROR] Zenith compiler build failed.
    exit /b %errorlevel%
)
echo [OK] Zenith compiler built successfully: zenith.exe
echo.

echo Running Zenith Test Suite...
py tests/run_tests.py
if %errorlevel% neq 0 (
    echo [WARNING] Test suite returned errors - but build is complete.
)

echo.
echo [OK] Build complete!
echo.
echo To serve SecureVault on web:
echo   zenith.exe serve securevault --port 8080 --target web
echo.
endlocal
