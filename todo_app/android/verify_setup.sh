#!/bin/bash
set -e

echo "========================================="
echo "Zenith Android Setup Verification"
echo "========================================="
echo ""

ERRORS=0

# Check Rust files
echo "✓ Checking Rust native library..."
if [ -f "rust/include/zenith_native.h" ]; then
    echo "  ✓ zenith_native.h exists"
else
    echo "  ✗ MISSING: rust/include/zenith_native.h"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "rust/src/lib.rs" ]; then
    echo "  ✓ lib.rs exists"
    # Check for key functions
    if grep -q "zenith_native_init" rust/src/lib.rs; then
        echo "  ✓ zenith_native_init() implemented"
    else
        echo "  ✗ MISSING: zenith_native_init() in lib.rs"
        ERRORS=$((ERRORS + 1))
    fi
    
    if grep -q "zenith_native_execute" rust/src/lib.rs; then
        echo "  ✓ zenith_native_execute() implemented"
    else
        echo "  ✗ MISSING: zenith_native_execute() in lib.rs"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "  ✗ MISSING: rust/src/lib.rs"
    ERRORS=$((ERRORS + 1))
fi

# Check Dart files
echo ""
echo "✓ Checking Dart/Flutter app..."
if [ -f "dart/pubspec.yaml" ]; then
    echo "  ✓ pubspec.yaml exists"
else
    echo "  ✗ MISSING: dart/pubspec.yaml"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "dart/lib/main.dart" ]; then
    echo "  ✓ main.dart exists"
else
    echo "  ✗ MISSING: dart/lib/main.dart"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "dart/lib/zenith_native_bindings.dart" ]; then
    echo "  ✓ zenith_native_bindings.dart exists"
else
    echo "  ✗ MISSING: dart/lib/zenith_native_bindings.dart"
    ERRORS=$((ERRORS + 1))
fi

# Check Android build system
echo ""
echo "✓ Checking Android build system..."
if [ -f "android/CMakeLists.txt" ]; then
    echo "  ✓ CMakeLists.txt exists"
else
    echo "  ✗ MISSING: android/CMakeLists.txt"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/app/build.gradle" ]; then
    echo "  ✓ app/build.gradle exists"
else
    echo "  ✗ MISSING: android/app/build.gradle"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/build.gradle" ]; then
    echo "  ✓ build.gradle exists"
else
    echo "  ✗ MISSING: android/build.gradle"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/settings.gradle" ]; then
    echo "  ✓ settings.gradle exists"
else
    echo "  ✗ MISSING: android/settings.gradle"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/gradle.properties" ]; then
    echo "  ✓ gradle.properties exists"
else
    echo "  ✗ MISSING: android/gradle.properties"
    ERRORS=$((ERRORS + 1))
fi

# Check Android manifest
if [ -f "android/app/src/main/AndroidManifest.xml" ]; then
    echo "  ✓ AndroidManifest.xml exists"
    if grep -q 'android:hasCode="true"' android/app/src/main/AndroidManifest.xml; then
        echo "  ✓ hasCode set to true (Dart can run)"
    else
        echo "  ✗ WARNING: hasCode should be true"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "  ✗ MISSING: android/app/src/main/AndroidManifest.xml"
    ERRORS=$((ERRORS + 1))
fi

# Check Java activity
if [ -f "android/app/src/main/java/com/zenith/app/ZenithActivity.java" ]; then
    echo "  ✓ ZenithActivity.java exists"
else
    echo "  ✗ MISSING: ZenithActivity.java"
    ERRORS=$((ERRORS + 1))
fi

# Check build scripts
echo ""
echo "✓ Checking build scripts..."
if [ -f "android/build_rust.sh" ]; then
    echo "  ✓ build_rust.sh exists"
else
    echo "  ✗ MISSING: build_rust.sh"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/build_dart.sh" ]; then
    echo "  ✓ build_dart.sh exists"
else
    echo "  ✗ MISSING: build_dart.sh"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "android/build_all.sh" ]; then
    echo "  ✓ build_all.sh exists"
else
    echo "  ✗ MISSING: build_all.sh"
    ERRORS=$((ERRORS + 1))
fi

# Check jniLibs directories
echo ""
echo "✓ Checking jniLibs structure..."
for abi in arm64-v8a armeabi-v7a x86_64; do
    if [ -d "android/app/src/main/jniLibs/$abi" ]; then
        echo "  ✓ jniLibs/$abi directory exists"
    else
        echo "  ✗ MISSING: jniLibs/$abi directory"
        ERRORS=$((ERRORS + 1))
    fi
done

# Check native entry point
echo ""
echo "✓ Checking native entry point..."
if [ -f "android/main.cpp" ]; then
    echo "  ✓ main.cpp exists"
    if grep -q "ANativeActivity_onCreate" android/main.cpp; then
        echo "  ✓ ANativeActivity_onCreate() implemented"
    else
        echo "  ✗ MISSING: ANativeActivity_onCreate() in main.cpp"
        ERRORS=$((ERRORS + 1))
    fi
else
    echo "  ✗ MISSING: android/main.cpp"
    ERRORS=$((ERRORS + 1))
fi

# Summary
echo ""
echo "========================================="
if [ $ERRORS -eq 0 ]; then
    echo "✓ ALL CHECKS PASSED!"
    echo "========================================="
    echo ""
    echo "Your Zenith Android setup is complete!"
    echo ""
    echo "Next steps:"
    echo "1. Install prerequisites:"
    echo "   - Android NDK (set ANDROID_NDK env var)"
    echo "   - Rust with cargo-ndk: cargo install cargo-ndk"
    echo "   - Flutter SDK"
    echo ""
    echo "2. Build the app:"
    echo "   cd android"
    echo "   ./build_all.sh"
    echo ""
    echo "3. Install on device:"
    echo "   adb install ../dart/build/app/outputs/flutter-apk/app-release.apk"
    exit 0
else
    echo "✗ $ERRORS ERROR(S) FOUND"
    echo "========================================="
    echo "Please fix the missing components above."
    exit 1
fi
