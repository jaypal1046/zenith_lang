# 🦀 Rust Integration - Basic Setup

Learn how to integrate Rust libraries into your Zenith applications for maximum performance.

## 📋 What You'll Learn

- Setting up Rust bridge
- Creating FFI bindings
- Calling Rust functions from Zenith
- Error handling
- Memory management

## 🎯 Prerequisites

- Rust installed (`rustup install stable`)
- Zenith SDK 1.0+

## 📝 Step-by-Step Guide

### 1. Create Rust Library

```rust
// rust_lib/src/lib.rs
use zenith_ffi::prelude::*;

#[zenith_export]
pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

#[zenith_export]
pub fn fibonacci(n: u32) -> u64 {
    match n {
        0 => 0,
        1 => 1,
        _ => {
            let mut a = 0u64;
            let mut b = 1u64;
            for _ in 2..=n {
                let temp = a + b;
                a = b;
                b = temp;
            }
            b
        }
    }
}

#[zenith_export]
pub fn process_data(data: Vec<u8>) -> Vec<u8> {
    data.iter().map(|&x| x * 2).collect()
}
```

### 2. Configure Cargo.toml

```toml
# rust_lib/Cargo.toml
[package]
name = "zenith_rust_lib"
version = "0.1.0"
edition = "2021"

[lib]
crate-type = ["cdylib", "rlib"]

[dependencies]
zenith-ffi = "0.1"
serde = { version = "1.0", features = ["derive"] }
serde_json = "1.0"

[profile.release]
opt-level = 3
lto = true
```

### 3. Build Rust Library

```bash
cd rust_lib

# Build for current platform
cargo build --release

# Build for specific platforms
cargo build --release --target wasm32-unknown-unknown  # Web
cargo build --release --target aarch64-apple-ios       # iOS
cargo build --release --target aarch64-linux-android   # Android
```

### 4. Use in Zenith Application

```zenith
import 'package:zenith_rust/zenith_rust.dart';
import 'dart:ffi';
import 'dart:io';

void main() async {
  // Initialize Rust bridge
  await RustBridge.initialize();
  
  runApp(RustDemoApp());
}

class RustDemoApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('🦀 Rust Integration Demo')),
        body: RustExample(),
      ),
    );
  }
}

class RustExample extends StatefulWidget {
  @override
  _RustExampleState createState() => _RustExampleState();
}

class _RustExampleState extends State<RustExample> {
  String result = '';
  bool isLoading = false;
  
  // Load the Rust library
  late final DynamicLibrary lib;
  
  @override
  void initState() {
    super.initState();
    loadLibrary();
  }
  
  Future<void> loadLibrary() async {
    if (Platform.isAndroid || Platform.isIOS) {
      lib = DynamicLibrary.open('libzenith_rust_lib.so');
    } else if (Platform.isWindows) {
      lib = DynamicLibrary.open('zenith_rust_lib.dll');
    } else if (Platform.isMacOS) {
      lib = DynamicLibrary.open('libzenith_rust_lib.dylib');
    } else if (Platform.isLinux) {
      lib = DynamicLibrary.open('libzenith_rust_lib.so');
    }
  }
  
  // Call Rust function: add
  int callAdd(int a, int b) {
    final addFunc = lib.lookupFunction<Int32 Function(Int32, Int32), int Function(int, int)>('add');
    return addFunc(a, b);
  }
  
  // Call Rust function: fibonacci
  int callFibonacci(int n) {
    final fibFunc = lib.lookupFunction<Uint64 Function(Uint32), int Function(int)>('fibonacci');
    return fibFunc(n);
  }
  
  // Call Rust function: process_data
  List<int> callProcessData(List<int> data) {
    final processFunc = lib.lookupFunction<Pointer<Uint8> Function(Pointer<Uint8>, Int32), Pointer<Uint8> Function(Pointer<Uint8>, int)>('process_data');
    // Implementation depends on data size and memory management
    // See advanced example for complete implementation
    return [];
  }
  
  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: EdgeInsets.all(20),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            'Basic Arithmetic',
            style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
          ),
          SizedBox(height: 16),
          ElevatedButton(
            onPressed: () {
              final sum = callAdd(5, 3);
              setState(() {
                result = '5 + 3 = $sum (calculated in Rust)';
              });
            },
            child: Text('Calculate 5 + 3'),
          ),
          SizedBox(height: 20),
          Text(
            'Fibonacci Sequence',
            style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
          ),
          SizedBox(height: 16),
          ElevatedButton(
            onPressed: () {
              final fib = callFibonacci(50);
              setState(() {
                result = 'Fibonacci(50) = $fib (calculated in Rust)';
              });
            },
            child: Text('Calculate Fibonacci(50)'),
          ),
          SizedBox(height: 20),
          if (result.isNotEmpty)
            Container(
              padding: EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: Colors.blue.withOpacity(0.1),
                borderRadius: BorderRadius.circular(8),
              ),
              child: Text(
                result,
                style: TextStyle(fontSize: 18, fontFamily: 'monospace'),
              ),
            ),
        ],
      ),
    );
  }
}
```

### 5. Using zenith_rust Package (Simplified)

```zenith
import 'package:zenith_rust/zenith_rust.dart';

// Auto-generated bindings make it easier!
class SimpleRustExample extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        ElevatedButton(
          onPressed: () {
            // Direct function call with auto-bindings
            final sum = RustLib.add(5, 3);
            print('Result: $sum');
          },
          child: Text('Call Rust add()'),
        ),
        ElevatedButton(
          onPressed: () {
            final fib = RustLib.fibonacci(50);
            print('Fibonacci: $fib');
          },
          child: Text('Call Rust fibonacci()'),
        ),
      ],
    );
  }
}
```

## 🔧 Configuration

### pubspec.yaml

```yaml
dependencies:
  zenith_rust: ^1.0.0
  
dev_dependencies:
  zenith_rust_generator: ^1.0.0
  
zenith_rust:
  rust_lib_path: ../rust_lib
  bindings_output: lib/rust_bindings.dart
  platforms:
    - android
    - ios
    - windows
    - macos
    - linux
    - web
```

### Build Script

```bash
#!/bin/bash
# build_rust.sh

echo "Building Rust libraries for all platforms..."

# Desktop
cargo build --release --target x86_64-pc-windows-msvc
cargo build --release --target x86_64-apple-darwin
cargo build --release --target x86_64-unknown-linux-gnu

# Mobile
cargo build --release --target aarch64-apple-ios
cargo build --release --target aarch64-linux-android

# WebAssembly
cargo build --release --target wasm32-unknown-unknown

echo "Build complete!"
```

## 💡 Best Practices

1. **Keep Rust functions pure** - Avoid side effects when possible
2. **Use serde for complex types** - Serialize/deserialize JSON for complex data
3. **Handle errors properly** - Return Result types from Rust
4. **Profile before optimizing** - Only use Rust for performance-critical code
5. **Test on all platforms** - Ensure compatibility across targets

## ⚠️ Common Pitfalls

- Forgetting to copy .so/.dll/.dylib files to app bundle
- Not handling null pointers in FFI
- Memory leaks from unmanaged allocations
- Thread safety issues with shared state

## 📚 Next Steps

- [Using serde for JSON](02-json.md) - Complex data serialization
- [Async Operations](03-async.md) - Async Rust in Zenith
- [Performance Optimization](04-performance.md) - Advanced techniques

---

**Next**: [JSON Serialization →](02-json.md)
