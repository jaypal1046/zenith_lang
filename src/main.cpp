#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <memory>
#include <array>
#include "../include/frontend/lexer.h"
#include "../include/frontend/parser.h"
#include "../include/frontend/semantic.h"
#include "../include/backend/codegen.h"
#include "../include/backend/js_codegen.h"
#include "../include/backend/wasm_codegen.h"

std::string getDirectory(const std::string& filepath) {
    size_t last_slash = filepath.find_last_of("/\\");
    if (last_slash == std::string::npos) {
        return "";
    }
    return filepath.substr(0, last_slash + 1);
}

void resolveImports(ProgramNode* program, std::set<std::string>& loaded_files, const std::string& current_dir) {
    std::vector<std::unique_ptr<ASTNode>> new_statements;
    
    for (auto& stmt : program->statements) {
        if (auto* imp = dynamic_cast<ImportNode*>(stmt.get())) {
            std::string mod = imp->module_name;
            if (mod.length() > 4 && mod.substr(mod.length() - 4) == ".zen") {
                std::string full_path = current_dir + mod;
                if (loaded_files.find(full_path) == loaded_files.end()) {
                    loaded_files.insert(full_path);
                    
                    std::ifstream file(full_path);
                    if (!file.is_open()) {
                        std::cerr << "Error: Could not open imported file: " << full_path << "\n";
                        exit(1);
                    }
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    std::string code = buffer.str();
                    file.close();
                    
                    Lexer sub_lexer(code);
                    auto sub_tokens = sub_lexer.tokenize();
                    Parser sub_parser(sub_tokens);
                    auto sub_ast = sub_parser.parseProgram();
                    
                    resolveImports(sub_ast.get(), loaded_files, getDirectory(full_path));
                    
                    for (auto& sub_stmt : sub_ast->statements) {
                        new_statements.push_back(std::move(sub_stmt));
                    }
                }
                continue;
            }
        }
        new_statements.push_back(std::move(stmt));
    }
    
    program->statements = std::move(new_statements);
}

int runCreateProject(const std::string& project_name, const std::string& argv0) {
    namespace fs = std::filesystem;
    fs::path project_path = (project_name == ".") ? fs::current_path() : fs::absolute(project_name);
    
    std::cout << "Creating new Zenith project in: " << project_path.string() << "\n";
    
    try {
        fs::create_directories(project_path);
        fs::create_directories(project_path / "include");
        fs::create_directories(project_path / "lib");
        fs::create_directories(project_path / "lib" / "common");
        fs::create_directories(project_path / "lib" / "android");
        fs::create_directories(project_path / "lib" / "ios");
        fs::create_directories(project_path / "lib" / "web");
        fs::create_directories(project_path / "lib" / "desktop");
        fs::create_directories(project_path / "lib" / "linux");
        fs::create_directories(project_path / "lib" / "windows");
        fs::create_directories(project_path / "lib" / "mac");
        fs::create_directories(project_path / "android");
        fs::create_directories(project_path / "ios");
        fs::create_directories(project_path / "web");
        fs::create_directories(project_path / "desktop");
        fs::create_directories(project_path / "linux");
        fs::create_directories(project_path / "windows");
        fs::create_directories(project_path / "mac");
    } catch (const std::exception& e) {
        std::cerr << "Error: Could not create directories: " << e.what() << "\n";
        return 1;
    }
    
    // 1. Copy zenith_runtime.h and platform components
    std::vector<fs::path> possible_paths = {
        fs::absolute(argv0).parent_path() / "include" / "zenith_runtime.h",
        fs::absolute(argv0).parent_path() / "zenith_runtime.h",
        fs::current_path() / "include" / "zenith_runtime.h",
        fs::current_path() / "zenith_runtime.h",
        fs::current_path() / ".." / "include" / "zenith_runtime.h"
    };
    
    fs::path src_header;
    for (const auto& p : possible_paths) {
        if (fs::exists(p)) {
            src_header = p;
            break;
        }
    }
    
    if (src_header.empty()) {
        std::cerr << "[WARNING] Could not locate 'zenith_runtime.h'. Custom compilation might fail if not manually added to 'include/'.\n";
    } else {
        std::cout << "   [OK] Found runtime library: " << src_header.string() << "\n";
        try {
            fs::path target_header = project_path / "include" / "zenith_runtime.h";
            if (fs::exists(target_header)) {
                fs::remove(target_header);
            }
            fs::copy_file(src_header, target_header, fs::copy_options::overwrite_existing);
            std::cout << "   [OK] Copied 'zenith_runtime.h' to include/\n";
            
            fs::path src_dir = src_header.parent_path() / "zenith";
            fs::path target_dir = project_path / "include" / "zenith";
            if (fs::exists(target_dir)) {
                fs::remove_all(target_dir);
            }
            if (fs::exists(src_dir)) {
                fs::copy(src_dir, target_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                std::cout << "   [OK] Copied runtime platform subdirectories to include/zenith/\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "[WARNING] Could not copy runtime library/components: " << e.what() << "\n";
        }
    }
    
    // 2. Write lib platform-specific templates
    std::ofstream common_file(project_path / "lib" / "common" / "app_common.zen");
    if (common_file.is_open()) {
        common_file << R"raw(class AppCommon() {
    String getCommonMessage() {
        return "Shared Code Block";
    }
}
)raw";
        common_file.close();
        std::cout << "   [OK] Created 'lib/common/app_common.zen'\n";
    }

    std::ofstream android_file(project_path / "lib" / "android" / "app_android.zen");
    if (android_file.is_open()) {
        android_file << R"raw(class AppAndroid() {
    String getPlatformName() {
        return "Android Client";
    }
}
)raw";
        android_file.close();
        std::cout << "   [OK] Created 'lib/android/app_android.zen'\n";
    }

    std::ofstream ios_file(project_path / "lib" / "ios" / "app_ios.zen");
    if (ios_file.is_open()) {
        ios_file << R"raw(class AppIos() {
    String getPlatformName() {
        return "iOS Client";
    }
}
)raw";
        ios_file.close();
        std::cout << "   [OK] Created 'lib/ios/app_ios.zen'\n";
    }

    std::ofstream web_file(project_path / "lib" / "web" / "app_web.zen");
    if (web_file.is_open()) {
        web_file << R"raw(class AppWeb() {
    String getPlatformName() {
        return "Web Application";
    }
}
)raw";
        web_file.close();
        std::cout << "   [OK] Created 'lib/web/app_web.zen'\n";
    }

    std::ofstream desktop_file(project_path / "lib" / "desktop" / "app_desktop.zen");
    if (desktop_file.is_open()) {
        desktop_file << R"raw(class AppDesktop() {
    String getPlatformName() {
        return "Desktop Native Application";
    }
}
)raw";
        desktop_file.close();
        std::cout << "   [OK] Created 'lib/desktop/app_desktop.zen'\n";
    }

    std::ofstream linux_file(project_path / "lib" / "linux" / "app_linux.zen");
    if (linux_file.is_open()) {
        linux_file << R"raw(class AppLinux() {
    String getPlatformName() {
        return "Linux Native Application";
    }
}
)raw";
        linux_file.close();
        std::cout << "   [OK] Created 'lib/linux/app_linux.zen'\n";
    }

    std::ofstream windows_file(project_path / "lib" / "windows" / "app_windows.zen");
    if (windows_file.is_open()) {
        windows_file << R"raw(class AppWindows() {
    String getPlatformName() {
        return "Windows Native Application";
    }
}
)raw";
        windows_file.close();
        std::cout << "   [OK] Created 'lib/windows/app_windows.zen'\n";
    }

    std::ofstream mac_file(project_path / "lib" / "mac" / "app_mac.zen");
    if (mac_file.is_open()) {
        mac_file << R"raw(class AppMac() {
    String getPlatformName() {
        return "macOS Native Application";
    }
}
)raw";
        mac_file.close();
        std::cout << "   [OK] Created 'lib/mac/app_mac.zen'\n";
    }

    // 2.2 Write lib/main.zen
    std::string main_zen_content = R"raw(// Zenith Multiplatform Reorganized Application
