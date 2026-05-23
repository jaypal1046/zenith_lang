@echo off
echo Building Zenith Compiler...
g++ -O3 -std=c++17 src/main.cpp src/frontend/lexer.cpp src/frontend/parser.cpp src/frontend/semantic.cpp src/frontend/formatter.cpp src/lsp/lsp.cpp src/backend/codegen.cpp src/backend/js_codegen.cpp src/backend/wasm_codegen.cpp -I include -o zenith.exe
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)
echo Build succeeded!

echo Running Zenith Compiler on tests/main.zen (C++ Target)...
.\zenith.exe tests/main.zen
if %errorlevel% neq 0 (
    echo C++ Compilation failed.
    exit /b %errorlevel%
)

echo.
echo Running Zenith Compiler on tests/main.zen (Web Target)...
.\zenith.exe tests/main.zen -target web
if %errorlevel% neq 0 (
    echo Web Compilation failed.
    exit /b %errorlevel%
)

echo.
echo Running Zenith Compiler on tests/main.zen (WASM Target)...
.\zenith.exe tests/main.zen -target wasm
if %errorlevel% neq 0 (
    echo WASM Compilation failed.
    exit /b %errorlevel%
)

echo.
echo Running Zenith Compiler on tests/website.zen (WASM Target)...
.\zenith.exe tests/website.zen -target wasm
if %errorlevel% neq 0 (
    echo WASM Website Compilation failed.
    exit /b %errorlevel%
)

echo.
echo Running Zenith Compiler on tests/gallery.zen (C++, Web, WASM Targets)...
.\zenith.exe tests/gallery.zen -target cpp
if %errorlevel% neq 0 (
    echo Gallery C++ Transpilation failed.
    exit /b %errorlevel%
)
.\zenith.exe tests/gallery.zen -target web
if %errorlevel% neq 0 (
    echo Gallery Web Transpilation failed.
    exit /b %errorlevel%
)
.\zenith.exe tests/gallery.zen -target wasm
if %errorlevel% neq 0 (
    echo Gallery WASM Transpilation failed.
    exit /b %errorlevel%
)

echo.
echo Compiling Generated C++ Code...
g++ -O3 -std=c++17 tests/main.cpp -I include -o tests/zenith_app.exe -lwinhttp
if %errorlevel% neq 0 (
    echo App build failed.
    exit /b %errorlevel%
)

g++ -O3 -std=c++17 tests/gallery.cpp -I include -o tests/gallery_app.exe -lwinhttp
if %errorlevel% neq 0 (
    echo Gallery App build failed.
    exit /b %errorlevel%
)

echo.
echo Executing Compiled Zenith Application...
.\tests\zenith_app.exe

