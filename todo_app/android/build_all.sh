#!/bin/bash
set -e

echo "========================================="
echo "Building Zenith Android App (Rust + Dart)"
echo "========================================="

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Step 1: Build Rust native library
echo ""
echo "Step 1: Building Rust native library..."
echo "----------------------------------------"
"$SCRIPT_DIR/build_rust.sh"

# Step 2: Build C++ native library with CMake
echo ""
echo "Step 2: Building C++ native library..."
echo "----------------------------------------"
cd "$SCRIPT_DIR"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Copy library to jniLibs
mkdir -p ../app/src/main/jniLibs/arm64-v8a
cp libzenith_native.so ../app/src/main/jniLibs/arm64-v8a/

echo "C++ library built successfully!"

# Step 3: Build Dart/Flutter app
echo ""
echo "Step 3: Building Dart/Flutter app..."
echo "----------------------------------------"
"$SCRIPT_DIR/build_dart.sh"

echo ""
echo "========================================="
echo "Build complete!"
echo "APK location: $SCRIPT_DIR/../dart/build/app/outputs/flutter-apk/app-release.apk"
echo "========================================="
