@echo off
echo ===================================================
echo   Zenith Android Native C++ Cross-Compiler Builder
echo ===================================================
echo.

:: 1. Detect NDK
set NDK_DIR=
for /d %%i in ("%USERPROFILE%\AppData\Local\Android\Sdk\ndk\*") do set NDK_DIR=%%i
if not defined NDK_DIR (
    echo [ERROR] Android NDK not found in %USERPROFILE%\AppData\Local\Android\Sdk\ndk
    exit /b 1
)
echo [OK] Found NDK: %NDK_DIR%

:: 2. Check for Connected ADB Devices
echo Checking connected devices...
adb devices
set ABI=
for /f "delims=" %%a in ('adb shell getprop ro.product.cpu.abi 2^>nul') do (
    for /f "delims=" %%b in ("%%a") do set "ABI=%%b"
)
if not defined ABI (
    echo [ERROR] No active Android device or emulator detected.
    echo Please make sure your emulator is running and visible in "adb devices".
    exit /b 1
)
echo [OK] Target Device ABI: %ABI%

:: 3. Select cross-compiler based on ABI
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

:: 4. Transpile Zenith source code to C++
echo.
echo Transpiling tests/main.zen to C++...
.\zenith.exe tests/main.zen
if %errorlevel% neq 0 (
    echo [ERROR] Zenith transpile failed.
    exit /b %errorlevel%
)

:: 5. Compile C++ to Android Native Binary
echo.
echo Cross-compiling transpiled C++ code for Android %ABI%...
call "%COMPILER%" -O3 -std=c++17 tests/main.cpp -I include -static-libstdc++ -o tests/zenith_app_android
if %errorlevel% neq 0 (
    echo [ERROR] Cross-compilation failed.
    exit /b %errorlevel%
)
echo [OK] Native binary built successfully: tests/zenith_app_android

:: 6. Push to Device
echo.
echo Pushing binary to Android device (/data/local/tmp/zenith_app)...
adb push tests/zenith_app_android /data/local/tmp/zenith_app
if %errorlevel% neq 0 (
    echo [ERROR] adb push failed.
    exit /b %errorlevel%
)

:: 7. Grant Execution Permissions
adb shell chmod +x /data/local/tmp/zenith_app
if %errorlevel% neq 0 (
    echo [ERROR] Failed to set execute permissions.
    exit /b %errorlevel%
)

:: 8. Execute Native Zenith App on Android
echo.
echo Running Zenith Native Application on Android Device...
echo ---------------------------------------------------
adb shell /data/local/tmp/zenith_app
echo ---------------------------------------------------
echo Execution completed.