import std.io;
import "common/app_common.zen";
import "desktop/app_desktop.zen";
import "android/app_android.zen";
import "ios/app_ios.zen";
import "web/app_web.zen";
import "linux/app_linux.zen";
import "windows/app_windows.zen";
import "mac/app_mac.zen";

class MyApp() {
    String title = "Zenith Cross-Platform App";
    Int counter = 0;
    String api_result = "No data fetched.";
    AppCommon common = AppCommon();
    AppDesktop desktop = AppDesktop();
    AppAndroid android = AppAndroid();
    AppIos ios = AppIos();
    AppWeb web = AppWeb();
    AppLinux linux = AppLinux();
    AppWindows windows = AppWindows();
    AppMac mac = AppMac();

    UI build() {
        return Column(
            // Header
            Container(
                Text(title + " (" + common.getCommonMessage() + ")", fontWeight: "bold", color: "cyan"),
                padding: 1
            ),

            // Platform details
            Card(
                Text("Platform Modules Loaded:", fontWeight: "bold", color: "blue"),
                Text(" - Common: " + common.getCommonMessage()),
                Text(" - Desktop module target: " + desktop.getPlatformName()),
                Text(" - Android module target: " + android.getPlatformName()),
                Text(" - iOS module target: " + ios.getPlatformName()),
                Text(" - Web module target: " + web.getPlatformName()),
                Text(" - Linux module target: " + linux.getPlatformName()),
                Text(" - Windows module target: " + windows.getPlatformName()),
                Text(" - macOS module target: " + mac.getPlatformName()),
                padding: 1
            ),

            // Counter section (Reactive)
            Card(
                Text("Click Counter Sample", fontWeight: "bold", color: "yellow"),
                Row(
                    Text("Value: " + counter, color: "green"),
                    Button("Increment", onClick: handleIncrement)
                ),
                padding: 1
            ),

            // REST API Integration
            Card(
                Text("REST Network API Verification", fontWeight: "bold", color: "magenta"),
                Row(
                    Text("API Payload: " + api_result),
                    Button("Fetch Data", onClick: handleFetch)
                ),
                padding: 1
            )
        );
    }

    Void handleIncrement() {
        setState {
            counter = counter + 1;
        }
    }

    Void handleFetch() {
        String res = httpGet("https://jsonplaceholder.typicode.com/todos/1");
        setState {
            api_result = res;
        }
    }
}

Void main() {
    println("Initializing Reorganized Zenith Application...");
    MyApp app = MyApp();
    app.build().render();
    println("Zenith Application Shutdown.");
}
)raw";

    std::ofstream main_file(project_path / "lib" / "main.zen");
    if (main_file.is_open()) {
        main_file << main_zen_content;
        main_file.close();
        std::cout << "   [OK] Created 'lib/main.zen'\n";
    } else {
        std::cerr << "Error: Could not write lib/main.zen\n";
        return 1;
    }

    // 3. Write desktop/build.bat
    std::string desktop_build_bat = R"raw(@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Desktop Runner (Windows)     ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

:: -- Detect host info --
for /f "tokens=*" %%c in ('where g++ 2^>nul') do set GCC_PATH=%%c
if defined GCC_PATH (
    echo   [OK]    Toolchain: g++ - MinGW
) else (
    echo   [ERROR] g++ not found. Install MinGW and add to PATH.
    exit /b 1
)

echo.
echo   [1/3]   Transpiling lib/main.zen -^> main.cpp ...
%COMPILER% ../lib/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    C++ source ready

echo   [2/3]   Compiling  C++17 -^> desktop/main_app.exe ...
g++ -O3 -std=c++17 main.cpp -I ../include -o main_app.exe -lwinhttp
if %errorlevel% neq 0 (
    echo   [ERROR] Native compilation failed.
    exit /b %errorlevel%
)
echo   [OK]    Build complete: desktop/main_app.exe

echo   [3/3]   Launching  Zenith Desktop App ...
echo.
echo   +-------------------------------------------------+
echo   ^|  Zenith App Output                              ^|
echo   +-------------------------------------------------+
echo.

