#!/bin/bash
GREEN='\033[0;92m'; RED='\033[0;91m'; BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Game Runner                |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM_FLAGS="-lpthread -framework CoreFoundation"
    echo -e "  ${GREEN}[OK]${RESET}    Platform: macOS"
else
    PLATFORM_FLAGS="-lpthread"
    echo -e "  ${GREEN}[OK]${RESET}    Platform: Linux"
fi

if ! command -v g++ &>/dev/null && [[ "$OSTYPE" != "darwin"* ]]; then
    echo -e "  ${RED}[ERROR]${RESET} g++ not found."
    exit 1
fi
if [[ "$OSTYPE" == "darwin"* ]] && ! command -v clang++ &>/dev/null; then
    echo -e "  ${RED}[ERROR]${RESET} clang++ not found."
    exit 1
fi

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main_game.cpp ..."
$COMPILER ../lib/main.zen -target cpp -o main_game.cpp
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Transpile failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    C++ source ready"

echo -e "  ${BOLD}[2/3]${RESET}   Compiling C++17 -> desktop/main_game ..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    clang++ -O3 -std=c++17 main_game.cpp -I ../include -o main_game $PLATFORM_FLAGS
else
    g++ -O3 -std=c++17 main_game.cpp -I ../include -o main_game $PLATFORM_FLAGS
fi
if [ $? -ne 0 ]; then echo -e "  ${RED}[ERROR]${RESET} Compilation failed." && exit 1; fi
echo -e "  ${GREEN}[OK]${RESET}    Build complete: desktop/main_game"

echo -e "  ${BOLD}[3/3]${RESET}   Launching Zenith Game ..."
./main_game
