#!/bin/bash
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