.\main_app.exe

echo.
echo   [OK] App exited successfully
echo.

endlocal
)raw";

    std::ofstream d_bat(project_path / "desktop" / "build.bat");
    if (d_bat.is_open()) {
        d_bat << desktop_build_bat;
        d_bat.close();
        std::cout << "   [OK] Created 'desktop/build.bat'\n";
    }

    // 4. Write desktop/build.sh
    std::string desktop_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN DESKTOP (Linux/Mac) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Desktop Runner              |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

# Detect OS and pick flags
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM_FLAGS="-lpthread -framework CoreFoundation"
    echo -e "  ${GREEN}[OK]${RESET}    Platform: macOS (Desktop)"
else
    PLATFORM_FLAGS="-lpthread"
    echo -e "  ${GREEN}[OK]${RESET}    Platform: Linux (Desktop)"
fi

if ! command -v g++ &>/dev/null; then
    echo -e "  ${RED}[ERROR]${RESET} g++ not found. Install build-essential (Linux) or Xcode (Mac)."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    Toolchain: g++"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling  C++17 -> desktop/main_app ..."
g++ -O3 -std=c++17 main.cpp -I ../include -o main_app $PLATFORM_FLAGS
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Compilation failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: desktop/main_app"

echo -e "  ${BOLD}[3/3]${RESET}   Launching  Zenith Desktop App ..."
echo ""
echo "  +-------------------------------------------------+"
echo "  |  Zenith App Output                              |"
echo "  +-------------------------------------------------+"
echo ""

./main_app

echo ""
echo -e "  ${GREEN}[OK]${RESET} App exited successfully"
echo ""
)raw";

    std::ofstream d_sh(project_path / "desktop" / "build.sh");
    if (d_sh.is_open()) {
        d_sh << desktop_build_sh;
        d_sh.close();
        std::cout << "   [OK] Created 'desktop/build.sh'\n";
    }

    // 5. Write android/build.bat
    std::string android_build_bat = R"raw(@echo off
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
)raw";

    std::ofstream a_bat(project_path / "android" / "build.bat");
    if (a_bat.is_open()) {
        a_bat << android_build_bat;
        a_bat.close();
        std::cout << "   [OK] Created 'android/build.bat'\n";
    }

    // 5.5 Write android/build.sh
    std::string android_build_sh = R"raw(#!/bin/bash
echo "==================================================="
echo "  Zenith Android C++ Builder (Mac/Linux)"
echo "==================================================="
echo

if [ -f "../zenith" ]; then
    COMPILER="../zenith"
elif [ -f "../../zenith" ]; then
    COMPILER="../../zenith"
else
    COMPILER="zenith"
fi

# Detect NDK
if [ -n "$ANDROID_NDK_HOME" ]; then
    NDK_DIR="$ANDROID_NDK_HOME"
elif [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/ndk" ]; then
    NDK_DIR=$(ls -d $ANDROID_HOME/ndk/* 2>/dev/null | sort -V | tail -n 1)
else
    if [[ "$OSTYPE" == "darwin"* ]]; then
        NDK_BASE="$HOME/Library/Android/sdk/ndk"
    else
        NDK_BASE="$HOME/Android/Sdk/ndk"
    fi
    NDK_DIR=$(ls -d $NDK_BASE/* 2>/dev/null | sort -V | tail -n 1)
fi

if [ -z "$NDK_DIR" ]; then
    echo "[ERROR] Android NDK not found. Please set ANDROID_NDK_HOME or ANDROID_HOME."
    exit 1
fi
echo "[OK] Found NDK: $NDK_DIR"

if [[ "$OSTYPE" == "darwin"* ]]; then
    PREBUILT_HOST="darwin-x86_64"
else
    PREBUILT_HOST="linux-x86_64"
fi

echo "Checking connected devices..."
adb devices
ABI=$(adb shell getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r\n')
if [ -z "$ABI" ]; then
    echo "[ERROR] No active Android device or emulator detected via adb."
    exit 1
fi
echo "[OK] Target Device ABI: $ABI"

case "$ABI" in
    "x86") COMPILER_NAME="i686-linux-android30-clang++" ;;
    "x86_64") COMPILER_NAME="x86_64-linux-android30-clang++" ;;
    "arm64-v8a") COMPILER_NAME="aarch64-linux-android30-clang++" ;;
    "armeabi-v7a") COMPILER_NAME="armv7a-linux-androideabi30-clang++" ;;
    *)
        echo "[ERROR] Unsupported target device ABI: $ABI"
        exit 1
        ;;
esac

NDK_CLANG="$NDK_DIR/toolchains/llvm/prebuilt/$PREBUILT_HOST/bin/$COMPILER_NAME"
if [ ! -f "$NDK_CLANG" ]; then
    echo "[ERROR] Compiler not found: $NDK_CLANG"
    exit 1
fi

echo "Transpiling main.zen to C++..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then exit 1; fi

echo "Cross-compiling for Android $ABI..."
$NDK_CLANG -O3 -std=c++17 main.cpp -I ../include -static-libstdc++ -llog -o main_app_android
if [ $? -ne 0 ]; then
    echo "[ERROR] Android compilation failed."
    exit 1
fi

echo "Pushing binary to Android device..."
adb push main_app_android /data/local/tmp/main_app
adb shell chmod +x /data/local/tmp/main_app
echo "Running on device..."
adb shell /data/local/tmp/main_app
)raw";

    std::ofstream a_sh(project_path / "android" / "build.sh");
    if (a_sh.is_open()) {
        a_sh << android_build_sh;
        a_sh.close();
        std::cout << "   [OK] Created 'android/build.sh'\n";
    }

    // 6. Write android/AndroidManifest.xml
    std::string android_manifest = R"raw(<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="com.zenith.template"
    android:versionCode="1"
    android:versionName="1.0">

    <!-- Network permissions required for Agentic LLM calls -->
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.ACCESS_NETWORK_STATE" />

    <application
        android:label="Zenith App"
        android:allowBackup="false"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
        android:hasCode="false">

        <!-- NativeActivity drives the Zenith NDK binary -->
        <activity
            android:name="android.app.NativeActivity"
            android:label="Zenith App"
            android:configChanges="orientation|keyboardHidden|screenSize"
            android:exported="true">
            <meta-data android:name="android.app.lib_name" android:value="zenith_main" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>

    </application>
</manifest>
)raw";

    std::ofstream a_manifest(project_path / "android" / "AndroidManifest.xml");
    if (a_manifest.is_open()) {
        a_manifest << android_manifest;
        a_manifest.close();
        std::cout << "   [OK] Created 'android/AndroidManifest.xml'\n";
    }

    // 6.5 Write android/CMakeLists.txt  (NDK CMake build for Android Studio)
    std::string android_cmake = R"raw(cmake_minimum_required(VERSION 3.22.1)
project(zenith_main)

# Transpiled Zenith C++17 source (generated by: zenith lib/main.zen -target cpp -o android/main.cpp)
set(ZENITH_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_library(zenith_main SHARED ${ZENITH_SRC})

target_include_directories(zenith_main PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_compile_options(zenith_main PRIVATE -O3 -std=c++17)

# Link Android system libraries
target_link_libraries(zenith_main
    android
    log
    m
)
)raw";

    std::ofstream a_cmake(project_path / "android" / "CMakeLists.txt");
    if (a_cmake.is_open()) {
        a_cmake << android_cmake;
        a_cmake.close();
        std::cout << "   [OK] Created 'android/CMakeLists.txt'\n";
    }

    // 7. Write android/MainActivity.cpp
    std::string android_activity = R"raw(// Zenith Native Android Entry Driver (NDK)
// This file bridges the NativeActivity lifecycle to the Zenith app.
#include <android/native_activity.h>
#include "../include/zenith_runtime.h"

// Called by NativeActivity when the app starts.
// In a pure NDK binary (not via NativeActivity) this is unused;
// instead main() in main.cpp is the entry point.
extern "C" void ANativeActivity_onCreate(
    ANativeActivity* /*activity*/,
    void*            /*savedState*/,
    size_t           /*savedStateSize*/
) {
    // Lifecycle hook placeholder (native-app-glue extends this).
}
)raw";

    std::ofstream a_activity(project_path / "android" / "MainActivity.cpp");
    if (a_activity.is_open()) {
        a_activity << android_activity;
        a_activity.close();
        std::cout << "   [OK] Created 'android/MainActivity.cpp'\n";
    }

    // 8. Write ios/build.sh
    std::string ios_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN iOS - Flutter-style build runner
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  iOS Runner (Device/Sim)     |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

