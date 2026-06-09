# Zenith Plugin System

## Architecture Overview

The Zenith Plugin System allows you to write your **UI and business logic in Zenith**, while leveraging existing packages from:
- **pub.dev** (Dart/Flutter plugins like location, camera, sensors)
- **crates.io** (Rust crates for cryptography, compression, etc.)

```
┌─────────────────────────────────────────────────────────┐
│                   ZENITH CODE                           │
│  (Your UI & Business Logic)                             │
│                                                         │
│  import "location" as loc;                              │
│  import "crypto" as cry;                                │
│  var pos = loc.get_current_position();                  │
│  var hash = cry.sha256("data");                         │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────┴───────────┐
         │                       │
         ▼                       ▼
┌─────────────────┐   ┌─────────────────┐
│   DART PLUGINS  │   │   RUST CRATES   │
│   (pub.dev)     │   │   (crates.io)   │
│                 │   │                 │
│ • location      │   │ • sha256        │
│ • camera        │   │ • aes-gcm       │
│ • sensors       │   │ • lz4           │
│ • http          │   │ • serde_json    │
└─────────────────┘   └─────────────────┘
         │                       │
         └───────────┬───────────┘
                     │
         ┌───────────▼───────────┐
         │   ZENITH RUNTIME      │
         │   (C++ Core)          │
         └───────────────────────┘
```

## How It Works

### 1. Write Zenith Code
```zenith
// examples/plugin_usage.zenith
import "location" as loc;
import "camera" as cam;
import "crypto" as cry;

func main() {
    // Call Dart plugin (pub.dev)
    var location_data = loc.get_current_position({
        "accuracy": "high",
        "timeout": 5000
    });
    
    print("Location: " + location_data);
    
    // Call Rust crate
    var hash = cry.sha256("Hello Zenith");
    print("SHA256: " + hash);
    
    // Use camera (Dart plugin with native C++)
    var image = cam.capture_photo({
        "resolution": "1920x1080",
        "flash": "auto"
    });
    
    display(image);
}
```

### 2. Create Dart Plugin Wrapper
```dart
// dart/lib/plugin_wrappers.dart
class LocationPlugin {
  static const MethodChannel _channel = 
      MethodChannel('com.zenith.plugin/location');

  static void register() {
    _channel.setMethodCallHandler(_handleMethodCall);
  }

  static Future<dynamic> _handleMethodCall(MethodCall call) async {
    final method = call.arguments['method'];
    final args = jsonDecode(call.arguments['args_json']);

    switch (method) {
      case 'get_current_position':
        // Use actual location package from pub.dev
        var location = Location();
        var data = await location.getLocation();
        return jsonEncode(data);
      default:
        throw PlatformException(code: 'UNKNOWN_METHOD');
    }
  }
}
```

### 3. Register Plugin in main.dart
```dart
void main() {
  registerAllPlugins(); // Registers location, camera, etc.
  runApp(ZenithApp());
}
```

### 4. Build & Run
```bash
cd android
./build_all.sh
adb install ../dart/build/app/outputs/flutter-apk/app-release.apk
```

## Available Plugins

### Dart Plugins (pub.dev)
| Plugin | Package | Description |
|--------|---------|-------------|
| `location` | `location:^5.0.0` | GPS location services |
| `camera` | `camera:^0.10.0` | Camera access & preview |
| `sensors` | `sensors_plus:^4.0.0` | Accelerometer, gyroscope |
| `http` | `http:^1.1.0` | HTTP client |
| `shared_preferences` | `shared_preferences:^2.2.0` | Local storage |

### Rust Crates (crates.io)
| Plugin | Crate | Description |
|--------|-------|-------------|
| `crypto` | `sha2:^0.10.0` | SHA256, SHA512 hashing |
| `compression` | `lz4:^1.24.0` | Fast compression |
| `encryption` | `aes-gcm:^0.10.0` | AES-GCM encryption |
| `json` | `serde_json:^1.0.0` | JSON parsing |

## Adding New Plugins

### For Dart Packages:
1. Add dependency to `dart/pubspec.yaml`
2. Create wrapper in `dart/lib/plugin_wrappers.dart`
3. Register in `registerAllPlugins()`

### For Rust Crates:
1. Add dependency to `rust/Cargo.toml`
2. Implement FFI function in `rust/src/lib.rs`
3. Export in `rust/include/zenith_native.h`

## Benefits

✅ **Write once in Zenith** - Your app logic stays in one language  
✅ **Access 50,000+ pub.dev packages** - Leverage existing Flutter ecosystem  
✅ **High-performance Rust** - Use Rust for CPU-intensive tasks  
✅ **Native performance** - Direct FFI/JNI calls, no overhead  
✅ **Type-safe** - JSON-based communication with validation  

## Next Steps

1. Explore `examples/plugin_usage.zenith` for usage patterns
2. Check `dart/lib/plugin_wrappers.dart` for implementation templates
3. Read `rust/src/lib.rs` for Rust FFI examples
4. Build the demo: `./android/build_all.sh`
