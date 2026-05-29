@echo off
setlocal EnableDelayedExpansion

for /f "delims=" %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"
set "CYAN=%ESC%[96m"
set "GREEN=%ESC%[92m"
set "YELLOW=%ESC%[93m"
set "RED=%ESC%[91m"
set "BOLD=%ESC%[1m"
set "DIM=%ESC%[2m"
set "RESET=%ESC%[0m"

echo.
echo %BOLD%%CYAN%  ^+--------------------------------------------------+%RESET%
echo %BOLD%%CYAN%  ^|          Zenith  .  Android Runner               ^|%RESET%
echo %BOLD%%CYAN%  ^+--------------------------------------------------+%RESET%
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo %DIM%  Using Zenith compiler: %COMPILER%%RESET%
echo.

echo %BOLD%  ^.  Checking Android NDK...%RESET%
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
    echo %RED%  x  Android NDK not found. Set ANDROID_NDK_HOME or ANDROID_HOME.%RESET%
    exit /b 1
)
echo %GREEN%  OK NDK  %DIM%%NDK_DIR%%RESET%

echo %BOLD%  ^.  Locating connected devices...%RESET%
set ABI=
for /f "delims=" %%a in ('adb shell getprop ro.product.cpu.abi 2^>nul') do (
    for /f "delims=" %%b in ("%%a") do set "ABI=%%b"
)
if not defined ABI (
    echo %RED%  x  No active Android device or emulator found.%RESET%
    exit /b 1
)
for /f "tokens=1" %%d in ('adb devices ^| findstr /v "List" ^| findstr "device"') do set DEVICE_ID=%%d
for /f "delims=" %%m in ('adb shell getprop ro.product.model 2^>nul') do set DEVICE_MODEL=%%m
echo %GREEN%  OK Device  %DIM%%DEVICE_MODEL% (%DEVICE_ID%) -- ABI: %ABI%%RESET%

echo %BOLD%  ^.  Selecting NDK toolchain...%RESET%
set COMPILER_NAME=
if "%ABI%"=="x86"         set COMPILER_NAME=i686-linux-android30-clang++.cmd
if "%ABI%"=="x86_64"      set COMPILER_NAME=x86_64-linux-android30-clang++.cmd
if "%ABI%"=="arm64-v8a"   set COMPILER_NAME=aarch64-linux-android30-clang++.cmd
if "%ABI%"=="armeabi-v7a" set COMPILER_NAME=armv7a-linux-androideabi30-clang++.cmd
if not defined COMPILER_NAME (
    echo %RED%  x  Unsupported ABI: %ABI%%RESET%
    exit /b 1
)
set TOOLCHAIN_BIN=%NDK_DIR%\toolchains\llvm\prebuilt\windows-x86_64\bin
set NDK_CLANG=%TOOLCHAIN_BIN%\%COMPILER_NAME%
if not exist "%NDK_CLANG%" (
    echo %RED%  x  Clang not found: %NDK_CLANG%%RESET%
    exit /b 1
)
echo %GREEN%  OK Toolchain  %DIM%%COMPILER_NAME%%RESET%

echo.
echo %BOLD%  1/4  Transpiling  %DIM%lib/main.zen -^> android/main.cpp%RESET%
%COMPILER% ../lib/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 ( echo %RED%  x  Transpile failed.%RESET% & exit /b %errorlevel% )
echo %GREEN%  OK C++ source ready%RESET%

echo %BOLD%  2/4  Compiling   %DIM%C++17 -^> Android %ABI% binary%RESET%
call "%NDK_CLANG%" -O3 -std=c++17 main.cpp -I ../include -static-libstdc++ -llog -o main_app_android 2^>^&1
if %errorlevel% neq 0 ( echo %RED%  x  Compilation failed.%RESET% & exit /b %errorlevel% )
echo %GREEN%  OK Build complete%RESET%

echo %BOLD%  3/4  Installing  %DIM%-^> /data/local/tmp/zenith_app%RESET%
adb push main_app_android /data/local/tmp/zenith_app >nul 2>^&1
adb shell chmod +x /data/local/tmp/zenith_app
if %errorlevel% neq 0 ( echo %RED%  x  Install failed.%RESET% & exit /b %errorlevel% )
echo %GREEN%  OK Installed on device%RESET%

echo %BOLD%  4/4  Running    %DIM%Zenith App on %DEVICE_MODEL%%RESET%
echo.
echo %YELLOW%  +----------------------------------------------------+%RESET%
echo %YELLOW%  ^|  Zenith App Output (stdout + Logcat)               ^|%RESET%
echo %YELLOW%  +----------------------------------------------------+%RESET%
echo.
adb logcat -c 2>nul
adb shell /data/local/tmp/zenith_app
echo.
echo %GREEN%  OK App exited successfully%RESET%
echo.
echo %DIM%  Tip: To see full Logcat stream run:%RESET%
echo %DIM%       adb logcat -s ZenithAndroid%RESET%
echo.
endlocal