# -- CHECK: Must be macOS + Xcode --
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo -e "  ${RED}[ERROR]${RESET} iOS builds require macOS with Xcode."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    Platform: macOS"

SDK_PATH=$(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
if [ -z "$SDK_PATH" ]; then
    echo -e "  ${RED}[ERROR]${RESET} iOS SDK not found. Install Xcode from the Mac App Store."
    exit 1
fi
IOS_VER=$(xcrun --sdk iphoneos --show-sdk-version 2>/dev/null || echo "?")
echo -e "  ${GREEN}[OK]${RESET}    iOS SDK $IOS_VER: $SDK_PATH"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling  C++17 -> iOS arm64 binary ..."
xcrun -sdk iphoneos clang++ -arch arm64 -O3 -std=c++17 main.cpp \
    -I ../include -isysroot "$SDK_PATH" -o main_app_ios
if [ $? -ne 0 ]; then
    echo -e "  ${RED}[ERROR]${RESET} iOS compilation failed."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: ios/main_app_ios"

echo -e "  ${BOLD}[3/3]${RESET}   Deploying ..."
echo ""
echo -e "  ${YELLOW}[INFO]${RESET} To deploy to a physical device, codesign first:"
echo -e "  ${DIM}        codesign -s \"Apple Development\" main_app_ios${RESET}"
echo ""
echo -e "  ${YELLOW}[INFO]${RESET} To run in iOS Simulator, recompile with -sdk iphonesimulator"
echo ""
echo -e "  ${GREEN}[OK]${RESET} iOS binary ready at ios/main_app_ios"
echo ""
)raw";

    std::ofstream i_sh(project_path / "ios" / "build.sh");
    if (i_sh.is_open()) {
        i_sh << ios_build_sh;
        i_sh.close();
        std::cout << "   [OK] Created 'ios/build.sh'\n";
    }

    // 9. Write ios/Info.plist
    std::string ios_plist = R"raw(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleIdentifier</key>
    <string>com.zenith.template</string>
    <key>CFBundleName</key>
    <string>Zenith App</string>
    <key>CFBundleVersion</key>
    <string>1.0</string>
</dict>
</plist>
)raw";

    std::ofstream i_plist(project_path / "ios" / "Info.plist");
    if (i_plist.is_open()) {
        i_plist << ios_plist;
        i_plist.close();
        std::cout << "   [OK] Created 'ios/Info.plist'\n";
    }

    // 9.1 Write web/build.bat
    std::string web_build_bat = R"raw(@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Web Runner                   ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

echo   [1/2]   Transpiling lib/main.zen -^> main.html ...
%COMPILER% ../lib/main.zen -target web -o main.html
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    HTML + JS output ready: web/main.html

echo   [2/2]   Launching  in default browser ...
start main.html

echo.
echo   [OK] Browser launched
echo.
echo   Tip: Serve with a local HTTP server for full AJAX support:
echo         python -m http.server 8080
echo.

endlocal
)raw";

    std::ofstream w_bat(project_path / "web" / "build.bat");
    if (w_bat.is_open()) {
        w_bat << web_build_bat;
        w_bat.close();
        std::cout << "   [OK] Created 'web/build.bat'\n";
    }

    // 9.2 Write web/build.sh
    std::string web_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN WEB (Mac/Linux) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Web Runner                  |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

echo -e "  ${BOLD}[1/2]${RESET}   Transpiling lib/main.zen -> main.html ..."
$COMPILER ../lib/main.zen -target web -o main.html
if [ $? -ne 0 ]; then
    echo -e "  ${RED}[ERROR]${RESET} Transpile failed."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    HTML + JS output ready: web/main.html"

