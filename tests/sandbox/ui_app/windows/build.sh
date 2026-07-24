#!/bin/bash
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
