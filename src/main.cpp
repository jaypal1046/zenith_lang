#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <memory>
#include <array>
#include <thread>
#include <chrono>
#include <regex>
#include <algorithm>
#include <atomic>
#include <iomanip>
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
#else
  #include <unistd.h>
  #include <signal.h>
  #include <sys/types.h>
  #include <fcntl.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif
#include "../include/frontend/lexer.h"
#include "../include/frontend/parser.h"
#include "../include/frontend/semantic.h"
#include "../include/frontend/formatter.h"
#include "../include/lsp/lsp.h"
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
            std::string full_path = "";
            
            if (mod.length() > 4 && mod.substr(mod.length() - 4) == ".zen") {
                full_path = current_dir + mod;
            } else if (imp->kind == ImportNode::ImportKind::Zen) {
                // If it is a zen package import, look in lib/<package_name>/main.zen
                full_path = "lib/" + mod + "/main.zen";
            }
            
            if (!full_path.empty()) {
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

struct BridgeFunction {
    std::string ret;
    std::string name;
    std::vector<std::pair<std::string, std::string>> args; // type, name
};

std::vector<BridgeFunction> parseBridgeSpecs(const std::string& spec_str) {
    std::vector<BridgeFunction> funcs;
    std::regex func_regex("([A-Za-z0-9_<>]+)\\s+([A-Za-z0-9_]+)\\s*\\((.*?)\\)");
    std::smatch m;
    
    std::stringstream ss(spec_str);
    std::string token;
    while (std::getline(ss, token, ';')) {
        // trim whitespace
        token = std::regex_replace(token, std::regex("^\\s+|\\s+$"), "");
        if (token.empty()) continue;
        
        if (std::regex_search(token, m, func_regex)) {
            BridgeFunction fn;
            fn.ret = m[1].str();
            fn.name = m[2].str();
            
            std::string args_str = m[3].str();
            std::stringstream ass(args_str);
            std::string arg;
            std::regex arg_regex("([A-Za-z0-9_<>]+)\\s+([A-Za-z0-9_]+)");
            std::smatch am;
            while (std::getline(ass, arg, ',')) {
                arg = std::regex_replace(arg, std::regex("^\\s+|\\s+$"), "");
                if (arg.empty()) continue;
                
                if (std::regex_search(arg, am, arg_regex)) {
                    fn.args.push_back({am[1].str(), am[2].str()});
                }
            }
            funcs.push_back(fn);
        }
    }
    return funcs;
}

std::string genRustLib(const std::vector<BridgeFunction>& funcs, const std::string& package_name) {
    std::stringstream code;
    code << "use std::ffi::{CString, CStr};\n";
    code << "use std::os::raw::c_char;\n\n";
    
    for (const auto& f : funcs) {
        std::vector<std::string> rust_args;
        std::vector<std::string> conversions;
        std::vector<std::string> arg_names;
        
        for (const auto& arg : f.args) {
            std::string arg_type = arg.first;
            std::string arg_name = arg.second;
            if (arg_type == "String") {
                rust_args.push_back(arg_name + ": *const c_char");
                conversions.push_back("    let " + arg_name + " = unsafe { CStr::from_ptr(" + arg_name + ").to_string_lossy().into_owned() };");
            } else if (arg_type == "Int") {
                rust_args.push_back(arg_name + ": i32");
            } else if (arg_type == "Float") {
                rust_args.push_back(arg_name + ": f64");
            } else if (arg_type == "Bool") {
                rust_args.push_back(arg_name + ": bool");
            }
            arg_names.push_back(arg_name);
        }
        
        std::string args_decl = "";
        for (size_t i = 0; i < rust_args.size(); ++i) {
            args_decl += rust_args[i];
            if (i < rust_args.size() - 1) args_decl += ", ";
        }
        
        std::string ret_type = "";
        if (f.ret == "String") ret_type = "-> *mut c_char";
        else if (f.ret == "Int") ret_type = "-> i32";
        else if (f.ret == "Float") ret_type = "-> f64";
        else if (f.ret == "Bool") ret_type = "-> bool";
        
        code << "#[no_mangle]\n";
        code << "pub extern \"C\" fn " << f.name << "(" << args_decl << ") " << ret_type << " {\n";
        for (const auto& conv : conversions) {
            code << conv << "\n";
        }
        code << "    // Custom implementation using the dependency here\n";
        if (package_name == "uuid" && f.name == "generate_uuid") {
            code << "    let res = uuid::Uuid::new_v4().to_string();\n";
        } else {
            code << "    let res = format!(\"Rust [" << package_name << "] Result called\");\n";
        }
        
        if (f.ret == "String") {
            code << "    CString::new(res).unwrap().into_raw()\n";
        } else if (f.ret == "Int") {
            code << "    42\n";
        } else if (f.ret == "Float") {
            code << "    3.14159\n";
        } else if (f.ret == "Bool") {
            code << "    true\n";
        }
        code << "}\n\n";
    }
    return code.str();
}

std::string genDartLib(const std::vector<BridgeFunction>& funcs, const std::string& package_name) {
    std::stringstream code;
    code << "import 'dart:convert';\n";
    code << "import 'package:" << package_name << "/" << package_name << ".dart';\n\n";
    
    for (const auto& f : funcs) {
        std::vector<std::string> dart_args;
        for (const auto& arg : f.args) {
            std::string arg_type = arg.first;
            std::string arg_name = arg.second;
            if (arg_type == "String") dart_args.push_back("String " + arg_name);
            else if (arg_type == "Int") dart_args.push_back("int " + arg_name);
            else if (arg_type == "Float") dart_args.push_back("double " + arg_name);
            else if (arg_type == "Bool") dart_args.push_back("bool " + arg_name);
        }
        
        std::string args_decl = "";
        for (size_t i = 0; i < dart_args.size(); ++i) {
            args_decl += dart_args[i];
            if (i < dart_args.size() - 1) args_decl += ", ";
        }
        
        std::string ret_type = "void";
        if (f.ret == "String") ret_type = "String";
        else if (f.ret == "Int") ret_type = "int";
        else if (f.ret == "Float") ret_type = "double";
        else if (f.ret == "Bool") ret_type = "bool";
        
        code << "@pragma('wasm:export', '" << f.name << "')\n";
        code << ret_type << " " << f.name << "(" << args_decl << ") {\n";
        if (package_name == "crypto" && f.name == "sha256") {
            code << "  var bytes = utf8.encode(input);\n";
            code << "  return sha256.convert(bytes).toString();\n";
        } else {
            code << "  return 'Dart [" << package_name << "] Result called';\n";
        }
        code << "}\n\n";
    }
    code << "void main() {}\n";
    return code.str();
}

std::string genZenithWrapper(const std::vector<BridgeFunction>& funcs, const std::string& package_name, const std::string& kind) {
    std::stringstream code;
    code << "// Zenith wrapper bindings for " << package_name << "\n";
    code << "// Auto-generated by zenith bridge subcommand\n\n";
    
    if (kind == "rust") {
        code << "import native \"lib/" << package_name << "/bridge.dll\" for \"cpp\";\n";
        code << "import cdn \"lib/" << package_name << "/bridge.wasm\" for \"web\";\n\n";
    } else {
        code << "import cdn \"lib/" << package_name << "/bridge.wasm\" for \"web\";\n\n";
    }
    
    std::string abi = (kind == "rust") ? "C" : "js";
    code << "foreign \"" << abi << "\" {\n";
    for (const auto& f : funcs) {
        std::vector<std::string> args_decl;
        for (const auto& arg : f.args) {
            args_decl.push_back(arg.first + " " + arg.second);
        }
        std::string args_str = "";
        for (size_t i = 0; i < args_decl.size(); ++i) {
            args_str += args_decl[i];
            if (i < args_decl.size() - 1) args_str += ", ";
        }
        code << "    " << f.ret << " " << f.name << "(" << args_str << ");\n";
    }
    code << "}\n";
    return code.str();
}

int runCreateProject(const std::string& project_name, const std::string& template_type, const std::string& argv0) {
    namespace fs = std::filesystem;
    fs::path project_path = (project_name == ".") ? fs::current_path() : fs::absolute(project_name);
    
    std::cout << "Creating new Zenith project (" << template_type << ") in: " << project_path.string() << "\n";
    
    try {
        fs::create_directories(project_path);
        fs::create_directories(project_path / "include");
        fs::create_directories(project_path / "lib");
        if (template_type == "app") {
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
        }
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
    
    if (template_type == "app") {
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
    }
    else if (template_type == "package") {
        // Write package main.zen
        std::ofstream main_file(project_path / "lib" / "main.zen");
        if (main_file.is_open()) {
            main_file << R"raw(// Zenith Logic Package
import std.io;

class Calculator() {
    Int add(Int a, Int b) {
        return a + b;
    }
    Int multiply(Int a, Int b) {
        return a * b;
    }
}

Void main() {
    println("Running logical package tests...");
    Calculator calc = Calculator();
    println("Add test: 10 + 20 = " + calc.add(10, 20));
}
)raw";
            main_file.close();
            std::cout << "   [OK] Created 'lib/main.zen'\n";
        }

        // Write package build.bat
        std::ofstream p_bat(project_path / "build.bat");
        if (p_bat.is_open()) {
            p_bat << R"raw(@echo off
echo.
echo   +==================================================+
echo   ^|    Zenith  *  Package Test Runner (Windows)       ^|
echo   +==================================================+
echo.
set COMPILER=zenith
if exist "zenith.exe" set COMPILER=.\zenith.exe
if exist "..\zenith.exe" set COMPILER=..\zenith.exe
%COMPILER% lib/main.zen -target cpp -o main.cpp
g++ -O3 -std=c++17 main.cpp -o main_app.exe -lws2_32
if %errorlevel% equ 0 (
    echo.
    echo   [OK] Build Succeeded! Running package tests:
    echo   --------------------------------------------------
    .\main_app.exe
    echo   --------------------------------------------------
)
)raw";
            p_bat.close();
            std::cout << "   [OK] Created 'build.bat' test runner\n";
        }
    }

    std::cout << "\n===================================================\n";
    if (template_type == "app") {
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
    } else {
        std::cout << "   Zenith Package Bootstrapped Successfully!\n";
        std::cout << "===================================================\n";
        std::cout << "Project layout organized for logical libraries:\n";
        std::cout << "  include/   - Standard Zenith C++ helper headers\n";
        std::cout << "  lib/       - Zenith source code directory\n";
        std::cout << "    main.zen - Package entrypoint & tests (edit this!)\n";
        std::cout << "===================================================\n";
        std::cout << "To build and run package tests:\n";
        std::cout << "  build.bat  (or compile directly with zenith)\n";
        std::cout << "===================================================\n";
    }
    
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
            std::cerr << "Usage: zenith create <project_name|.> [--template=app|package]\n";
            return 1;
        }
        std::string project_name = argv[2];
        std::string template_type = "app";
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--template=package" || arg == "package") {
                template_type = "package";
            } else if (arg == "--template=app" || arg == "app") {
                template_type = "app";
            }
        }
        return runCreateProject(project_name, template_type, argv[0]);
    }

    if (argc >= 2 && std::string(argv[1]) == "run") {
        if (argc < 3) {
            std::cerr << "Usage: zenith run <desktop|web|wasm|android|ios>\n";
            return 1;
        }
        std::string platform = argv[2];
        return runPlatformProject(platform, argv[0]);
    }

    // LSP subcommand
    if (argc >= 2 && (std::string(argv[1]) == "lsp" || std::string(argv[1]) == "--lsp")) {
        runLspServer();
        return 0;
    }

    // Format subcommand
    if (argc >= 2 && std::string(argv[1]) == "format") {
        bool write_in_place = false;
        std::string target_file = "";
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-w" || arg == "--write") {
                write_in_place = true;
            } else {
                target_file = arg;
            }
        }
        if (target_file.empty()) {
            std::cerr << "Usage: zenith format [-w] <file.zen>\n";
            return 1;
        }

        std::ifstream file(target_file);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << target_file << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string code = buffer.str();
        file.close();

        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        if (!ast) {
            std::cerr << "Format Error: Could not parse AST.\n";
            return 1;
        }

        Formatter formatter;
        std::string formatted = formatter.format(ast.get());

        if (write_in_place) {
            std::ofstream out_file(target_file);
            if (!out_file.is_open()) {
                std::cerr << "Error: Could not write to file " << target_file << "\n";
                return 1;
            }
            out_file << formatted;
            out_file.close();
            std::cout << "Formatted file: " << target_file << "\n";
        } else {
            std::cout << formatted;
        }
        return 0;
    }

    // Helper functions for Package Manager
    auto getPackageName = [](std::string url) -> std::string {
        while (!url.empty() && (url.back() == '/' || url.back() == '\\')) {
            url.pop_back();
        }
        if (url.length() > 4 && url.substr(url.length() - 4) == ".git") {
            url = url.substr(0, url.length() - 4);
        }
        size_t last_slash = url.find_last_of("/\\");
        if (last_slash != std::string::npos) {
            return url.substr(last_slash + 1);
        }
        return url;
    };

    // Install subcommand
    if (argc >= 2 && std::string(argv[1]) == "install") {
        std::string url = "";
        if (argc >= 3) {
            url = argv[2];
        }

        namespace fs = std::filesystem;
        fs::create_directories("lib");

        if (!url.empty()) {
            std::string package_name = getPackageName(url);
            std::string target_dir = "lib/" + package_name;
            std::string cmd = "git clone " + url + " " + target_dir;
            std::cout << "Installing package " << package_name << " from " << url << "...\n";
            int result = system(cmd.c_str());
            if (result != 0) {
                std::cerr << "Error: Git clone failed with code " << result << "\n";
                return 1;
            }
            std::cout << "[OK] Package " << package_name << " installed successfully under " << target_dir << ".\n";

            // Update zenith.json
            std::ifstream f_in("zenith.json");
            std::string content = "";
            if (f_in.is_open()) {
                std::stringstream buffer;
                buffer << f_in.rdbuf();
                content = buffer.str();
                f_in.close();
            }

            if (content.empty()) {
                std::ofstream f_out("zenith.json");
                f_out << "{\n  \"dependencies\": {\n    \"" << package_name << "\": \"" << url << "\"\n  }\n}\n";
                f_out.close();
            } else {
                if (content.find(url) == std::string::npos) {
                    size_t deps = content.find("\"dependencies\"");
                    if (deps != std::string::npos) {
                        size_t closing_brace = content.find("}", deps);
                        if (closing_brace != std::string::npos) {
                            size_t last_comma_search = content.rfind("\"", closing_brace);
                            std::string comma = "";
                            if (last_comma_search != std::string::npos && last_comma_search > deps) {
                                comma = ",\n";
                            }
                            std::string insertion = comma + "    \"" + package_name + "\": \"" + url + "\"";
                            content.insert(closing_brace, insertion);
                            std::ofstream f_out("zenith.json");
                            f_out << content;
                            f_out.close();
                        }
                    } else {
                        std::ofstream f_out("zenith.json");
                        f_out << "{\n  \"dependencies\": {\n    \"" << package_name << "\": \"" << url << "\"\n  }\n}\n";
                        f_out.close();
                    }
                }
            }
        } else {
            // Restore all packages from zenith.json
            std::ifstream f_in("zenith.json");
            if (!f_in.is_open()) {
                std::cerr << "Usage: zenith install <package-url> OR create zenith.json dependencies.\n";
                return 1;
            }
            std::stringstream buffer;
            buffer << f_in.rdbuf();
            std::string content = buffer.str();
            f_in.close();

            std::regex dep_regex("\"([a-zA-Z0-9_\\-]+)\"\\s*:\\s*\"([^\"]+)\"");
            auto words_begin = std::sregex_iterator(content.begin(), content.end(), dep_regex);
            auto words_end = std::sregex_iterator();
            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::string dep_name = (*i)[1].str();
                std::string dep_url = (*i)[2].str();
                if (dep_name == "dependencies") continue;
                
                std::string target_dir = "lib/" + dep_name;
                if (!fs::exists(target_dir)) {
                    std::string cmd = "git clone " + dep_url + " " + target_dir;
                    std::cout << "Installing missing dependency " << dep_name << " from " << dep_url << "...\n";
                    system(cmd.c_str());
                } else {
                    std::cout << "Dependency " << dep_name << " is already installed.\n";
                }
            }
        }
        return 0;
    }

    // =========================================================================
    // LIST subcommand: zenith list
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "list") {
        namespace fs = std::filesystem;
        std::cout << "\n\033[1m\033[96m╔══════════════════════════════════════╗\033[0m\n";
        std::cout << "\033[1m\033[96m║   Zenith Package Manager — Installed  ║\033[0m\n";
        std::cout << "\033[1m\033[96m╚══════════════════════════════════════╝\033[0m\n\n";

        // Read zenith.json
        std::ifstream f_in("zenith.json");
        std::string content = "";
        if (f_in.is_open()) {
            std::stringstream buffer; buffer << f_in.rdbuf();
            content = buffer.str(); f_in.close();
        }

        // Parse dependencies
        std::vector<std::pair<std::string,std::string>> deps;
        std::regex dep_regex("\"([a-zA-Z0-9_\\-]+)\"\\s*:\\s*\"([^\"]+)\"");
        auto words_begin = std::sregex_iterator(content.begin(), content.end(), dep_regex);
        auto words_end = std::sregex_iterator();
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::string name = (*i)[1].str();
            std::string url = (*i)[2].str();
            if (name != "dependencies") deps.push_back({name, url});
        }

        if (deps.empty()) {
            std::cout << "  \033[33mNo packages declared in zenith.json\033[0m\n";
        } else {
            std::cout << "  \033[1mPackage\033[0m             \033[1mSource\033[0m\n";
            std::cout << "  " << std::string(50, '-') << "\n";
            for (const auto& dep : deps) {
                std::string installed_str = fs::exists("lib/" + dep.first) ? " \033[32m✓ installed\033[0m" : " \033[31m✗ missing\033[0m";
                std::cout << "  \033[96m" << dep.first << "\033[0m" << std::string(std::max(1,(int)(20-dep.first.size())), ' ')
                          << dep.second << installed_str << "\n";
            }
        }

        // Also scan lib/ for packages not in zenith.json
        std::cout << "\n  \033[1mScanned lib/ directory:\033[0m\n";
        if (fs::exists("lib")) {
            bool found = false;
            for (const auto& entry : fs::directory_iterator("lib")) {
                if (entry.is_directory()) {
                    found = true;
                    bool in_json = false;
                    for (const auto& d : deps) { if (d.first == entry.path().filename().string()) in_json = true; }
                    std::cout << "  \033[36m  " << entry.path().filename().string() << "\033[0m"
                              << (in_json ? " (registered)" : " \033[33m(unregistered)\033[0m") << "\n";
                }
            }
            if (!found) std::cout << "  \033[2m  (empty)\033[0m\n";
        } else {
            std::cout << "  \033[2m  lib/ not found\033[0m\n";
        }
        std::cout << "\n";
        return 0;
    }

    // =========================================================================
    // SEARCH subcommand: zenith search <query>
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "search") {
        std::string query = (argc >= 3) ? std::string(argv[2]) : "";
        std::cout << "\n\033[1m\033[95m╔══════════════════════════════════════╗\033[0m\n";
        std::cout << "\033[1m\033[95m║   Zenith Package Registry — Search    ║\033[0m\n";
        std::cout << "\033[1m\033[95m╚══════════════════════════════════════╝\033[0m\n\n";

        // Curated registry of known Zenith-compatible packages
        struct RegEntry { std::string name, desc, url, tags; };
        static const std::vector<RegEntry> registry = {
            {"zenith-ui",       "Core UI component library for Zenith apps",          "https://github.com/zenith-lang/zenith-ui.git",       "ui widgets components"},
            {"zenith-http",     "HTTP client with async/await for Zenith",             "https://github.com/zenith-lang/zenith-http.git",      "http networking async"},
            {"zenith-auth",     "Authentication and JWT utilities",                    "https://github.com/zenith-lang/zenith-auth.git",      "auth jwt security"},
            {"zenith-db",       "Database abstraction layer (SQLite/Postgres)",        "https://github.com/zenith-lang/zenith-db.git",        "database sql sqlite"},
            {"zenith-charts",   "Data visualization and charting widgets",             "https://github.com/zenith-lang/zenith-charts.git",    "charts graphs visualization"},
            {"zenith-i18n",     "Internationalization and localization support",       "https://github.com/zenith-lang/zenith-i18n.git",      "i18n l10n localization"},
            {"zenith-router",   "Client-side router for single-page apps",             "https://github.com/zenith-lang/zenith-router.git",    "routing navigation spa"},
            {"zenith-forms",    "Form validation and input management",                "https://github.com/zenith-lang/zenith-forms.git",     "forms validation input"},
            {"zenith-state",    "Global state management (Flux/Redux pattern)",        "https://github.com/zenith-lang/zenith-state.git",     "state management redux"},
            {"zenith-test",     "Unit testing framework for Zenith projects",          "https://github.com/zenith-lang/zenith-test.git",      "testing unit test"},
            {"Spoon-Knife",     "GitHub demo repository (default example)",            "https://github.com/octocat/Spoon-Knife.git",          "demo example"},
        };

        std::string q_lower = query;
        std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(), ::tolower);

        int found_count = 0;
        std::cout << "  \033[1m" << (query.empty() ? "All packages" : "Results for: \"" + query + "\"") << "\033[0m\n";
        std::cout << "  " << std::string(60, '-') << "\n";
        for (const auto& e : registry) {
            std::string combined = e.name + " " + e.desc + " " + e.tags;
            std::string c_lower = combined;
            std::transform(c_lower.begin(), c_lower.end(), c_lower.begin(), ::tolower);
            if (query.empty() || c_lower.find(q_lower) != std::string::npos) {
                std::cout << "\n  \033[1m\033[96m" << e.name << "\033[0m\n";
                std::cout << "    " << e.desc << "\n";
                std::cout << "    \033[2m" << e.url << "\033[0m\n";
                std::cout << "    \033[33mInstall:\033[0m zenith install " << e.url << "\n";
                found_count++;
            }
        }
        if (found_count == 0) {
            std::cout << "\n  \033[33mNo packages found for \"" << query << "\"\033[0m\n";
            std::cout << "  \033[2mTip: You can install any GitHub repository with: zenith install <git-url>\033[0m\n";
        }
        std::cout << "\n  \033[2mShowing " << found_count << " package(s) from Zenith registry.\033[0m\n\n";
        return 0;
    }

    // =========================================================================
    // UPDATE subcommand: zenith update [package]
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "update") {
        namespace fs = std::filesystem;
        std::string target_pkg = (argc >= 3) ? std::string(argv[2]) : "";
        std::cout << "\n\033[1m\033[92m[Zenith] Updating packages...\033[0m\n";

        auto do_update = [&](const std::string& name) {
            std::string pkg_path = "lib/" + name;
            if (!fs::exists(pkg_path)) {
                std::cout << "  \033[31m[!] Package '" << name << "' not found in lib/\033[0m\n";
                return;
            }
            std::cout << "  \033[96m→\033[0m Updating " << name << "... ";
            std::string cmd = "git -C " + pkg_path + " pull --ff-only 2>&1";
            int res = system(cmd.c_str());
            if (res == 0) std::cout << "\033[32m✓ up to date\033[0m\n";
            else std::cout << "\033[31m✗ update failed (exit " << res << ")\033[0m\n";
        };

        if (!target_pkg.empty()) {
            do_update(target_pkg);
        } else {
            // Update all from zenith.json
            std::ifstream f_in("zenith.json");
            if (!f_in.is_open()) {
                std::cerr << "  No zenith.json found. Run 'zenith install <url>' first.\n";
                return 1;
            }
            std::stringstream buffer; buffer << f_in.rdbuf();
            std::string content = buffer.str(); f_in.close();
            std::regex dep_regex("\"([a-zA-Z0-9_\\-]+)\"\\s*:\\s*\"([^\"]+)\"");
            auto it = std::sregex_iterator(content.begin(), content.end(), dep_regex);
            for (; it != std::sregex_iterator(); ++it) {
                std::string name = (*it)[1].str();
                if (name != "dependencies") do_update(name);
            }
        }
        std::cout << "\033[32m[OK] Update complete.\033[0m\n\n";
        return 0;
    }

    // =========================================================================
    // REMOVE subcommand: zenith remove <package>
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "remove") {
        namespace fs = std::filesystem;
        if (argc < 3) {
            std::cerr << "Usage: zenith remove <package-name>\n";
            return 1;
        }
        std::string pkg = argv[2];
        std::string pkg_path = "lib/" + pkg;
        std::cout << "\n\033[1m\033[91m[Zenith] Removing package: " << pkg << "\033[0m\n";

        bool removed_dir = false;
        if (fs::exists(pkg_path)) {
            fs::remove_all(pkg_path);
            std::cout << "  \033[32m✓\033[0m Deleted lib/" << pkg << "/\n";
            removed_dir = true;
        } else {
            std::cout << "  \033[33m⚠\033[0m lib/" << pkg << "/ not found (already removed?)\n";
        }

        // Remove from zenith.json
        std::ifstream f_in("zenith.json");
        if (f_in.is_open()) {
            std::stringstream buffer; buffer << f_in.rdbuf();
            std::string content = buffer.str(); f_in.close();

            // Remove the line with this package
            std::regex rm_regex("\\s*\"" + pkg + "\"\\s*:\\s*\"[^\"]*\",?");
            std::string updated = std::regex_replace(content, rm_regex, "");
            // Clean up trailing commas before closing brace
            std::regex trailing_comma(",\\s*\\}");
            updated = std::regex_replace(updated, trailing_comma, "\n}");

            std::ofstream f_out("zenith.json");
            f_out << updated; f_out.close();
            std::cout << "  \033[32m✓\033[0m Removed from zenith.json\n";
        }

        if (removed_dir)
            std::cout << "\033[32m[OK] Package '" << pkg << "' removed successfully.\033[0m\n\n";
        return 0;
    }

    // =========================================================================
    // PUBLISH subcommand: zenith publish
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "publish") {
        namespace fs = std::filesystem;
        std::cout << "\n\033[1m\033[93m╔═══════════════════════════════════════╗\033[0m\n";
        std::cout << "\033[1m\033[93m║   Zenith Package Publisher             ║\033[0m\n";
        std::cout << "\033[1m\033[93m╚═══════════════════════════════════════╝\033[0m\n\n";

        // Read zenith.json for package name
        std::ifstream f_in("zenith.json");
        std::string pkg_name = fs::current_path().filename().string();
        std::string git_url = "";
        if (f_in.is_open()) {
            std::stringstream buffer; buffer << f_in.rdbuf();
            std::string content = buffer.str(); f_in.close();
            std::regex name_re("\"name\"\\s*:\\s*\"([^\"]+)\"");
            std::smatch nm;
            if (std::regex_search(content, nm, name_re)) pkg_name = nm[1].str();
        }

        // Check git remote
        FILE* git_remote = popen("git remote get-url origin 2>&1", "r");
        if (git_remote) {
            char buf[512]; if (fgets(buf, sizeof(buf), git_remote)) git_url = buf;
            pclose(git_remote);
            while (!git_url.empty() && (git_url.back() == '\n' || git_url.back() == '\r')) git_url.pop_back();
        }

        std::cout << "  Package: \033[96m" << pkg_name << "\033[0m\n";
        if (!git_url.empty()) std::cout << "  Git URL: \033[2m" << git_url << "\033[0m\n";
        std::cout << "\n  \033[1mPublishing to Zenith Registry:\033[0m\n\n";
        std::cout << "  \033[2m1. Ensure your project is pushed to GitHub:\033[0m\n";
        std::cout << "     git push origin main\n\n";
        std::cout << "  \033[2m2. Users can install your package with:\033[0m\n";
        std::cout << "     \033[33mzenith install " << (git_url.empty() ? "https://github.com/<user>/" + pkg_name + ".git" : git_url) << "\033[0m\n\n";
        std::cout << "  \033[2m3. To list in the official registry, open a PR at:\033[0m\n";
        std::cout << "     https://github.com/zenith-lang/registry\n\n";
        std::cout << "  \033[1m\033[32m✓ Package is ready to publish!\033[0m\n\n";
        return 0;
    }

    // =========================================================================
    // DAEMON subcommand: zenith daemon start|stop|status
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "daemon") {
        namespace fs = std::filesystem;
        std::string action = (argc >= 3) ? std::string(argv[2]) : "status";

#ifdef _WIN32
        std::string pid_file  = std::string(getenv("TEMP") ? getenv("TEMP") : ".") + "\\zenith_daemon.pid";
        std::string log_file  = std::string(getenv("TEMP") ? getenv("TEMP") : ".") + "\\zenith_daemon.log";
#else
        std::string pid_file  = "/tmp/zenith_daemon.pid";
        std::string log_file  = "/tmp/zenith_daemon.log";
#endif

        auto read_pid = [&]() -> int {
            std::ifstream pf(pid_file);
            if (!pf.is_open()) return -1;
            int pid = -1; pf >> pid; return pid;
        };

        auto is_running = [&](int pid) -> bool {
            if (pid <= 0) return false;
#ifdef _WIN32
            // Use tasklist command to check if process is running (avoids windows.h)
            std::string cmd = "tasklist /FI \"PID eq " + std::to_string(pid) + "\" /NH 2>nul";
            FILE* tl = _popen(cmd.c_str(), "r");
            if (!tl) return false;
            char buf[256] = {};
            bool found = false;
            while (fgets(buf, sizeof(buf), tl)) {
                if (std::string(buf).find(std::to_string(pid)) != std::string::npos) { found = true; break; }
            }
            _pclose(tl);
            return found;
#else
            return (kill(pid, 0) == 0);
#endif
        };

        if (action == "start") {
            int existing_pid = read_pid();
            if (existing_pid > 0 && is_running(existing_pid)) {
                std::cout << "\033[33m[Daemon] Already running (PID " << existing_pid << ")\033[0m\n";
                return 0;
            }

            // Determine watch dir
            std::string watch_dir = ".";
            std::string watch_target = "cpp";
            for (int i = 3; i < argc; ++i) {
                std::string a = argv[i];
                if ((a == "-d" || a == "--dir") && i+1 < argc) { watch_dir = argv[++i]; }
                else if (a == "-target" && i+1 < argc) { watch_target = argv[++i]; }
            }

            std::cout << "\033[1m\033[96m[Daemon] Starting Zenith compiler daemon...\033[0m\n";
            std::cout << "  Watch dir: " << watch_dir << "\n";
            std::cout << "  Target:    " << watch_target << "\n";
            std::cout << "  PID file:  " << pid_file << "\n";
            std::cout << "  Log file:  " << log_file << "\n\n";

#ifdef _WIN32
            std::string self = argv[0];
            std::string cmd = "start /B \"Zenith Daemon\" " + self + " _daemon_worker " +
                              watch_dir + " " + watch_target + " >" + log_file + " 2>&1";
            system(cmd.c_str());
            // Write approximate PID (Windows: not trivial to get child PID via system())
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            // Try to find it via tasklist
            FILE* tl = popen("powershell -Command \"(Get-Process | Where-Object {$_.MainWindowTitle -eq 'Zenith Daemon'} | Select-Object -First 1).Id\" 2>nul", "r");
            int daemon_pid = -1;
            if (tl) {
                char buf[64]; if (fgets(buf, sizeof(buf), tl)) {
                    try { daemon_pid = std::stoi(buf); } catch (...) {}
                }
                pclose(tl);
            }
            if (daemon_pid > 0) {
                std::ofstream pf(pid_file); pf << daemon_pid;
                std::cout << "\033[32m[Daemon] Started with PID " << daemon_pid << "\033[0m\n";
            } else {
                std::cout << "\033[32m[Daemon] Started in background. Log: " << log_file << "\033[0m\n";
            }
#else
            pid_t child = fork();
            if (child == 0) {
                // Child daemon process
                setsid();
                int log_fd = open(log_file.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0644);
                if (log_fd >= 0) { dup2(log_fd, STDOUT_FILENO); dup2(log_fd, STDERR_FILENO); close(log_fd); }
                // Re-exec as daemon worker
                execl(argv[0], argv[0], "_daemon_worker", watch_dir.c_str(), watch_target.c_str(), nullptr);
                _exit(1);
            } else if (child > 0) {
                std::ofstream pf(pid_file); pf << child;
                std::cout << "\033[32m[Daemon] Started with PID " << child << "\033[0m\n";
                std::cout << "  Log: " << log_file << "\n";
            } else {
                std::cerr << "\033[31m[Daemon] Failed to fork.\033[0m\n";
                return 1;
            }
#endif
            return 0;
        }

        else if (action == "stop") {
            int pid = read_pid();
            if (pid <= 0 || !is_running(pid)) {
                std::cout << "\033[33m[Daemon] Not running.\033[0m\n";
                if (fs::exists(pid_file)) fs::remove(pid_file);
                return 0;
            }
#ifdef _WIN32
            std::string kill_cmd = "taskkill /PID " + std::to_string(pid) + " /F >nul 2>&1";
            system(kill_cmd.c_str());
#else
            kill(pid, SIGTERM);
#endif
            fs::remove(pid_file);
            std::cout << "\033[32m[Daemon] Stopped (PID " << pid << ").\033[0m\n";
            return 0;
        }

        else if (action == "status") {
            int pid = read_pid();
            bool running = (pid > 0 && is_running(pid));
            std::cout << "\n\033[1m[Zenith Daemon Status]\033[0m\n";
            if (running) {
                std::cout << "  Status:   \033[32m● Running\033[0m (PID " << pid << ")\n";
            } else {
                std::cout << "  Status:   \033[31m○ Stopped\033[0m\n";
            }
            std::cout << "  PID file: " << pid_file << "\n";
            std::cout << "  Log file: " << log_file << "\n";

            // Print last 10 lines of log
            if (fs::exists(log_file)) {
                std::ifstream lf(log_file);
                std::vector<std::string> lines;
                std::string l;
                while (std::getline(lf, l)) lines.push_back(l);
                std::cout << "\n  \033[1mRecent log:\033[0m\n";
                size_t start = lines.size() > 10 ? lines.size() - 10 : 0;
                for (size_t i = start; i < lines.size(); ++i)
                    std::cout << "    \033[2m" << lines[i] << "\033[0m\n";
            }
            std::cout << "\n";
            return 0;
        }

        else {
            std::cerr << "Usage: zenith daemon start|stop|status\n";
            return 1;
        }
    }

    // =========================================================================
    // DAEMON WORKER (internal, invoked by daemon start)
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "_daemon_worker") {
        namespace fs = std::filesystem;
        std::string watch_dir = (argc >= 3) ? argv[2] : ".";
        std::string watch_target = (argc >= 4) ? argv[3] : "cpp";

        std::cout << "[Zenith Daemon Worker] Watching: " << watch_dir
                  << " | Target: " << watch_target << "\n";
        std::cout << "[Zenith Daemon Worker] Pre-warming compiler cache...\n";
        std::cout << "[Zenith Daemon Worker] Ready.\n";

        std::unordered_map<std::string, fs::file_time_type> file_times;
        auto scan = [&](const std::string& dir) -> std::vector<std::string> {
            std::vector<std::string> changed;
            try {
                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".zen") {
                        std::string p = entry.path().string();
                        auto mt = fs::last_write_time(entry);
                        if (file_times.find(p) == file_times.end() || file_times[p] != mt) {
                            if (file_times.count(p)) changed.push_back(p);
                            file_times[p] = mt;
                        }
                    }
                }
            } catch(...) {}
            return changed;
        };

        // Initial scan
        scan(watch_dir);

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            auto changed = scan(watch_dir);
            for (const auto& changed_file : changed) {
                std::cout << "[Daemon] Change detected: " << changed_file << "\n";
                // Re-transpile changed file
                std::ifstream f(changed_file);
                if (!f.is_open()) continue;
                std::stringstream buf; buf << f.rdbuf();
                std::string code = buf.str(); f.close();

                Lexer lexer(code);
                auto tokens = lexer.tokenize();
                Parser parser(tokens);
                auto ast = parser.parseProgram();
                if (!ast) { std::cout << "[Daemon] Parse error in " << changed_file << "\n"; continue; }

                SemanticAnalyzer analyzer;
                analyzer.analyze(ast.get());
                std::cout << "[Daemon] ✓ Re-analyzed " << changed_file << "\n";
            }
        }
        return 0;
    }

    // =========================================================================
    // ADD subcommand: zenith add <npm-package>
    // Resolves jsDelivr CDN URL + generates .zen binding stub in lib/<pkg>/
    // Usage: zenith add chart.js   |   zenith add gsap   |   zenith add axios
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "add") {
        namespace fs = std::filesystem;

        if (argc < 3) {
            std::cerr << "Usage: zenith add <npm-package-name>\n";
            std::cerr << "  Example: zenith add chart.js\n";
            std::cerr << "  Example: zenith add gsap\n";
            std::cerr << "  Example: zenith add axios\n";
            return 1;
        }

        std::string pkg = argv[2];
        // Strip leading @ scope prefix for dir name if scoped: @scope/pkg -> scope_pkg
        std::string dir_name = pkg;
        for (auto& c : dir_name) if (c == '/' || c == '@') c = '_';

        std::string cdn_url  = "https://cdn.jsdelivr.net/npm/" + pkg;
        std::string pkg_dir  = "lib/" + dir_name;
        std::string zen_file = pkg_dir + "/main.zen";
        std::string json_key = dir_name;

        std::cout << "\n";
        std::cout << "  \033[1m\033[96m╔═══════════════════════════════════════════╗\033[0m\n";
        std::cout << "  \033[1m\033[96m║   Zenith Add — npm CDN Bridge             ║\033[0m\n";
        std::cout << "  \033[1m\033[96m╚═══════════════════════════════════════════╝\033[0m\n\n";
        std::cout << "  \033[2mPackage :\033[0m  " << pkg << "\n";
        std::cout << "  \033[2mCDN URL :\033[0m  " << cdn_url << "\n";
        std::cout << "  \033[2mLib dir :\033[0m  " << pkg_dir << "\n\n";

        // 1. Create lib/<pkg>/ directory
        fs::create_directories(pkg_dir);
        std::cout << "  \033[32m✓\033[0m  Created " << pkg_dir << "/\n";

        // 2. Generate the .zen binding stub file
        std::string stub =
            "// Zenith CDN binding for npm package: " + pkg + "\n"
            "// Auto-generated by: zenith add " + pkg + "\n"
            "// CDN: " + cdn_url + "\n"
            "//\n"
            "// Usage in your .zen file:\n"
            "//   import npm \"" + pkg + "\"\n"
            "//   foreign \"js\" { ... }\n"
            "//\n"
            "// Example — using in a page:\n"
            "//\n"
            "// import npm \"" + pkg + "\";\n"
            "//\n"
            "// foreign \"js\" {\n"
            "//     // Declare functions from the library here\n"
            "//     // Example (adjust to the library's actual API):\n"
            "//     Void " + dir_name + "_init(ctx: String, config: String);\n"
            "// }\n"
            "//\n"
            "// class MyPage() {\n"
            "//     UI build() {\n"
            "//         return Column(\n"
            "//             Text(\"" + pkg + " loaded from jsDelivr CDN\")\n"
            "//         );\n"
            "//     }\n"
            "// }\n\n"
            "// -- Package Metadata --\n"
            "// name: " + pkg + "\n"
            "// cdn: " + cdn_url + "\n"
            "// source: jsDelivr\n";

        std::ofstream zen_out(zen_file);
        if (zen_out.is_open()) {
            zen_out << stub;
            zen_out.close();
            std::cout << "  \033[32m✓\033[0m  Generated binding stub: " << zen_file << "\n";
        } else {
            std::cerr << "  \033[31m✗\033[0m  Failed to write " << zen_file << "\n";
        }

        // 3. Update zenith.json
        std::string json_path = "zenith.json";
        std::string json_content = "{}";
        {
            std::ifstream jf(json_path);
            if (jf.is_open()) {
                std::stringstream jbuf; jbuf << jf.rdbuf();
                json_content = jbuf.str();
            }
        }
        // Inject the new dependency into the JSON (simple string manipulation)
        std::string dep_entry = "\"" + json_key + "\": \"" + cdn_url + "\"";
        if (json_content.find(dep_entry) == std::string::npos) {
            // Find the closing } and insert before it
            auto close_pos = json_content.rfind('}');
            if (close_pos != std::string::npos) {
                // Check if there are existing entries (need comma)
                bool has_entries = (json_content.find(':') != std::string::npos);
                std::string insert_str = (has_entries ? ",\n  " : "\n  ") + dep_entry + "\n";
                json_content.insert(close_pos, insert_str);
            }
            std::ofstream jout(json_path);
            if (jout.is_open()) { jout << json_content; jout.close(); }
            std::cout << "  \033[32m✓\033[0m  Updated zenith.json\n";
        } else {
            std::cout << "  \033[33m⚠\033[0m  Already in zenith.json (no change)\n";
        }

        std::cout << "\n";
        std::cout << "  \033[1m\033[32m✓ Package '" << pkg << "' added!\033[0m\n\n";
        std::cout << "  \033[2mUse it in your .zen file:\033[0m\n\n";
        std::cout << "  \033[96mimport npm \"" << pkg << "\";\033[0m\n";
        std::cout << "  \033[96mforeign \"js\" {\033[0m\n";
        std::cout << "  \033[96m    Void myFunc(arg: String);\033[0m\n";
        std::cout << "  \033[96m}\033[0m\n\n";
        std::cout << "  \033[2mThen run:\033[0m  zenith serve pages/index.zen\n\n";
        return 0;
    }

    // =========================================================================
    // BRIDGE subcommand: zenith bridge <dart|rust> <package_name> "<function_specs>"
    // Scaffolds binary package bridges and auto-generates Zenith bindings
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "bridge") {
        namespace fs = std::filesystem;
        
        if (argc < 5) {
            std::cerr << "Usage: zenith bridge <dart|rust> <package_name> \"<exported_functions_declaration>\"\n";
            std::cerr << "  Example: zenith bridge rust uuid \"String generate_uuid();\"\n";
            std::cerr << "  Example: zenith bridge dart crypto \"String sha256(String input);\"\n";
            return 1;
        }
        
        std::string kind = argv[2];
        for (auto& c : kind) c = std::tolower(c);
        std::string package_name = argv[3];
        std::string spec_str = argv[4];
        
        if (kind != "dart" && kind != "rust") {
            std::cerr << "Error: Bridge kind must be 'dart' or 'rust'\n";
            return 1;
        }
        
        auto funcs = parseBridgeSpecs(spec_str);
        if (funcs.empty()) {
            std::cerr << "Error: Could not parse any function signatures from: " << spec_str << "\n";
            return 1;
        }
        
        std::cout << "\n";
        std::cout << "  \033[1m\033[96m╔═══════════════════════════════════════════╗\033[0m\n";
        std::cout << "  \033[1m\033[96m║   Zenith Bridge — Binary Package Linker   ║\033[0m\n";
        std::cout << "  \033[1m\033[96m╚═══════════════════════════════════════════╝\033[0m\n\n";
        std::cout << "  \033[2mKind    :\033[0m  " << kind << "\n";
        std::cout << "  \033[2mPackage :\033[0m  " << package_name << "\n";
        std::cout << "  \033[2mExports :\033[0m\n";
        for (const auto& f : funcs) {
            std::cout << "    • " << f.ret << " " << f.name << "(";
            for (size_t i = 0; i < f.args.size(); ++i) {
                std::cout << f.args[i].first << " " << f.args[i].second;
                if (i < f.args.size() - 1) std::cout << ", ";
            }
            std::cout << ")\n";
        }
        std::cout << "\n";
        
        std::string bridge_dir = "lib/" + package_name;
        fs::create_directories(bridge_dir);
        
        if (kind == "rust") {
            fs::create_directories(bridge_dir + "/src");
            
            // 1. Cargo.toml
            std::ofstream cargo_file(bridge_dir + "/Cargo.toml");
            if (cargo_file.is_open()) {
                cargo_file << "[package]\n"
                           << "name = \"rust_" << package_name << "_bridge\"\n"
                           << "version = \"0.1.0\"\n"
                           << "edition = \"2021\"\n\n"
                           << "[lib]\n"
                           << "crate-type = [\"cdylib\"]\n\n"
                           << "[dependencies]\n"
                           << package_name << " = \"*\"\n";
                cargo_file.close();
            }
            
            // 2. src/lib.rs
            std::ofstream lib_file(bridge_dir + "/src/lib.rs");
            if (lib_file.is_open()) {
                lib_file << genRustLib(funcs, package_name);
                lib_file.close();
            }
            
            // 3. main.zen
            std::ofstream zen_file(bridge_dir + "/main.zen");
            if (zen_file.is_open()) {
                zen_file << genZenithWrapper(funcs, package_name, "rust");
                zen_file.close();
            }
            
            std::cout << "  \033[32m✓\033[0m Scaffolded Rust package bridge in: \033[92m" << bridge_dir << "\033[0m\n";
            std::cout << "  \033[2mTo build, run:\033[0m\n";
            std::cout << "    cd " << bridge_dir << " && cargo build --release\n\n";
        }
        else if (kind == "dart") {
            // 1. pubspec.yaml
            std::ofstream pubspec_file(bridge_dir + "/pubspec.yaml");
            if (pubspec_file.is_open()) {
                pubspec_file << "name: dart_" << package_name << "_bridge\n"
                             << "environment:\n"
                             << "  sdk: '>=2.15.0 <3.0.0'\n\n"
                             << "dependencies:\n"
                             << "  " << package_name << ": any\n";
                pubspec_file.close();
            }
            
            // 2. main.dart
            std::ofstream main_dart_file(bridge_dir + "/main.dart");
            if (main_dart_file.is_open()) {
                main_dart_file << genDartLib(funcs, package_name);
                main_dart_file.close();
            }
            
            // 3. main.zen
            std::ofstream zen_file(bridge_dir + "/main.zen");
            if (zen_file.is_open()) {
                zen_file << genZenithWrapper(funcs, package_name, "dart");
                zen_file.close();
            }
            
            std::cout << "  \033[32m✓\033[0m Scaffolded Dart package bridge in: \033[92m" << bridge_dir << "\033[0m\n";
            std::cout << "  \033[2mTo build, run:\033[0m\n";
            std::cout << "    cd " << bridge_dir << " && dart pub get && dart compile wasm main.dart\n\n";
        }
        return 0;
    }

    // =========================================================================
    // SERVE subcommand: zenith serve <file.zen|dir/> [--port 8080] [--target web|wasm]
    // Dynamic SSR + File-System Router + Static Assets + HMR — like Next.js
    // =========================================================================
    if (argc >= 2 && std::string(argv[1]) == "serve") {
        if (argc < 3) {
            std::cerr << "Usage: zenith serve <file.zen|project-dir/> [--port 8080] [--target web|wasm]\n";
            return 1;
        }

        namespace fs = std::filesystem;

        std::string serve_arg = argv[2];
        int serve_port = 8080;
        std::string serve_target = "web";

        for (int i = 3; i < argc; ++i) {
            std::string a = argv[i];
            if ((a == "--port" || a == "-p") && i + 1 < argc) {
                try { serve_port = std::stoi(argv[++i]); } catch (...) {}
            } else if (a == "--target" && i + 1 < argc) {
                serve_target = argv[++i];
            }
        }

        if (serve_target != "web" && serve_target != "wasm") {
            std::cerr << "[Serve] Error: --target must be 'web' or 'wasm'\n";
            return 1;
        }

        // ----------------------------------------------------------------
        // Detect mode: single file OR directory (Next.js project router)
        // ----------------------------------------------------------------
        bool is_dir_mode = fs::is_directory(serve_arg);
        std::string serve_file = "";

        // Route table: URL path -> absolute .zen file path
        std::unordered_map<std::string, std::string> route_table;
        std::string pages_dir = "";
        std::string public_dir = "";

        if (is_dir_mode) {
            pages_dir  = serve_arg + "/pages";
            public_dir = serve_arg + "/public";

            // Scan pages/ and build route table
            auto build_routes = [&]() {
                route_table.clear();
                if (!fs::exists(pages_dir)) return;
                for (const auto& entry : fs::recursive_directory_iterator(pages_dir)) {
                    if (!entry.is_regular_file()) continue;
                    if (entry.path().extension() != ".zen") continue;

                    // Convert path: pages/index.zen -> /
                    //               pages/about.zen -> /about
                    //               pages/blog/post.zen -> /blog/post
                    std::string rel = fs::relative(entry.path(), pages_dir).string();
                    // Normalise slashes
                    for (auto& c : rel) if (c == '\\') c = '/';
                    // Strip .zen extension
                    if (rel.size() > 4) rel = rel.substr(0, rel.size() - 4);
                    // index -> /
                    std::string route = "/" + rel;
                    if (rel == "index") route = "/";
                    // Strip trailing /index
                    if (route.size() > 7 && route.substr(route.size() - 6) == "/index")
                        route = route.substr(0, route.size() - 6);

                    route_table[route] = entry.path().string();
                }
            };
            build_routes();

            if (route_table.empty()) {
                std::cerr << "[Serve] No .zen files found in " << pages_dir << "\n";
                std::cerr << "[Serve] Create pages/index.zen to get started.\n";
                return 1;
            }
        } else {
            // Single file mode (original behaviour)
            serve_file = serve_arg;
            route_table["/"] = serve_file;
        }

        // ----------------------------------------------------------------
        // MIME type table for static asset serving
        // ----------------------------------------------------------------
        auto get_mime = [](const std::string& ext) -> std::string {
            if (ext == ".html" || ext == ".htm") return "text/html";
            if (ext == ".css")  return "text/css";
            if (ext == ".js")   return "application/javascript";
            if (ext == ".json") return "application/json";
            if (ext == ".png")  return "image/png";
            if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
            if (ext == ".gif")  return "image/gif";
            if (ext == ".svg")  return "image/svg+xml";
            if (ext == ".ico")  return "image/x-icon";
            if (ext == ".wasm") return "application/wasm";
            if (ext == ".ttf")  return "font/ttf";
            if (ext == ".woff" || ext == ".woff2") return "font/woff2";
            return "application/octet-stream";
        };

        // ----------------------------------------------------------------
        // 404 page generator — lists all available routes
        // ----------------------------------------------------------------
        auto make_404 = [&](const std::string& path) -> std::string {
            std::string links;
            for (const auto& [r, _] : route_table) {
                links += "<li><a href=\"" + r + "\">" + r + "</a></li>\n";
            }
            return R"html(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><title>404 — Zenith SSR</title>
<style>
body{margin:0;background:#0b0f19;color:#e2e8f0;font-family:'Segoe UI',sans-serif;
display:flex;align-items:center;justify-content:center;min-height:100vh;}
.box{text-align:center;padding:40px;border:1px solid rgba(255,255,255,0.08);
border-radius:16px;background:rgba(255,255,255,0.03);}
h1{font-size:5rem;margin:0;background:linear-gradient(135deg,#a5b4fc,#c084fc);
-webkit-background-clip:text;-webkit-text-fill-color:transparent;}
p{color:#94a3b8;margin:8px 0 24px;} ul{text-align:left;}
a{color:#818cf8;text-decoration:none;} a:hover{text-decoration:underline;}
</style></head><body>
<div class="box">
<h1>404</h1>
<p>Page <code>)html" + path + R"html(</code> not found in Zenith router.</p>
<p style="font-size:0.9rem;">Available routes:</p>
<ul>)html" + links + R"html(</ul>
</div></body></html>
)html";
        };

        // ----------------------------------------------------------------
        // Helper: transpile the zen file and return HTML string
        // ----------------------------------------------------------------
        auto transpile_to_html = [&](const std::string& zen_path, const std::string& tgt) -> std::string {
            std::ifstream f(zen_path);
            if (!f.is_open()) return "<html><body><h1>Error: Cannot open " + zen_path + "</h1></body></html>";
            std::stringstream buf; buf << f.rdbuf();
            std::string source = buf.str(); f.close();

            Lexer lexer(source);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto ast = parser.parseProgram();
            if (!ast) return "<html><body><h1>Parse error in " + zen_path + "</h1></body></html>";

            SemanticAnalyzer analyzer;
            analyzer.analyze(ast.get());

            if (tgt == "wasm") {
                WasmCodeGenerator wasm_gen;
                return wasm_gen.generate(ast.get());
            } else {
                JsCodeGenerator js_gen;
                return js_gen.generate(ast.get());
            }
        };

        // ----------------------------------------------------------------
        // Helper: parse HTTP request line to extract method + path
        // ----------------------------------------------------------------
        auto parse_request_line = [](const std::string& req, std::string& method, std::string& path) {
            method = "GET"; path = "/";
            size_t line_end = req.find("\r\n");
            if (line_end == std::string::npos) line_end = req.find("\n");
            if (line_end == std::string::npos) return;
            std::string first_line = req.substr(0, line_end);
            size_t sp1 = first_line.find(' ');
            if (sp1 == std::string::npos) return;
            method = first_line.substr(0, sp1);
            size_t sp2 = first_line.find(' ', sp1 + 1);
            if (sp2 == std::string::npos) sp2 = first_line.size();
            path = first_line.substr(sp1 + 1, sp2 - sp1 - 1);
        };

        // ----------------------------------------------------------------
        // Helper: build a complete HTTP/1.1 response string
        // ----------------------------------------------------------------
        auto make_http_response = [](int status, const std::string& status_text,
                                     const std::string& content_type, const std::string& body) -> std::string {
            std::string resp;
            resp += "HTTP/1.1 " + std::to_string(status) + " " + status_text + "\r\n";
            resp += "Content-Type: " + content_type + "; charset=utf-8\r\n";
            resp += "Content-Length: " + std::to_string(body.size()) + "\r\n";
            resp += "Connection: close\r\n";
            resp += "Cache-Control: no-store\r\n";
            resp += "X-Powered-By: Zenith-SSR/1.0\r\n";
            resp += "\r\n";
            resp += body;
            return resp;
        };

        // ----------------------------------------------------------------
        // HMR: atomic file-change generation + background watcher thread
        // ----------------------------------------------------------------
        std::atomic<uint64_t> hmr_generation{0};
        fs::file_time_type hmr_last_mtime{};
        try {
            std::string watch_target_path = is_dir_mode ? pages_dir : serve_file;
            hmr_last_mtime = fs::last_write_time(watch_target_path);
        } catch (...) {}

        // Background thread: poll all .zen files mtime every 300ms
        std::thread hmr_watcher([&]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                try {
                    if (is_dir_mode) {
                        // Watch entire pages/ directory modification time
                        auto mtime = fs::last_write_time(pages_dir);
                        if (mtime != hmr_last_mtime) {
                            hmr_last_mtime = mtime;
                            ++hmr_generation;
                            std::cout << "  \033[93m\u21bb\033[0m  HMR: pages/ changed, reloading browsers...\n";
                        }
                        // Also scan individual files in case dir mtime didn't change
                        for (const auto& entry : fs::recursive_directory_iterator(pages_dir)) {
                            if (!entry.is_regular_file() || entry.path().extension() != ".zen") continue;
                            auto fmtime = fs::last_write_time(entry);
                            if (fmtime != hmr_last_mtime) {
                                hmr_last_mtime = fmtime;
                                ++hmr_generation;
                                std::cout << "  \033[93m\u21bb\033[0m  HMR: " << entry.path().filename().string() << " changed, reloading browsers...\n";
                                break;
                            }
                        }
                    } else {
                        auto mtime = fs::last_write_time(serve_file);
                        if (mtime != hmr_last_mtime) {
                            hmr_last_mtime = mtime;
                            ++hmr_generation;
                            std::cout << "  \033[93m\u21bb\033[0m  HMR: change detected, reloading browsers...\n";
                        }
                    }
                } catch (...) {}
            }
        });
        hmr_watcher.detach();

        // ----------------------------------------------------------------
        // HMR: tiny JS snippet injected into every served page
        // ----------------------------------------------------------------
        std::string hmr_script = R"rawhmr(
<script>
// Zenith HMR Client — auto-injected by `zenith serve`
(function() {
    var port = )rawhmr" + std::to_string(serve_port) + R"rawhmr(;
    function connect() {
        var es = new EventSource('http://localhost:' + port + '/__zenith_hmr');
        es.onmessage = function(e) {
            if (e.data === 'reload') {
                console.log('[Zenith HMR] Reloading...');
                location.reload();
            }
        };
        es.onerror = function() {
            es.close();
            setTimeout(connect, 2000); // reconnect on server restart
        };
    }
    connect();
    console.log('[Zenith HMR] Hot reload active on port )rawhmr" + std::to_string(serve_port) + R"rawhmr(');
})();
</script>
)rawhmr";

        // ----------------------------------------------------------------
        // Helper: inject HMR script before </body>
        // ----------------------------------------------------------------
        auto inject_hmr = [&](std::string html) -> std::string {
            auto pos = html.rfind("</body>");
            if (pos != std::string::npos) {
                html.insert(pos, hmr_script);
            } else {
                html += hmr_script;
            }
            return html;
        };

        // Print banner
        std::cout << "\n";
        std::cout << "  \033[1m\033[96m\u2554\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557\033[0m\n";
        std::cout << "  \033[1m\033[96m\u2551   Zenith SSR Web Server  \u26a1  v1.0          \u2551\033[0m\n";
        std::cout << "  \033[1m\033[96m\u255a\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d\033[0m\n\n";
        if (is_dir_mode) {
            std::cout << "  \033[2mMode   :\033[0m  \033[1mDirectory Router\033[0m (Next.js-style)\n";
            std::cout << "  \033[2mPages  :\033[0m  " << pages_dir << "\n";
            std::cout << "  \033[2mPublic :\033[0m  " << public_dir << "\n";
            std::cout << "  \033[2mRoutes :\033[0m\n";
            for (const auto& [r, f] : route_table) {
                std::cout << "           \033[32m" << r << "\033[0m  \033[2m→  " << f << "\033[0m\n";
            }
        } else {
            std::cout << "  \033[2mSource :\033[0m  " << serve_file << "\n";
            std::cout << "  \033[2mMode   :\033[0m  Single File SSR\n";
        }
        std::cout << "  \033[2mTarget :\033[0m  " << serve_target << "\n";
        std::cout << "  \033[2mPort   :\033[0m  " << serve_port << "\n";
        std::cout << "  \033[2mHMR    :\033[0m  http://localhost:" << serve_port << "/__zenith_hmr\n\n";

