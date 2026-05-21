#!/bin/bash
# =====================================================
#   ZENITH RUN ANDROID — Flutter-style build runner
# =====================================================

# ANSI colors
CYAN='\033[0;96m'; GREEN='\033[0;92m'; YELLOW='\033[0;93m'
RED='\033[0;91m';  BOLD='\033[1m';     DIM='\033[2m';  RESET='\033[0m'

echo ""
echo -e "${BOLD}${CYAN}  ╔══════════════════════════════════════════════════╗${RESET}"
echo -e "${BOLD}${CYAN}  ║          Zenith  •  Android Runner               ║${RESET}"
echo -e "${BOLD}${CYAN}  ╚══════════════════════════════════════════════════╝${RESET}"
echo ""

# ── Locate Zenith compiler ──────────────────────────────
if   [ -f "../zenith" ];   then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "${DIM}  Using Zenith compiler: $COMPILER${RESET}"
echo ""

# ── Step 1: Detect Android SDK / NDK ───────────────────
echo -e "${BOLD}  •  Checking Android NDK...${RESET}"
if [ -n "$ANDROID_NDK_HOME" ]; then
    NDK_DIR="$ANDROID_NDK_HOME"
elif [ -n "$ANDROID_HOME" ] && [ -d "$ANDROID_HOME/ndk" ]; then
    NDK_DIR=$(ls -d "$ANDROID_HOME/ndk/"* 2>/dev/null | sort -V | tail -n 1)
else
    if [[ "$OSTYPE" == "darwin"* ]]; then
        NDK_BASE="$HOME/Library/Android/sdk/ndk"
        PREBUILT_HOST="darwin-x86_64"
    else
        NDK_BASE="$HOME/Android/Sdk/ndk"
        PREBUILT_HOST="linux-x86_64"
    fi
    NDK_DIR=$(ls -d "$NDK_BASE/"* 2>/dev/null | sort -V | tail -n 1)
fi

if [ -z "$NDK_DIR" ]; then
    echo -e "${RED}  ✗  Android NDK not found.${RESET}"
    echo -e "${YELLOW}     Set ANDROID_NDK_HOME or ANDROID_HOME and retry.${RESET}"
    exit 1
fi
echo -e "${GREEN}  ✓  NDK  ${DIM}$NDK_DIR${RESET}"

# ── Step 2: Detect connected device / emulator ─────────
echo -e "${BOLD}  •  Locating connected devices...${RESET}"
ABI=$(adb shell getprop ro.product.cpu.abi 2>/dev/null | tr -d '\r\n')
if [ -z "$ABI" ]; then
    echo -e "${RED}  ✗  No active Android device or emulator found.${RESET}"
    echo -e "${YELLOW}     Start an emulator or plug in a device, then retry.${RESET}"
    exit 1
fi
DEVICE_MODEL=$(adb shell getprop ro.product.model 2>/dev/null | tr -d '\r\n')
DEVICE_ID=$(adb devices | grep -v "List" | grep "device" | awk '{print $1}' | head -n 1)
echo -e "${GREEN}  ✓  Device  ${DIM}$DEVICE_MODEL ($DEVICE_ID) — ABI: $ABI${RESET}"

# ── Step 3: Select NDK cross-compiler ──────────────────
echo -e "${BOLD}  •  Selecting NDK toolchain...${RESET}"
case "$ABI" in
    "x86")        COMPILER_NAME="i686-linux-android30-clang++" ;;
    "x86_64")     COMPILER_NAME="x86_64-linux-android30-clang++" ;;
    "arm64-v8a")  COMPILER_NAME="aarch64-linux-android30-clang++" ;;
    "armeabi-v7a") COMPILER_NAME="armv7a-linux-androideabi30-clang++" ;;
    *)
        echo -e "${RED}  ✗  Unsupported ABI: $ABI${RESET}"
        exit 1 ;;
esac

NDK_CLANG="$NDK_DIR/toolchains/llvm/prebuilt/$PREBUILT_HOST/bin/$COMPILER_NAME"
if [ ! -f "$NDK_CLANG" ]; then
    echo -e "${RED}  ✗  Clang not found: $NDK_CLANG${RESET}"
    exit 1
fi
echo -e "${GREEN}  ✓  Toolchain  ${DIM}$COMPILER_NAME${RESET}"

# ── Step 4: Transpile Zenith → C++ ─────────────────────
echo ""
echo -e "${BOLD}  1/4  Transpiling  ${DIM}lib/main.zen → android/main.cpp${RESET}"
$COMPILER ../lib/main.zen -target cpp -o main.cpp
if [ $? -ne 0 ]; then
    echo -e "${RED}  ✗  Transpile failed.${RESET}"
    exit 1
fi
echo -e "${GREEN}  ✓  C++ source ready${RESET}"

# ── Step 5: Cross-compile C++ → Android binary ─────────
echo -e "${BOLD}  2/4  Compiling   ${DIM}C++17 → Android $ABI binary${RESET}"
"$NDK_CLANG" -O3 -std=c++17 main.cpp -I ../include -static-libstdc++ -llog -o main_app_android
if [ $? -ne 0 ]; then
    echo -e "${RED}  ✗  Compilation failed.${RESET}"
    exit 1
fi
echo -e "${GREEN}  ✓  Build complete${RESET}"

# ── Step 6: Push binary to device ──────────────────────
echo -e "${BOLD}  3/4  Installing  ${DIM}→ /data/local/tmp/zenith_app${RESET}"
adb push main_app_android /data/local/tmp/zenith_app >/dev/null 2>&1
adb shell chmod +x /data/local/tmp/zenith_app
if [ $? -ne 0 ]; then
    echo -e "${RED}  ✗  Install failed.${RESET}"
    exit 1
fi
echo -e "${GREEN}  ✓  Installed on device${RESET}"

# ── Step 7: Run app ─────────────────────────────────────
echo -e "${BOLD}  4/4  Running    ${DIM}Zenith App on $DEVICE_MODEL${RESET}"
echo ""
echo -e "${YELLOW}  ┌────────────────────────────────────────────────────┐${RESET}"
echo -e "${YELLOW}  │  Zenith App Output (stdout + Logcat)               │${RESET}"
echo -e "${YELLOW}  └────────────────────────────────────────────────────┘${RESET}"
echo ""

# Clear logcat so we only see fresh output
adb logcat -c 2>/dev/null

# Run binary on device
adb shell /data/local/tmp/zenith_app

echo ""
echo -e "${GREEN}  ✓  App exited successfully${RESET}"
echo ""
echo -e "${DIM}  Tip: To see full Logcat stream run:${RESET}"
echo -e "${DIM}       adb logcat -s ZenithAndroid${RESET}"
echo ""
