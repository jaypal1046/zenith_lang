# 🚀 Zenith Plugin System - Quick Start Guide

## Overview
The Zenith Plugin Bridge System enables you to write your entire application UI and logic in **Zenith**, while seamlessly accessing thousands of existing packages from **Dart (pub.dev)** and **Rust (crates.io)**.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                   ZENITH CODE                           │
│  import "native:location" as loc;                       │
│  import "native:crypto" as cry;                         │
│  var pos = loc.get_current_position();                  │
└────────────────────┬────────────────────────────────────┘
                     │
         ┌───────────┴───────────┐
         │   JSON-RPC Bridge     │
         └───────────┬───────────┘
                     │
         ┌───────────┴───────────┐
         ▼                       ▼
┌─────────────────┐   ┌─────────────────┐
│   DART PLUGINS  │   │   RUST CRATES   │
│   (pub.dev)     │   │   (crates.io)   │
│ • location      │   │ • sha256        │
│ • camera        │   │ • aes-gcm       │
│ • sensors       │   │ • compression   │
└─────────────────┘   └─────────────────┘
```

## Quick Start

### Step 1: Add Dependencies

**For Dart plugins** (`dart/pubspec.yaml`):
```yaml
dependencies:
  location: ^5.0.0
  camera: ^0.10.0
  http: ^1.1.0
```

**For Rust crates** (`rust/Cargo.toml`):
```toml
[dependencies]
sha2 = "0.10"
aes-gcm = "0.10"
serde_json = "1.0"
```

### Step 2: Write Zenith Code

```zenith
// examples/plugin_usage.zenith
import "native:location" as loc;
import "native:camera" as cam;
import "native:crypto" as cry;

func main() {
    // Call Dart plugin (pub.dev)
    var location_data = loc.get_current_position({
        "accuracy": "high",
        "timeout": 5000
    });
    print("Location: " + location_data);

    // Call Rust crate (crates.io)
    var hash = cry.sha256("Hello Zenith");
    print("SHA256: " + hash);

    // Use camera
    var image = cam.capture_photo({
        "resolution": "1920x1080",
        "flash": "auto"
    });
    display(image);
}
```

### Step 3: Create Dart Plugin Wrapper

```dart
// dart/lib/plugin_wrappers.dart
import 'package:flutter/services.dart';
import 'package:location/location.dart';

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
        var location = Location();
        var data = await location.getLocation();
        return jsonEncode(data);
      default:
        throw PlatformException(code: 'UNKNOWN_METHOD');
    }
  }
}
```

### Step 4: Build & Run

```bash
cd /workspace/todo_app/android
./build_all.sh
adb install ../dart/build/app/outputs/flutter-apk/app-release.apk
```

## Available Plugins

### Dart Plugins (pub.dev)
| Plugin | Package | Status |
|--------|---------|--------|
| `location` | `location:^5.0.0` | ✅ Template Ready |
| `camera` | `camera:^0.10.0` | ✅ Template Ready |
| `sensors` | `sensors_plus:^4.0.0` | 📝 To Implement |
| `http` | `http:^1.1.0` | 📝 To Implement |
| `shared_preferences` | `shared_preferences:^2.2.0` | 📝 To Implement |

### Rust Crates (crates.io)
| Plugin | Crate | Status |
|--------|-------|--------|
| `crypto` | `sha2:^0.10.0` | ✅ Implemented |
| `compression` | `lz4:^1.24.0` | 📝 To Implement |
| `encryption` | `aes-gcm:^0.10.0` | 📝 To Implement |

## Adding New Plugins

### For Dart Packages:
1. Add dependency to `dart/pubspec.yaml`
2. Create wrapper in `dart/lib/plugin_wrappers.dart`
3. Register in `registerAllPlugins()`

### For Rust Crates:
1. Add dependency to `rust/Cargo.toml`
2. Implement FFI function in `rust/src/lib.rs`
3. Export in `rust/include/zenith_native.h`

## Key Files Reference

| File | Purpose |
|------|---------|
| `examples/plugin_usage.zenith` | Usage examples |
| `dart/lib/plugin_wrappers.dart` | Dart plugin templates |
| `rust/src/lib.rs` | Rust FFI implementations |
| `android/app/src/main/java/com/zenith/app/PluginHost.java` | Java bridge |
| `android/PLUGIN_SYSTEM.md` | Full architecture docs |

## Benefits

✅ **Write once in Zenith** - Your app logic stays in one language  
✅ **Access 50,000+ pub.dev packages** - Leverage existing Flutter ecosystem  
✅ **High-performance Rust** - Use Rust for CPU-intensive tasks  
✅ **Native performance** - Direct FFI/JNI calls, no overhead  
✅ **Type-safe** - JSON-based communication with validation  

## Next Steps

1. **Explore examples**: Check `examples/plugin_usage.zenith`
2. **Review templates**: See `dart/lib/plugin_wrappers.dart`
3. **Read full docs**: Open `android/PLUGIN_SYSTEM.md`
4. **Build demo**: Run `./android/build_all.sh`
5. **Start coding**: Create your own `.zenith` files!

---

**Happy Coding with Zenith! 🎉**