#ifdef _WIN32
        // ----------------------------------------------------------------
        // Windows: WinSock2 TCP server
        // ----------------------------------------------------------------
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            std::cerr << "[Serve] Error: WSAStartup failed.\n";
            return 1;
        }

        SOCKET server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (server_sock == INVALID_SOCKET) {
            std::cerr << "[Serve] Error: socket() failed: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }

        // Allow port reuse
        int opt_val = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt_val, sizeof(opt_val));

        sockaddr_in server_addr{};
        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port        = htons((u_short)serve_port);

        if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "[Serve] Error: bind() failed on port " << serve_port << ": " << WSAGetLastError() << "\n";
            closesocket(server_sock);
            WSACleanup();
            return 1;
        }

        listen(server_sock, SOMAXCONN);

        std::cout << "  \033[32m✓ Server listening on\033[0m  \033[1mhttp://localhost:" << serve_port << "\033[0m\n\n";
        std::cout << "  Press \033[1mCtrl+C\033[0m to stop.\n\n";
        std::cout << "  \033[2m──────────────────────────────────────────────\033[0m\n";
        std::cout << "  \033[2m  Request Log\033[0m\n";
        std::cout << "  \033[2m──────────────────────────────────────────────\033[0m\n\n";

        while (true) {
            sockaddr_in client_addr{};
            int client_len = sizeof(client_addr);
            SOCKET client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);
            if (client_sock == INVALID_SOCKET) continue;

            // Read the HTTP request
            std::string request_data;
            char recv_buf[4096];
            int bytes_recv = recv(client_sock, recv_buf, sizeof(recv_buf) - 1, 0);
            if (bytes_recv > 0) {
                recv_buf[bytes_recv] = '\0';
                request_data = recv_buf;
            }

            std::string method, req_path;
            parse_request_line(request_data, method, req_path);

            // Strip query string from path
            auto qpos = req_path.find('?');
            if (qpos != std::string::npos) req_path = req_path.substr(0, qpos);

            std::string response_str;

            if (req_path == "/__zenith_hmr") {
                // SSE endpoint: long-poll until generation changes
                uint64_t client_gen = hmr_generation.load();
                std::string sse_header;
                sse_header += "HTTP/1.1 200 OK\r\n";
                sse_header += "Content-Type: text/event-stream\r\n";
                sse_header += "Cache-Control: no-cache\r\n";
                sse_header += "Connection: keep-alive\r\n";
                sse_header += "Access-Control-Allow-Origin: *\r\n";
                sse_header += "X-Powered-By: Zenith-HMR/1.0\r\n";
                sse_header += "\r\n";
                send(client_sock, sse_header.c_str(), (int)sse_header.size(), 0);
                for (int w = 0; w < 300; ++w) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (hmr_generation.load() != client_gen) {
                        std::string evt = "data: reload\n\n";
                        send(client_sock, evt.c_str(), (int)evt.size(), 0);
                        break;
                    }
                }
                closesocket(client_sock);
                continue;
            }

            // Static file from public/
            if (!public_dir.empty() && fs::exists(public_dir)) {
                std::string static_path = public_dir + req_path;
                if (fs::is_regular_file(static_path)) {
                    std::ifstream sf(static_path, std::ios::binary);
                    std::string body((std::istreambuf_iterator<char>(sf)), {});
                    std::string ext = fs::path(static_path).extension().string();
                    std::string mime = get_mime(ext);
                    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: " + mime + "\r\n";
                    resp += "Content-Length: " + std::to_string(body.size()) + "\r\n";
                    resp += "Cache-Control: public, max-age=3600\r\nConnection: close\r\n\r\n";
                    resp += body;
                    send(client_sock, resp.c_str(), (int)resp.size(), 0);
                    closesocket(client_sock);
                    std::cout << "  \033[34m◆\033[0m  " << method << "  " << req_path << "  \033[2m[static]\033[0m\n";
                    continue;
                }
            }

            // Route lookup
            auto route_it = route_table.find(req_path);
            if (route_it == route_table.end()) {
                std::string body404 = make_404(req_path);
                response_str = make_http_response(404, "Not Found", "text/html", body404);
                std::cout << "  \033[31m✗\033[0m  " << method << "  " << req_path << "  \033[2m[404]\033[0m\n";
            } else {
                auto t_start = std::chrono::steady_clock::now();
                std::string html = inject_hmr(transpile_to_html(route_it->second, serve_target));
                auto t_end = std::chrono::steady_clock::now();
                double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
                response_str = make_http_response(200, "OK", "text/html", html);
                std::cout << "  \033[32m●\033[0m  " << method << "  " << req_path
                          << "  \033[2m[" << std::fixed << std::setprecision(1) << ms << "ms]\033[0m\n";
            }

            send(client_sock, response_str.c_str(), (int)response_str.size(), 0);
            closesocket(client_sock);
        }

        closesocket(server_sock);
        WSACleanup();

