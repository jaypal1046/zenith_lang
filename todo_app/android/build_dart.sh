#!/bin/bash
set -e

echo "Building Dart/Flutter app for Android..."

cd "$(dirname "$0")/../dart"

# Check if Flutter is installed
if ! command -v flutter &> /dev/null; then
    echo "Error: Flutter is not installed. Please install Flutter first."
    exit 1
fi

# Get dependencies
echo "Getting Flutter dependencies..."
flutter pub get

# Generate FFI bindings from Rust header
echo "Generating FFI bindings..."
dart run ffigen --config pubspec.yaml

# Build APK
echo "Building APK..."
flutter build apk --release

echo "Dart/Flutter app built successfully!"
ls -la build/app/outputs/flutter-apk/
