#!/bin/bash
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
