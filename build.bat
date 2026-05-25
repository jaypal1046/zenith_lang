@echo off
setlocal

echo.
echo   +==================================================+
2: echo   ^|          Building Zenith Compiler               ^|
3: echo   +==================================================+
4: echo.

g++ -O3 -std=c++17 src/main.cpp src/frontend/lexer.cpp src/frontend/parser.cpp src/frontend/semantic.cpp src/frontend/formatter.cpp src/lsp/lsp.cpp src/backend/codegen.cpp src/backend/js_codegen.cpp src/backend/wasm_codegen.cpp -I include -o zenith.exe -lws2_32 -lpthread
if %errorlevel% neq 0 (
    echo [ERROR] Zenith compiler build failed.
    exit /b %errorlevel%
)
echo [OK] Zenith compiler built successfully (zenith.exe).
echo.

echo Running Zenith Reorganized Test Suite...
py tests/run_tests.py
if %errorlevel% neq 0 (
    echo [ERROR] Test suite failed.
    exit /b %errorlevel%
)

echo [OK] All tasks completed successfully!
endlocal
