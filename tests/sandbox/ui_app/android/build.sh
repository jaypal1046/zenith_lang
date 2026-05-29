#!/bin/bash
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
