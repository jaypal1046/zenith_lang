@echo off
echo ===================================================
echo   Zenith Android Native C++ Builder
echo ===================================================
echo.

:: Detect NDK
set NDK_DIR=
for /d %%i in ("%USERPROFILE%\AppData\Local\Android\Sdk\ndk\*") do set NDK_DIR=%%i
if not defined NDK_DIR (
    echo [ERROR] Android NDK not found in %USERPROFILE%\AppData\Local\Android\Sdk\ndk
    exit /b 1
)
echo [OK] Found NDK: %NDK_DIR%

:: Check for Connected ADB Devices
echo Checking connected devices...
adb devices
set ABI=
for /f "delims=" %%a in ('adb shell getprop ro.product.cpu.abi 2^>nul') do (
    for /f "delims=" %%b in ("%%a") do set "ABI=%%b"
)
if not defined ABI (
    echo [ERROR] No active Android device or emulator detected via adb.
    exit /b 1
)
echo [OK] Target Device ABI: %ABI%

:: Select cross-compiler based on ABI
set COMPILER_NAME=
if "%ABI%"=="x86" set COMPILER_NAME=i686-linux-android30-clang++.cmd
if "%ABI%"=="x86_64" set COMPILER_NAME=x86_64-linux-android30-clang++.cmd
if "%ABI%"=="arm64-v8a" set COMPILER_NAME=aarch64-linux-android30-clang++.cmd
if "%ABI%"=="armeabi-v7a" set COMPILER_NAME=armv7a-linux-androideabi30-clang++.cmd

if not defined COMPILER_NAME (
    echo [ERROR] Unsupported target device ABI: %ABI%
    exit /b 1
)

set TOOLCHAIN_BIN=%NDK_DIR%\toolchains\llvm\prebuilt\windows-x86_64\bin
set COMPILER=%TOOLCHAIN_BIN%\%COMPILER_NAME%
if not exist "%COMPILER%" (
    echo [ERROR] Compiler not found: %COMPILER%
    exit /b 1
)
echo [OK] Selected Compiler: %COMPILER_NAME%

:: Transpile Zenith source code to C++
echo.
echo Transpiling main.zen to C++...
if exist "..\zenith.exe" (
    ..\zenith.exe main.zen
) else if exist "zenith.exe" (
    zenith.exe main.zen
) else (
    zenith main.zen
)
if %errorlevel% neq 0 (
    echo [ERROR] Zenith transpile failed.
    exit /b %errorlevel%
)

:: Compile C++ to Android Native Binary
echo.
echo Cross-compiling for Android %ABI%...
call "%COMPILER%" -O3 -std=c++17 main.cpp -I include -static-libstdc++ -llog -o main_app_android
if %errorlevel% neq 0 (
    echo [ERROR] Android Cross-compilation failed.
    exit /b %errorlevel%
)
echo [OK] Native binary built successfully: main_app_android

:: Push to Device
echo.
echo Pushing binary to Android device (/data/local/tmp/main_app)...
adb push main_app_android /data/local/tmp/main_app
if %errorlevel% neq 0 (
    echo [ERROR] adb push failed.
    exit /b %errorlevel%
)

:: Grant Execution Permissions
adb shell chmod +x /data/local/tmp/main_app
if %errorlevel% neq 0 (
    echo [ERROR] Failed to set execute permissions.
    exit /b %errorlevel%
)

:: Execute Native Zenith App on Android
echo.
echo Running Zenith Native Application on Android Device...
echo ---------------------------------------------------
adb shell /data/local/tmp/main_app
echo ---------------------------------------------------
echo Execution completed.