#else
        // ----------------------------------------------------------------
        // POSIX: BSD sockets TCP server
        // ----------------------------------------------------------------
        int server_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (server_sock < 0) {
            std::cerr << "[Serve] Error: socket() failed.\n";
            return 1;
        }

        int opt_val = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

        sockaddr_in server_addr{};
        server_addr.sin_family      = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port        = htons(serve_port);

        if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "[Serve] Error: bind() failed on port " << serve_port << ".\n";
            close(server_sock);
            return 1;
        }

        listen(server_sock, SOMAXCONN);

        std::cout << "  \033[32m✓ Server listening on\033[0m  \033[1mhttp://localhost:" << serve_port << "\033[0m\n\n";
        std::cout << "  Press \033[1mCtrl+C\033[0m to stop.\n\n";
        std::cout << "  \033[2m──────────────────────────────────────────────\033[0m\n";
        std::cout << "  \033[2m  Request Log\033[0m\n";
        std::cout << "  \033[2m──────────────────────────────────────────────\033[0m\n\n";

        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);
            if (client_sock < 0) continue;

            std::string request_data;
            char recv_buf[4096];
            ssize_t bytes_recv = recv(client_sock, recv_buf, sizeof(recv_buf) - 1, 0);
            if (bytes_recv > 0) {
                recv_buf[bytes_recv] = '\0';
                request_data = recv_buf;
            }

            std::string method, req_path;
            parse_request_line(request_data, method, req_path);

            std::string response_str;
            if (req_path == "/favicon.ico") {
                response_str = make_http_response(204, "No Content", "text/plain", "");
            } else if (req_path == "/__zenith_hmr") {
                // SSE endpoint: long-poll until generation changes, then send reload
                uint64_t client_gen = hmr_generation.load();
                std::string sse_header;
                sse_header += "HTTP/1.1 200 OK\r\n";
                sse_header += "Content-Type: text/event-stream\r\n";
                sse_header += "Cache-Control: no-cache\r\n";
                sse_header += "Connection: keep-alive\r\n";
                sse_header += "Access-Control-Allow-Origin: *\r\n";
                sse_header += "X-Powered-By: Zenith-HMR/1.0\r\n";
                sse_header += "\r\n";
                send(client_sock, sse_header.c_str(), sse_header.size(), 0);
                for (int w = 0; w < 300; ++w) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (hmr_generation.load() != client_gen) {
                        std::string evt = "data: reload\n\n";
                        send(client_sock, evt.c_str(), evt.size(), 0);
                        break;
                    }
                }
                close(client_sock);
                continue;
            } else {
                auto t_start = std::chrono::steady_clock::now();
                std::string html = inject_hmr(transpile_to_html(serve_file, serve_target));
                auto t_end = std::chrono::steady_clock::now();
                double ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();

                response_str = make_http_response(200, "OK", "text/html", html);
                std::cout << "  \033[32m●\033[0m  " << method << "  " << req_path
                          << "  \033[2m[" << std::fixed << std::setprecision(1) << ms << "ms]\033[0m\n";
            }

            send(client_sock, response_str.c_str(), response_str.size(), 0);
            close(client_sock);
        }

        close(server_sock);
