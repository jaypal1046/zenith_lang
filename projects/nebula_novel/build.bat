@echo off
setlocal
taskkill /IM nebula_novel.exe /F 2>nul

echo.
echo   +==================================================+
echo   ^|      Zenith Game SDK * Nebula Novel Builder      ^|
echo   +==================================================+
echo.

set "COMPILER=%~dp0..\..\zenith.exe"
set "INC_DIR=%~dp0..\..\include"
set "SRC_DIR=%~dp0..\..\src"

echo 1. Rebuilding Zenith Compiler with MediaQuery semantics...
g++ -O3 -std=c++17 "%SRC_DIR%\main.cpp" "%SRC_DIR%\frontend\lexer.cpp" "%SRC_DIR%\frontend\parser.cpp" "%SRC_DIR%\frontend\semantic.cpp" "%SRC_DIR%\frontend\formatter.cpp" "%SRC_DIR%\lsp\lsp.cpp" "%SRC_DIR%\backend\codegen.cpp" "%SRC_DIR%\backend\js_codegen.cpp" "%SRC_DIR%\backend\wasm_codegen.cpp" -I "%INC_DIR%" -o "%COMPILER%" -lws2_32 -lwinhttp
if %errorlevel% neq 0 (
    echo [ERROR] Zenith compiler build failed.
    exit /b %errorlevel%
)

echo 2. Transpiling scenes/main.zen to C++...
"%COMPILER%" scenes/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo [ERROR] Transpilation failed.
    exit /b %errorlevel%
)

echo 3. Compiling native executable...
g++ -O3 -std=c++17 main.cpp -I "%INC_DIR%" -o nebula_novel.exe -lws2_32 -lwinhttp -lopengl32 -lgdi32 -luser32
if %errorlevel% neq 0 (
    echo [ERROR] C++ Compilation failed.
    exit /b %errorlevel%
)

echo.
echo   [OK] Build Successful! Output: nebula_novel.exe
endlocal
