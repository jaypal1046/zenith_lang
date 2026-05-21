@echo off
setlocal EnableDelayedExpansion

:: =====================================================
::   ZENITH RUN ANDROID - Flutter-style build runner
:: =====================================================

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Android Runner               ^|
echo   +==================================================+
echo.

:: ?? Locate Zenith compiler ????????????????????????????????
set COMPILER=
if exist "..\zenith.exe"     set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe"  set COMPILER=..\..\zenith.exe
if not defined COMPILER      set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

:: ?? Step 1: Detect Android SDK / NDK ?????????????????????
echo   [CHECK] Checking Android NDK...
set NDK_DIR=
if defined ANDROID_NDK_HOME (
    set NDK_DIR=%ANDROID_NDK_HOME%
) else if defined ANDROID_HOME (
    for /d %%i in ("%ANDROID_HOME%\ndk\*") do set NDK_DIR=%%i
)
if not defined NDK_DIR (
    for /d %%i in ("%USERPROFILE%\AppData\Local\Android\Sdk\ndk\*") do set NDK_DIR=%%i
)
if not defined NDK_DIR (
    echo   [ERROR] Android NDK not found. Set ANDROID_NDK_HOME or ANDROID_HOME.
    exit /b 1
)
echo   [OK]    NDK: %NDK_DIR%

:: ?? Step 2: Detect connected device / emulator ???????????
echo   [CHECK] Locating connected devices...
set ABI=
for /f "delims=" %%a in ('adb shell getprop ro.product.cpu.abi 2^>nul') do (
    for /f "delims=" %%b in ("%%a") do set "ABI=%%b"
)
if not defined ABI (
    echo   [ERROR] No active Android device or emulator found.
    echo          Start an emulator or plug in a device, then retry.
    exit /b 1
)
set DEVICE_ID=
for /f "tokens=1" %%d in ('adb devices ^| findstr /v "List" ^| findstr "device"') do (
    if not defined DEVICE_ID set DEVICE_ID=%%d
)
set DEVICE_MODEL=
for /f "delims=" %%m in ('adb shell getprop ro.product.model 2^>nul') do set DEVICE_MODEL=%%m
echo   [OK]    Device: %DEVICE_MODEL% ^(%DEVICE_ID%^) -- ABI: %ABI%

:: ?? Step 3: Select NDK cross-compiler ????????????????????
echo   [CHECK] Selecting NDK toolchain...
set COMPILER_NAME=
if "%ABI%"=="x86"         set COMPILER_NAME=i686-linux-android30-clang++.cmd
if "%ABI%"=="x86_64"      set COMPILER_NAME=x86_64-linux-android30-clang++.cmd
if "%ABI%"=="arm64-v8a"   set COMPILER_NAME=aarch64-linux-android30-clang++.cmd
if "%ABI%"=="armeabi-v7a" set COMPILER_NAME=armv7a-linux-androideabi30-clang++.cmd
if not defined COMPILER_NAME (
    echo   [ERROR] Unsupported ABI: %ABI%
    exit /b 1
)
set TOOLCHAIN_BIN=%NDK_DIR%\toolchains\llvm\prebuilt\windows-x86_64\bin
set NDK_CLANG=%TOOLCHAIN_BIN%\%COMPILER_NAME%
if not exist "%NDK_CLANG%" (
    echo   [ERROR] Clang not found: %NDK_CLANG%
    exit /b 1
)
echo   [OK]    Toolchain: %COMPILER_NAME%

:: ?? Step 4: Transpile Zenith -> C++ ??????????????????????
echo.
echo   [1/4]   Transpiling lib/main.zen -^> android/main.cpp ...
%COMPILER% ../lib/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    C++ source ready

:: ?? Step 5: Cross-compile C++ -> Android binary ??????????
echo   [2/4]   Compiling  C++17 -^> Android %ABI% binary ...
call "%NDK_CLANG%" -O3 -std=c++17 main.cpp -I ../include -static-libstdc++ -llog -o main_app_android 2>&1
if %errorlevel% neq 0 (
    echo   [ERROR] Compilation failed.
    exit /b %errorlevel%
)
echo   [OK]    Build complete

:: ?? Step 6: Push binary to device ????????????????????????
echo   [3/4]   Installing -^> /data/local/tmp/zenith_app ...
adb push main_app_android /data/local/tmp/zenith_app >nul 2>&1
adb shell chmod +x /data/local/tmp/zenith_app
if %errorlevel% neq 0 (
    echo   [ERROR] Install failed.
    exit /b %errorlevel%
)
echo   [OK]    Installed on %DEVICE_MODEL%

:: ?? Step 7: Clear old logcat + Run app ???????????????????
echo   [4/4]   Launching  Zenith App on %DEVICE_MODEL% ...
echo.
echo   +-------------------------------------------------+
echo   ^|  Zenith App Output                              ^|
echo   +-------------------------------------------------+
echo.

adb logcat -c 2>nul
adb shell /data/local/tmp/zenith_app

echo.
echo   +-------------------------------------------------+
echo   [OK] App exited successfully
echo.
echo   Tip: To stream full Logcat output run:
echo         adb logcat -s ZenithAndroid
echo.

endlocal

