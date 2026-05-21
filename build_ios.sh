#!/bin/bash
echo "==================================================="
echo "   Zenith iOS Native C++ Cross-Compiler Builder"
echo "==================================================="
echo

# 1. Check for macOS (since iOS compilation requires Apple SDKs)
if [[ "$OSTYPE" != "darwin"* ]]; then
    echo "[WARNING] iOS cross-compilation typically requires macOS with Xcode installed."
    echo "Attempting to find xcrun/clang++ anyway..."
fi

# 2. Check for xcrun command
if ! command -v xcrun &> /dev/null; then
    echo "[ERROR] 'xcrun' could not be found. Please install Xcode Command Line Tools."
    exit 1
fi
echo "[OK] Found Xcode build tools."

# 3. Detect iOS SDK
SDK_PATH=$(xcrun --sdk iphoneos --show-sdk-path 2>/dev/null)
if [ -z "$SDK_PATH" ]; then
    echo "[ERROR] iOS SDK not found. Please install Xcode and the iOS SDK."
    exit 1
fi
echo "[OK] Found iOS SDK: $SDK_PATH"

# 4. Transpile Zenith source code to C++
echo
echo "Transpiling tests/main.zen to C++..."
# Assuming zenith is compiled/runnable on the host machine
if [ -f "./zenith" ]; then
    ./zenith tests/main.zen
else
    echo "[WARNING] './zenith' compiler binary not found locally."
    echo "If you are cross-compiling, make sure you ran: g++ -O3 -std=c++17 src/main.cpp ... -o zenith"
    echo "Attempting to run build..."
fi

# 5. Compile C++ to iOS Native arm64 Binary
echo
echo "Cross-compiling transpiled C++ code for iOS arm64..."
xcrun -sdk iphoneos clang++ -arch arm64 -O3 -std=c++17 tests/main.cpp -I include -isysroot "$SDK_PATH" -o tests/zenith_app_ios

if [ $? -ne 0 ]; then
    echo "[ERROR] iOS Cross-compilation failed."
    exit 1
fi
echo "[OK] Native iOS binary built successfully: tests/zenith_app_ios"
echo
echo "To deploy to a physical iOS device:"
echo "1. Sign the binary using 'codesign -s \"Developer ID\" tests/zenith_app_ios'"
echo "2. Install/run via Xcode or fruitstrap/ios-deploy."