echo -e "  ${BOLD}[2/2]${RESET}   Launching  in default browser ..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    open main.html
else
    xdg-open main.html 2>/dev/null || sensible-browser main.html 2>/dev/null
fi

echo ""
echo -e "  ${GREEN}[OK]${RESET} Browser launched"
echo ""
echo -e "  ${YELLOW}[TIP]${RESET} Serve with a local HTTP server for full AJAX support:"
echo -e "  ${DIM}        python3 -m http.server 8080${RESET}"
echo ""
)raw";

    std::ofstream w_sh(project_path / "web" / "build.sh");
    if (w_sh.is_open()) {
        w_sh << web_build_sh;
        w_sh.close();
        std::cout << "   [OK] Created 'web/build.sh'\n";
    }

    // 9.3 Write web/build_wasm.bat
    std::string web_build_wasm_bat = R"raw(@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  WASM Runner                  ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

echo   [1/3]   Transpiling lib/main.zen -^> main_wasm.html ...
%COMPILER% ../lib/main.zen -target wasm -o main_wasm.html
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    WASM wrapper output ready: web/main_wasm.html

echo   [2/3]   Starting Python HTTP server on port 8000 ...
start /b python -m http.server 8000 2>nul
if %errorlevel% neq 0 (
    echo   [WARN]  Could not start Python HTTP server automatically.
) else (
    echo   [OK]    Server running in background on port 8000
)

echo   [3/3]   Launching  in default browser ...
start http://localhost:8000/main_wasm.html

echo.
echo   [OK] Browser launched to http://localhost:8000/main_wasm.html
echo.

endlocal
)raw";

    std::ofstream w_wasm_bat(project_path / "web" / "build_wasm.bat");
    if (w_wasm_bat.is_open()) {
        w_wasm_bat << web_build_wasm_bat;
        w_wasm_bat.close();
        std::cout << "   [OK] Created 'web/build_wasm.bat'\n";
    }

    // 9.4 Write web/build_wasm.sh
    std::string web_build_wasm_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN WASM (Mac/Linux) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  WASM Runner                |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main_wasm.html ..."
$COMPILER ../lib/main.zen -target wasm -o main_wasm.html
if [ $? -ne 0 ]; then
    echo -e "  ${RED}[ERROR]${RESET} Transpile failed."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    WASM wrapper output ready: web/main_wasm.html"

echo -e "  ${BOLD}[2/3]${RESET}   Starting Python HTTP server on port 8000 ..."
python3 -m http.server 8000 > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

# Check if the server is still running
if ps -p $SERVER_PID > /dev/null; then
    echo -e "  ${GREEN}[OK]${RESET}    Server started on port 8000 (PID: $SERVER_PID)"
else
    # Fallback to python
    python -m http.server 8000 > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    if ps -p $SERVER_PID > /dev/null; then
        echo -e "  ${GREEN}[OK]${RESET}    Server started on port 8000 (PID: $SERVER_PID)"
    else
        echo -e "  ${YELLOW}[WARN]${RESET}  Failed to launch HTTP server automatically."
    fi
fi

echo -e "  ${BOLD}[3/3]${RESET}   Launching  in default browser ..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    open http://localhost:8000/main_wasm.html
else
    xdg-open http://localhost:8000/main_wasm.html 2>/dev/null || sensible-browser http://localhost:8000/main_wasm.html 2>/dev/null
fi

