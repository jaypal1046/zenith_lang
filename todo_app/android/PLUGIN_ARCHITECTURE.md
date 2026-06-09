# Zenith Plugin Architecture for Android

## Overview

Zenith now supports a powerful plugin architecture that allows you to:
- **Write UI and business logic in Zenith** 
- **Use existing Dart packages from pub.dev** (location, camera, firebase, etc.)
- **Use high-performance Rust crates** (crypto, image processing, compression, etc.)

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    ZENITH APPLICATION                        │
│  (UI + Business Logic written in .zenith source files)      │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┴──────────────┐
        │                              │
        ▼                              ▼
┌──────────────────┐          ┌──────────────────┐
│   DART PLUGINS   │          │    RUST PLUGINS  │
│  (from pub.dev)  │          │  (from crates.io)│
│                  │          │                  │
│ • location       │          │ • crypto         │
│ • camera         │          │ • image          │
│ • firebase       │          │ • serde_json     │
│ • http           │          │ • regex          │
│ • shared_prefs   │          │ • compression    │
└────────┬─────────┘          └────────┬─────────┘
         │                             │
         │ JNI                         │ FFI
         │                             │
         ▼                             ▼
┌─────────────────────────────────────────────────────────────┐
│              NATIVE PLUGIN HOST (C++)                        │
│  • Plugin registry                                           │
│  • JSON-RPC call dispatch                                    │
│  • Event routing to Zenith VM                                │
└─────────────────────────────────────────────────────────────┘
```

## Using Plugins in Zenith Code

### Example 1: Using Location Plugin (Dart)

```zenith
// Import plugin
import "plugins/location" as location;

// Get current location
fn main() {
    var result = location.getCurrentLocation();
    
    if result.error {
        print("Error: " + result.error);
    } else {
        print("Latitude: " + result.latitude);
        print("Longitude: " + result.longitude);
        print("Accuracy: " + result.accuracy + " meters");
    }
}
```

### Example 2: Using Camera Plugin (Dart)

```zenith
import "plugins/camera" as camera;

fn takePhoto() {
    var result = camera.takePicture({
        "quality": 90,
        "saveToGallery": true
    });
    
    if result.path {
        print("Photo saved to: " + result.path);
        displayImage(result.path);
    }
}
```

### Example 3: Using Crypto Plugin (Rust)

```zenith
import "plugins/crypto" as crypto;

fn secureData() {
    var data = "Sensitive information";
    
    // Hash with SHA-256 (Rust implementation)
    var hash = crypto.hashSHA256(data);
    print("SHA-256: " + hash);
    
    // Encrypt with AES (Rust implementation)
    var key = "my-secret-key-32";
    var encrypted = crypto.encryptAES(data, key);
    print("Encrypted: " + encrypted);
}
```

### Example 4: Listening to Plugin Events

```zenith
import "plugins/location" as location;

fn trackMovement() {
    // Start continuous location updates
    location.startListening({"interval": 1000});
    
    // Handle location update events
    on event "onLocationChanged" {
        var loc = event.data;
        print("New location: " + loc.latitude + ", " + loc.longitude);
        updateMap(loc);
    }
}
```

## Available Plugins

### Dart Plugins (via pub.dev)

| Plugin | Package | Description |
|--------|---------|-------------|
| `location` | `geolocator` | GPS location, geocoding |
| `camera` | `camera` | Camera access, photo/video |
| `sensors` | `sensors_plus` | Accelerometer, gyroscope |
| `storage` | `shared_preferences` | Key-value storage |
| `http` | `http` | HTTP client |
| `firebase` | `firebase_core` | Firebase services |
| `image_picker` | `image_picker` | Pick images from gallery |

### Rust Plugins (via crates.io)

| Plugin | Crate | Description |
|--------|-------|-------------|
| `crypto` | `ring`, `rust-crypto` | SHA, AES, RSA encryption |
| `image` | `image` | Image processing |
| `json` | `serde_json` | Fast JSON parsing |
| `regex` | `regex` | Regular expressions |
| `compression` | `flate2` | GZIP, ZIP compression |

## Registering Plugins

Plugins are automatically registered when you import them. The build system handles:

1. **Dart plugins**: Added to `pubspec.yaml`, compiled into Flutter app
2. **Rust plugins**: Added to `Cargo.toml`, compiled to `.so` libraries
3. **Native binding**: C++ plugin host routes calls between Zenith VM and plugins

## Creating Custom Plugins

### Creating a Dart Plugin

1. Create plugin in Dart/Flutter project
2. Implement `MethodChannel` handler
3. Call from Zenith via `callDartPlugin()`

### Creating a Rust Plugin

1. Add function to `rust/src/lib.rs`
2. Mark with `#[no_mangle]` and `extern "C"`
3. Update `include/zenith_native.h`
4. Call from Zenith via `callRustPlugin()`

## Build Process

```bash
# Build Rust plugins
cd android
./build_rust.sh

# Build Dart plugins  
./build_dart.sh

# Build complete APK
./build_all.sh

# Install on device
adb install -r ../dart/build/app/outputs/flutter-apk/app-release.apk
```

## Error Handling

All plugin calls return JSON with standardized error format:

```json
// Success
{"status": "success", "data": {...}}

// Error
{"error": "Error message", "code": "ERROR_CODE"}
```

In Zenith code:

```zenith
var result = camera.takePicture();

if result.error {
    handleError(result.error, result.code);
} else {
    useResult(result.data);
}
```

## Performance Considerations

- **Rust plugins**: Best for CPU-intensive tasks (crypto, image processing)
- **Dart plugins**: Best for platform APIs (location, camera, sensors)
- **Async calls**: All plugin calls are asynchronous to avoid blocking Zenith VM
- **Event batching**: High-frequency events (sensors) are batched to reduce overhead

## Next Steps

1. Write your UI and logic in `.zenith` files
2. Import needed plugins with `import "plugins/<name>"`
3. Call plugin methods like regular functions
4. Build and deploy with `./build_all.sh`

For more examples, see `/workspace/todo_app/examples/plugins/`
