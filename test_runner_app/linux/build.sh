#!/bin/bash
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