echo ""
echo -e "  ${GREEN}[OK]${RESET} Browser launched to http://localhost:8000/main_wasm.html"
echo ""
)raw";

    std::ofstream w_wasm_sh(project_path / "web" / "build_wasm.sh");
    if (w_wasm_sh.is_open()) {
        w_wasm_sh << web_build_wasm_sh;
        w_wasm_sh.close();
        std::cout << "   [OK] Created 'web/build_wasm.sh'\n";
    }

    // 10. Write web/index.html (Dashboard portal)
    std::string web_index_html = R"raw(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Zenith Web Portal</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #0b0f19;
            --card-bg: rgba(255, 255, 255, 0.03);
            --border-color: rgba(255, 255, 255, 0.08);
            --text-primary: #ffffff;
            --text-secondary: #94a3b8;
            --accent-js: #f7df1e;
            --accent-wasm: #654ff0;
            --accent-glow: rgba(99, 102, 241, 0.15);
        }
        body {
            margin: 0;
            font-family: 'Outfit', sans-serif;
            background-color: var(--bg-color);
            color: var(--text-primary);
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            min-height: 100vh;
            overflow-x: hidden;
            background-image: 
                radial-gradient(circle at 10% 20%, rgba(99, 102, 241, 0.05) 0%, transparent 40%),
                radial-gradient(circle at 90% 80%, rgba(139, 92, 246, 0.05) 0%, transparent 40%);
        }
        .container {
            max-width: 800px;
            width: 90%;
            text-align: center;
            padding: 40px 20px;
        }
        h1 {
            font-size: 3rem;
            font-weight: 700;
            margin-bottom: 10px;
            background: linear-gradient(135deg, #a5b4fc, #c084fc);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            letter-spacing: -0.02em;
        }
        .subtitle {
            color: var(--text-secondary);
            font-size: 1.1rem;
            margin-bottom: 40px;
            font-weight: 300;
        }
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 24px;
            margin-bottom: 40px;
        }
        .card {
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 16px;
            padding: 30px;
            text-align: left;
            backdrop-filter: blur(12px);
            transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
            position: relative;
            overflow: hidden;
        }
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: linear-gradient(135deg, rgba(255,255,255,0.05), transparent);
            opacity: 0;
            transition: opacity 0.3s ease;
        }
        .card:hover {
            transform: translateY(-5px);
            border-color: rgba(99, 102, 241, 0.3);
            box-shadow: 0 10px 30px -10px var(--accent-glow);
        }
        .card:hover::before {
            opacity: 1;
        }
        .card-tag {
            display: inline-block;
            padding: 4px 10px;
            font-size: 0.75rem;
            font-weight: 600;
            border-radius: 20px;
            margin-bottom: 15px;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        .card-tag.js {
            background-color: rgba(247, 223, 30, 0.1);
            color: var(--accent-js);
            border: 1px solid rgba(247, 223, 30, 0.2);
        }
        .card-tag.wasm {
            background-color: rgba(101, 79, 240, 0.15);
            color: #a78bfa;
            border: 1px solid rgba(101, 79, 240, 0.25);
        }
        h2 {
            margin: 0 0 10px 0;
            font-size: 1.5rem;
            font-weight: 600;
        }
        p {
            color: var(--text-secondary);
            font-size: 0.95rem;
            line-height: 1.5;
            margin: 0 0 25px 0;
            font-weight: 300;
        }
        .btn {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            padding: 12px 24px;
            border-radius: 8px;
            font-weight: 600;
            font-size: 0.95rem;
            text-decoration: none;
            transition: all 0.2s ease;
            cursor: pointer;
            width: calc(100% - 48px);
            box-sizing: border-box;
            border: none;
        }
        .btn.js {
            background-color: var(--accent-js);
            color: #000000;
        }
        .btn.js:hover {
            background-color: #ffe62b;
            transform: scale(1.02);
        }
        .btn.wasm {
            background: linear-gradient(135deg, #6366f1, #8b5cf6);
            color: #ffffff;
        }
        .btn.wasm:hover {
            filter: brightness(1.1);
            transform: scale(1.02);
            box-shadow: 0 0 15px rgba(99, 102, 241, 0.4);
        }
        .footer {
            margin-top: 20px;
            color: rgba(255, 255, 255, 0.2);
            font-size: 0.85rem;
            font-weight: 300;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Zenith Web Explorer</h1>
        <div class="subtitle">Platform-segregated Web Application Deployments</div>
        
        <div class="cards-grid">
            <div class="card">
                <span class="card-tag js">JavaScript</span>
                <h2>Standard Web App</h2>
                <p>Compiles Zenith UI declarative widgets directly into hyper-optimized Javascript client files using a lightweight runtime.</p>
                <a href="main.html" class="btn js">Launch Web Build</a>
            </div>
            
            <div class="card">
                <span class="card-tag wasm">WebAssembly</span>
                <h2>WASM Binary App</h2>
                <p>Compiles Zenith AST to intermediate WebAssembly Text (WAT) representation, executing layout systems with high-performance low-level pipelines.</p>
                <a href="main_wasm.html" class="btn wasm">Launch WASM Build</a>
            </div>
        </div>
        
        <div class="footer">
            Zenith Engine &bull; Created with Zenith compiler create template
        </div>
    </div>
</body>
</html>
)raw";

    std::ofstream w_portal(project_path / "web" / "index.html");
    if (w_portal.is_open()) {
        w_portal << web_index_html;
        w_portal.close();
        std::cout << "   [OK] Created 'web/index.html'\n";
    }

    // Write windows/build.bat
    std::string windows_build_bat = R"raw(@echo off
setlocal

echo.
echo   +==================================================+
echo   ^|          Zenith  *  Windows Native Runner        ^|
echo   +==================================================+
echo.

set COMPILER=
if exist "..\zenith.exe"    set COMPILER=..\zenith.exe
if exist "..\..\zenith.exe" set COMPILER=..\..\zenith.exe
if not defined COMPILER     set COMPILER=zenith
echo   Using Zenith compiler: %COMPILER%
echo.

for /f "tokens=*" %%c in ('where g++ 2^>nul') do set GCC_PATH=%%c
if defined GCC_PATH (
    echo   [OK]    Toolchain: g++ - MinGW/MSYS2
) else (
    echo   [ERROR] g++ not found. Install MinGW and add to PATH.
    exit /b 1
)

echo.
echo   [1/3]   Transpiling lib/main.zen -^> main.cpp ...
%COMPILER% ../lib/main.zen -target cpp -o main.cpp
if %errorlevel% neq 0 (
    echo   [ERROR] Transpile failed.
    exit /b %errorlevel%
)
echo   [OK]    C++ source ready

echo   [2/3]   Compiling  C++17 -^> windows/main_app.exe ...
g++ -O3 -std=c++17 main.cpp -I ../include -o main_app.exe -lwinhttp
if %errorlevel% neq 0 (
    echo   [ERROR] Native compilation failed.
    exit /b %errorlevel%
)
echo   [OK]    Build complete: windows/main_app.exe

echo   [3/3]   Launching  Zenith Windows App ...
echo.
echo   +-------------------------------------------------+
echo   ^|  Zenith App Output                              ^|
echo   +-------------------------------------------------+
echo.

.\main_app.exe

echo.
echo   [OK] App exited successfully
echo.

endlocal
)raw";

    std::ofstream win_bat(project_path / "windows" / "build.bat");
    if (win_bat.is_open()) {
        win_bat << windows_build_bat;
        win_bat.close();
        std::cout << "   [OK] Created 'windows/build.bat'\n";
    }

    // Write windows/build.sh
    std::string windows_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN WINDOWS (via Wine/WSL) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Windows Runner (WSL/Wine)   |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

if command -v x86_64-w64-mingw32-g++ &>/dev/null; then
    CXX="x86_64-w64-mingw32-g++"
    echo -e "  ${GREEN}[OK]${RESET}    Toolchain: MinGW cross-compiler"
elif command -v g++ &>/dev/null; then
    CXX="g++"
    echo -e "  ${GREEN}[OK]${RESET}    Toolchain: g++ (native)"
else
    echo -e "  ${RED}[ERROR]${RESET} g++ not found."
    exit 1
fi
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling  C++17 -> windows/main_app.exe ..."
$CXX -O3 -std=c++17 main.cpp -I ../include -o main_app.exe
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Compilation failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: windows/main_app.exe"

echo -e "  ${BOLD}[3/3]${RESET}   Launching ..."
if command -v wine &>/dev/null; then
    echo ""
    echo "  +-------------------------------------------------+"
    echo "  |  Zenith App Output (via Wine)                   |"
    echo "  +-------------------------------------------------+"
    echo ""
    wine ./main_app.exe
else
    echo -e "  ${YELLOW}[INFO]${RESET} Run main_app.exe on a Windows machine or via Wine."
fi

echo ""
echo -e "  ${GREEN}[OK]${RESET} Done"
echo ""
)raw";

    std::ofstream win_sh(project_path / "windows" / "build.sh");
    if (win_sh.is_open()) {
        win_sh << windows_build_sh;
        win_sh.close();
        std::cout << "   [OK] Created 'windows/build.sh'\n";
    }

    // Write linux/build.sh
    std::string linux_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN LINUX - Flutter-style build runner
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Linux Native Runner         |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

