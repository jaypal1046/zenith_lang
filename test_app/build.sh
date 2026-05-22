#!/bin/bash
echo "==================================================="
echo "  Zenith App Builder: Desktop and Web compiler"
echo "==================================================="
echo

# Check if compiler is available
if command -v zenith &> /dev/null; then
    COMPILER="zenith"
elif [ -f "../zenith" ]; then
    COMPILER="../zenith"
elif [ -f "./zenith" ]; then
    COMPILER="./zenith"
else
    echo "[ERROR] 'zenith' compiler executable not found in PATH or local directory."
    exit 1
fi

echo "Using compiler: $COMPILER"

echo "1. Transpiling main.zen to C++ target..."
$COMPILER main.zen -target cpp
if [ $? -ne 0 ]; then exit 1; fi

echo "2. Transpiling main.zen to Web Target..."
$COMPILER main.zen -target web
if [ $? -ne 0 ]; then exit 1; fi

echo "3. Transpiling main.zen to WASM Target..."
$COMPILER main.zen -target wasm
if [ $? -ne 0 ]; then exit 1; fi

echo
echo "4. Compiling C++ code into Desktop binary..."
g++ -O3 -std=c++17 main.cpp -I include -o main_app
if [ $? -eq 0 ]; then
    echo "[OK] Native application built: ./main_app"
else
    echo "[WARNING] g++ compiler failed."
fi

echo
echo "==================================================="
echo "  Zenith Build Succeeded!"
echo "==================================================="
echo "To run Desktop: ./main_app"
echo "To run Web: Launch server on directory and open main.html or main_wasm.html"
echo
