# Zenith Android Build Instructions

This directory contains the complete Android build system for Zenith apps with Rust and Dart/Flutter integration.

## Prerequisites

1. **Android NDK** (r25 or later)
   - Set `ANDROID_NDK` environment variable
   - Or install via Android Studio SDK Manager

2. **Rust with cargo-ndk**
   ```bash
   rustup target add aarch64-linux-android armv7-linux-androideabi x86_64-linux-android
   cargo install cargo-ndk
   ```

3. **Flutter SDK** (3.0 or later)
   - Add Flutter to PATH
   - Run `flutter doctor` to verify installation

4. **CMake** (3.22 or later)

## Project Structure

```
android/
├── app/
│   ├── src/main/
│   │   ├── java/com/zenith/app/
│   │   │   └── ZenithActivity.java    # Java JNI bridge
│   │   ├── jniLibs/                    # Native libraries (generated)
│   │   └── AndroidManifest.xml         # App manifest
│   ├── build.gradle                    # App-level Gradle config
│   └── proguard-rules.pro              # ProGuard rules
├── build.gradle                        # Project-level Gradle config
├── settings.gradle                     # Gradle settings
├── CMakeLists.txt                      # CMake build config
├── gradle.properties                   # Gradle properties
├── build_rust.sh                       # Rust build script
├── build_dart.sh                       # Dart/Flutter build script
└── build_all.sh                        # Complete build script
```

## Building

### Quick Build (All Components)

```bash
cd android
./build_all.sh
```

This will:
1. Build Rust native library for all Android ABIs
2. Build C++ native library with CMake
3. Build Dart/Flutter APK

### Step-by-Step Build

#### 1. Build Rust Library

```bash
./build_rust.sh
```

Output: `app/src/main/jniLibs/*/libzenith_native.so`

#### 2. Build C++ Library

```bash
mkdir -p build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Output: `build/libzenith_native.so`

#### 3. Build Dart/Flutter App

```bash
./build_dart.sh
```

Output: `../dart/build/app/outputs/flutter-apk/app-release.apk`

## Architecture

### Native Integration Flow

```
┌─────────────────┐
│  Dart/Flutter   │
│      UI         │
└────────┬────────┘
         │ FFI
         ▼
┌─────────────────┐
│  Rust Native    │
│   Library       │
│  (libzenith_    │
│   native.so)    │
└────────┬────────┘
         │ JNI
         ▼
┌─────────────────┐
│  C++ Runtime    │
│  (Zenith Core)  │
└─────────────────┘
```

### Component Responsibilities

- **Dart/Flutter**: UI rendering, user interaction
- **Rust**: High-performance computations, FFI interface
- **C++**: Zenith language runtime, transpiler output execution

## Testing

### Test Rust Functions

```bash
cd rust
cargo test
```

### Test on Device

```bash
adb install -r ../dart/build/app/outputs/flutter-apk/app-release.apk
adb logcat | grep Zenith
```

## Troubleshooting

### Common Issues

1. **NDK not found**
   ```bash
   export ANDROID_NDK=/path/to/ndk
   ```

2. **Rust target missing**
   ```bash
   rustup target add aarch64-linux-android
   ```

3. **Flutter dependencies**
   ```bash
   cd dart
   flutter pub get
   ```

4. **CMake version too old**
   - Install CMake 3.22+ from https://cmake.org/download/

## Deployment

Install the APK on a device:

```bash
adb install dart/build/app/outputs/flutter-apk/app-release.apk
```

Or distribute through Google Play Store after signing.