# -- CHECK: Host info --
KERNEL=$(uname -r 2>/dev/null || echo "unknown")
echo -e "  ${GREEN}[OK]${RESET}    Platform: Linux (kernel $KERNEL)"
if ! command -v g++ &>/dev/null; then
    echo -e "  ${RED}[ERROR]${RESET} g++ not found. Run: sudo apt install build-essential"
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    Toolchain: g++"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling  C++17 -> linux/main_app ..."
g++ -O3 -std=c++17 main.cpp -I ../include -o main_app -lpthread
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Compilation failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: linux/main_app"

echo -e "  ${BOLD}[3/3]${RESET}   Launching  Zenith Linux App ..."
echo ""
echo "  +-------------------------------------------------+"
echo "  |  Zenith App Output                              |"
echo "  +-------------------------------------------------+"
echo ""

./main_app

echo ""
echo -e "  ${GREEN}[OK]${RESET} App exited successfully"
echo ""
)raw";

    std::ofstream lin_sh(project_path / "linux" / "build.sh");
    if (lin_sh.is_open()) {
        lin_sh << linux_build_sh;
        lin_sh.close();
        std::cout << "   [OK] Created 'linux/build.sh'\n";
    }

    // Write mac/build.sh
    std::string mac_build_sh = R"raw(#!/bin/bash
# =====================================================
#   ZENITH RUN MACOS - Flutter-style build runner
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  macOS Native Runner         |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

# -- CHECK: macOS version --
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo -e "  ${YELLOW}[WARN]${RESET} macOS runner detected on non-macOS host."
fi
MACOS_VER=$(sw_vers -productVersion 2>/dev/null || echo "unknown")
echo -e "  ${GREEN}[OK]${RESET}    Platform: macOS $MACOS_VER"
if ! command -v clang++ &>/dev/null; then
    echo -e "  ${RED}[ERROR]${RESET} clang++ not found. Install Xcode Command Line Tools:"
    echo -e "          xcode-select --install"
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    Toolchain: clang++ (Apple)"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling  C++17 -> mac/main_app ..."
clang++ -O3 -std=c++17 main.cpp -I ../include -o main_app -lpthread -framework CoreFoundation
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Compilation failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: mac/main_app"

echo -e "  ${BOLD}[3/3]${RESET}   Launching  Zenith macOS App ..."
echo ""
echo "  +-------------------------------------------------+"
echo "  ^|  Zenith App Output                              ^|"
echo "  +-------------------------------------------------+"
echo ""

./main_app

echo ""
echo -e "  ${GREEN}[OK]${RESET} App exited successfully"
echo ""
)raw";

    std::ofstream macos_sh(project_path / "mac" / "build.sh");
    if (macos_sh.is_open()) {
        macos_sh << mac_build_sh;
        macos_sh.close();
        std::cout << "   [OK] Created 'mac/build.sh'\n";
    }

#ifndef _WIN32
    std::string chmod_cmd = "chmod +x \"" + (project_path / "desktop" / "build.sh").string() + "\" \"" +
                             (project_path / "android" / "build.sh").string() + "\" \"" +
                             (project_path / "ios" / "build.sh").string() + "\" \"" +
                             (project_path / "web" / "build.sh").string() + "\" \"" +
                             (project_path / "web" / "build_wasm.sh").string() + "\" \"" +
                             (project_path / "windows" / "build.sh").string() + "\" \"" +
                             (project_path / "linux" / "build.sh").string() + "\" \"" +
                             (project_path / "mac" / "build.sh").string() + "\" 2>/dev/null";
    system(chmod_cmd.c_str());
#endif

    std::cout << "\n===================================================\n";
    std::cout << "   Zenith Project Bootstrapped Successfully!\n";
    std::cout << "===================================================\n";
    std::cout << "Project layout organized by platform directories:\n";
    std::cout << "  android/   - Native Android NDK configurations\n";
    std::cout << "  ios/       - Apple Xcode/xcrun configuration\n";
    std::cout << "  web/       - JavaScript & WASM web portal and targets\n";
    std::cout << "  desktop/   - Native OS platform compiler driver\n";
    std::cout << "  linux/     - Linux specific compilation scripts\n";
    std::cout << "  windows/   - Windows specific compilation scripts\n";
    std::cout << "  mac/       - macOS specific compilation scripts\n";
    std::cout << "  include/   - Standard Zenith C++ helper headers\n";
    std::cout << "  lib/       - Zenith source code directory\n";
    std::cout << "    main.zen - App source entrypoint (edit this!)\n";
    std::cout << "===================================================\n";
    std::cout << "To compile and run your application:\n";
    std::cout << "  zenith run <desktop|windows|linux|mac|web|wasm|android|ios>\n";
    std::cout << "===================================================\n";
    
    return 0;
}