#endif
        return 0;
    }

    // Watch subcommand
    if (argc >= 2 && std::string(argv[1]) == "watch") {
        if (argc < 3) {
            std::cerr << "Usage: zenith watch <file.zen> [-target <cpp|web|wasm>]\n";
            return 1;
        }
        std::string watch_file = argv[2];
        std::string watch_target = "cpp";
        for (int i = 3; i < argc; ++i) {
            if (std::string(argv[i]) == "-target" && i + 1 < argc) {
                watch_target = argv[i + 1];
                i++;
            }
        }

        namespace fs = std::filesystem;
        std::string watch_dir = fs::path(watch_file).parent_path().string();
        if (watch_dir.empty()) watch_dir = ".";

        std::unordered_map<std::string, fs::file_time_type> file_times;
        
        auto scan_files = [&](const std::string& dir, std::unordered_map<std::string, fs::file_time_type>& times) -> bool {
            bool changed = false;
            std::vector<std::string> current_files;
            
            try {
                for (const auto& entry : fs::recursive_directory_iterator(dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".zen") {
                        std::string path_str = entry.path().string();
                        current_files.push_back(path_str);
                        
                        auto mtime = fs::last_write_time(entry);
                        if (times.find(path_str) == times.end()) {
                            times[path_str] = mtime;
                            changed = true;
                        } else if (times[path_str] != mtime) {
                            times[path_str] = mtime;
                            changed = true;
                        }
                    }
                }
            } catch (...) {}
            
            for (auto it = times.begin(); it != times.end(); ) {
                if (std::find(current_files.begin(), current_files.end(), it->first) == current_files.end()) {
                    it = times.erase(it);
                    changed = true;
                } else {
                    ++it;
                }
            }
            return changed;
        };

        std::cout << "[Watcher] Monitoring files in " << watch_dir << " for modifications. Target: " << watch_target << "\n";
        
        // Initial scan
        scan_files(watch_dir, file_times);

        // Run compile once first
        std::cout << "[Watcher] Initial compilation...\n";
        if (compileProject(watch_file, watch_target)) {
            if (watch_target == "cpp") {
                std::string out_cpp = watch_file.substr(0, watch_file.length() - 4) + ".cpp";
                std::string out_exe = watch_file.substr(0, watch_file.length() - 4) + (fs::path(watch_file).extension() == "" ? "" : ".exe");
                
#ifdef _WIN32
                std::string compile_cmd = "g++ -O3 -std=c++17 " + out_cpp + " -I include -o " + out_exe + " -lwinhttp";
#else
                std::string compile_cmd = "g++ -O3 -std=c++17 " + out_cpp + " -I include -o " + out_exe + " -lpthread";
#endif
                std::cout << "[Watcher] Building executable: " << compile_cmd << "\n";
                int build_res = system(compile_cmd.c_str());
                if (build_res == 0) {
                    std::cout << "[Watcher] Starting application...\n";
#ifdef _WIN32
                    std::string run_cmd = "start /B " + out_exe;
#else
                    std::string run_cmd = "./" + out_exe + " &";
#endif
                    system(run_cmd.c_str());
                }
            }
        }

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (scan_files(watch_dir, file_times)) {
#ifdef _WIN32
                system("cls");
#else
                system("clear");
#endif
                std::cout << "[Watcher] Changes detected! Recompiling...\n";
                if (compileProject(watch_file, watch_target)) {
                    if (watch_target == "cpp") {
                        std::string out_cpp = watch_file.substr(0, watch_file.length() - 4) + ".cpp";
                        std::string out_exe = watch_file.substr(0, watch_file.length() - 4) + (fs::path(watch_file).extension() == "" ? "" : ".exe");
                        
#ifdef _WIN32
                        std::string compile_cmd = "g++ -O3 -std=c++17 " + out_cpp + " -I include -o " + out_exe + " -lwinhttp";
                        std::string kill_cmd = "taskkill /F /IM " + fs::path(out_exe).filename().string() + " >nul 2>&1";
#else
                        std::string compile_cmd = "g++ -O3 -std=c++17 " + out_cpp + " -I include -o " + out_exe + " -lpthread";
                        std::string kill_cmd = "killall " + fs::path(out_exe).filename().string() + " >/dev/null 2>&1";
#endif
                        std::cout << "[Watcher] Rebuilding executable: " << compile_cmd << "\n";
                        int build_res = system(compile_cmd.c_str());
                        if (build_res == 0) {
                            std::cout << "[Watcher] Terminating running instance...\n";
                            system(kill_cmd.c_str());
                            std::cout << "[Watcher] Restarting application...\n";
#ifdef _WIN32
                            std::string run_cmd = "start /B " + out_exe;
#else
                            std::string run_cmd = "./" + out_exe + " &";
#endif
                            system(run_cmd.c_str());
                        }
                    } else {
                        std::cout << "[Watcher] Target assets updated.\n";
                    }
                }
            }
        }
        return 0;
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
        std::cerr << "\n\033[1m\033[96mZenith Compiler v0.2.0\033[0m\n\n";
        std::cerr << "\033[1mUSAGE:\033[0m\n";
        std::cerr << "  zenith <file.zen> [-target cpp|web|wasm] [-o <output>]  Transpile a Zenith file\n\n";
        std::cerr << "\033[1mPROJECT:\033[0m\n";
        std::cerr << "  zenith create <name|.>                Create a new Zenith project\n";
        std::cerr << "  zenith run <desktop|web|wasm|android|ios>  Run the project\n";
        std::cerr << "  zenith format [-w] <file.zen>         Format a Zenith source file\n\n";
        std::cerr << "\033[1mSSR WEB SERVER:\033[0m\n";
        std::cerr << "  zenith serve <file.zen> [--port 8080] [--target web|wasm]\n";
        std::cerr << "               Start a dynamic SSR HTTP server — recompiles .zen on every\n";
        std::cerr << "               request and streams fresh HTML, just like Next.js on Vercel.\n\n";
        std::cerr << "\033[1mPACKAGE MANAGER:\033[0m\n";
        std::cerr << "  zenith install <url>                  Install package from git URL\n";
        std::cerr << "  zenith install                        Install all from zenith.json\n";
        std::cerr << "  zenith list                           List installed packages\n";
        std::cerr << "  zenith search [query]                 Search package registry\n";
        std::cerr << "  zenith update [package]               Update one or all packages\n";
        std::cerr << "  zenith remove <package>               Uninstall a package\n";
        std::cerr << "  zenith publish                        Publish package instructions\n";
        std::cerr << "  zenith bridge <dart|rust> <pkg> \"<specs>\" Scaffold binary FFI packages\n\n";
        std::cerr << "\033[1mDEVELOPER TOOLS:\033[0m\n";
        std::cerr << "  zenith lsp                            Start LSP server (stdio)\n";
        std::cerr << "  zenith daemon start [-d dir]          Start compiler daemon\n";
        std::cerr << "  zenith daemon stop                    Stop compiler daemon\n";
        std::cerr << "  zenith daemon status                  Show daemon status + log\n";
        std::cerr << "  zenith watch <file.zen> [-target ..]  Hot-reload watch mode\n\n";

        return 1;
    }

    if (!compileProject(filename, target, out_filename_override)) {
        return 1;
    }

    return 0;
}
