#!/bin/bash
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