bool compileProject(const std::string& filename, const std::string& target, const std::string& out_filename_override = "") {
    if (filename.length() < 5 || filename.substr(filename.length() - 4) != ".zen") {
        std::cerr << "Error: Input file must have .zen extension\n";
        return false;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << "\n";
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string code = buffer.str();
    file.close();

    std::cout << "--- Zenith Compiler v0.2.0 ---\n";
    std::cout << "1. Lexing source: " << filename << "...\n";
    
    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    std::cout << "   [OK] Generated " << tokens.size() << " tokens.\n";

    std::cout << "2. Parsing AST...\n";
    Parser parser(tokens);
    auto ast = parser.parseProgram();
    std::cout << "   [OK] AST Parsed Successfully.\n";

    // Resolve custom imports recursively relative to the main file's path
    std::set<std::string> loaded_files;
    loaded_files.insert(filename);
    resolveImports(ast.get(), loaded_files, getDirectory(filename));

    std::cout << "3. Running Semantic Analysis...\n";
    SemanticAnalyzer analyzer;
    if (!analyzer.analyze(ast.get())) {
        return false;
    }

    std::cout << "4. Running Code Generator (Target: " << target << ")...\n";
    std::string transpiled_code;
    std::string out_filename;

    if (target == "web") {
        JSCodeGenerator codegen;
        transpiled_code = codegen.generate(ast.get());
        if (!out_filename_override.empty()) {
            out_filename = out_filename_override;
        } else {
            out_filename = filename.substr(0, filename.length() - 4) + ".html";
        }
    } else if (target == "wasm") {
        WASMCodeGenerator codegen;
        std::string wat_code = codegen.generate(ast.get());
        std::string html_wrapper = codegen.generateHTMLWrapper();
        
        std::string wat_filename, html_filename;
        if (!out_filename_override.empty()) {
            html_filename = out_filename_override;
            std::filesystem::path html_path(html_filename);
            wat_filename = (html_path.parent_path() / "main.wat").string();
        } else {
            wat_filename = filename.substr(0, filename.length() - 4) + ".wat";
            html_filename = filename.substr(0, filename.length() - 4) + "_wasm.html";
        }
        
        std::ofstream wat_file(wat_filename);
        wat_file << wat_code;
        wat_file.close();
        
        std::ofstream html_file(html_filename);
        html_file << html_wrapper;
        html_file.close();
        
        std::cout << "   [OK] Written WAT output to: " << wat_filename << "\n";
        std::cout << "   [OK] Written HTML loader to: " << html_filename << "\n";
        return true;
    } else {
        CodeGenerator codegen;
        transpiled_code = codegen.generate(ast.get());
        if (!out_filename_override.empty()) {
            out_filename = out_filename_override;
        } else {
            out_filename = filename.substr(0, filename.length() - 4) + ".cpp";
        }
    }

    std::ofstream out_file(out_filename);
    if (!out_file.is_open()) {
        std::cerr << "Error: Could not open output file " << out_filename << "\n";
        return false;
    }
    out_file << transpiled_code;
    out_file.close();
    std::cout << "   [OK] Written " << (target == "web" ? "HTML" : "C++") << " transpiled output to: " << out_filename << "\n";

    return true;
}

int runPlatformProject(const std::string& platform, const std::string& argv0) {
    namespace fs = std::filesystem;
    if (!fs::exists("lib/main.zen")) {
        std::cerr << "Error: 'lib/main.zen' not found in current directory.\n";
        std::cerr << "Make sure you are in a Zenith project directory.\n";
        return 1;
    }

    std::string cmd = "";
    if (platform == "desktop") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith Desktop (via desktop/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        cmd = "cd desktop && build.bat";
#else
        cmd = "cd desktop && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "windows") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith Windows (via windows/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        cmd = "cd windows && build.bat";
#else
        cmd = "cd windows && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "linux") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith Linux (via linux/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        std::cerr << "Error: Running Linux apps directly is not supported on Windows without WSL.\n";
        return 1;
#else
        cmd = "cd linux && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "mac" || platform == "macos") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith macOS (via mac/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        std::cerr << "Error: Running macOS apps is only supported on macOS.\n";
        return 1;
#else
        cmd = "cd mac && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "web") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith Web (via web/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        cmd = "cd web && build.bat";
#else
        cmd = "cd web && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "wasm") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith WebAssembly (via web/build_wasm)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        cmd = "cd web && build_wasm.bat";
#else
        cmd = "cd web && chmod +x build_wasm.sh && ./build_wasm.sh";
#endif
    }
    else if (platform == "android") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith Android (via android/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        cmd = "cd android && build.bat";
#else
        cmd = "cd android && chmod +x build.sh && ./build.sh";
#endif
    }
    else if (platform == "ios") {
        std::cout << "===================================================\n";
        std::cout << "   Running Zenith iOS (via ios/build)\n";
        std::cout << "===================================================\n";
#ifdef _WIN32
        std::cerr << "Error: Running iOS apps is only supported on macOS.\n";
        return 1;
#else
        cmd = "cd ios && chmod +x build.sh && ./build.sh";
#endif
    }
    else {
        std::cerr << "Error: Unknown platform: " << platform << "\n";
        std::cerr << "Supported platforms: desktop, windows, linux, mac, web, wasm, android, ios\n";
        return 1;
    }

    std::cout << "Executing: " << cmd << "\n";
    int res = system(cmd.c_str());
    if (res != 0) {
        std::cerr << "Error: Execution of build script failed with code " << res << "\n";
        return res;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::string(argv[1]) == "create") {
        if (argc < 3) {
            std::cerr << "Usage: zenith create <project_name|.>\n";
            return 1;
        }
        std::string project_name = argv[2];
        return runCreateProject(project_name, argv[0]);
    }

    if (argc >= 2 && std::string(argv[1]) == "run") {
        if (argc < 3) {
            std::cerr << "Usage: zenith run <desktop|web|wasm|android|ios>\n";
            return 1;
        }
        std::string platform = argv[2];
        return runPlatformProject(platform, argv[0]);
    }

    std::string target = "cpp";
    std::string filename = "";
    std::string out_filename_override = "";
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-target") {
            if (i + 1 < argc) {
                target = argv[++i];
            } else {
                std::cerr << "Error: -target option requires a value (cpp, web, or wasm)\n";
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                out_filename_override = argv[++i];
            } else {
                std::cerr << "Error: -o option requires a file path\n";
                return 1;
            }
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Usage: zenith <filename.zen> [-target <cpp|web|wasm>] [-o <output_file>]\n";
        std::cerr << "       zenith create <project_name|.>\n";
        std::cerr << "       zenith run <desktop|web|wasm|android|ios>\n";
        return 1;
    }

    if (!compileProject(filename, target, out_filename_override)) {
        return 1;
    }

    return 0;
}
