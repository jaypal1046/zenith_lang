#!/bin/bash
set -e

echo "Building Rust native library for Android..."

cd "$(dirname "$0")/../rust"

# Check if cargo-ndk is installed
if ! command -v cargo-ndk &> /dev/null; then
    echo "Installing cargo-ndk..."
    cargo install cargo-ndk
fi

# Build for all Android targets
echo "Building for arm64-v8a..."
cargo ndk -t arm64-v8a -o ../android/app/src/main/jniLibs build --release

echo "Building for armeabi-v7a..."
cargo ndk -t armeabi-v7a -o ../android/app/src/main/jniLibs build --release

echo "Building for x86_64..."
cargo ndk -t x86_64 -o ../android/app/src/main/jniLibs build --release

echo "Rust libraries built successfully!"
ls -la ../android/app/src/main/jniLibs/*/
