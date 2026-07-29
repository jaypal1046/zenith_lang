# Zenith Feature Audit: Standard Library, FFI & Platform Bridges

## 1. Executive Overview
Zenith provides standard library modules (`std.io`, `std.fs`, `std.net`, `std.math`), a block-based Foreign Function Interface (`foreign "C"`), dynamic library loading, Python FFI bindings, memory management runtime headers, multi-threaded concurrency headers, and Android/iOS platform wrappers.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Standard Library Modules**:
  - `std.io`: Print formatting (`println`).
  - `std.fs`: File system reading & writing.
  - `std.net`: HTTP networking.
  - `std.math`: Vector math and trigonometry.
- **Concurrency Framework** ([concurrency.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/std/concurrency.hpp)):
  - Multi-threaded worker queues and thread-pool execution primitives.
- **Memory Management Runtime** ([zenith_memory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/memory/zenith_memory.h)):
  - Reference counting (`Ref<T>`, `Weak<T>`) and garbage collection root management (`@gc_root`, `@managed`).
- **Dynamic Library FFI & Python Interop**:
  - `include/zenith/ffi/zenith_ffi_dynamic_lib.h` for `.dll`/`.so` shared library symbol resolution.
  - `include/zenith/ffi/zenith_ffi_python.h` for CPython C-API bindings.
- **Windows Platform & WinHTTP Integration**:
  - [zenith_windows.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/windows/zenith_windows.h) for WinHTTP requests and Win32 system console interop.
- **Mobile Platform Headers**:
  - Android JNI wrappers under [include/zenith/android/](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/android/).
  - iOS Metal & Objective-C wrappers under [include/zenith/ios/](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ios/).

---

## 3. ❌ Missing / Incomplete Features & Strategic Rust FFI Bridges
- **SQLite Database & Networking FFI**: Use thin C ABI bridges to Rust `rusqlite` and `reqwest` for robust, cross-platform stdlib database & HTTP services without bloat.

---

## 4. 💻 How to Use (Real Compiling Syntax)
```zenith
import std.io;

@library("builtin")
foreign "C" {
    Bool isKeyPressed(String key);
    Bool wasKeyPressed(String key);
}

Void main() {
    if (wasKeyPressed("Space")) {
        println("Space bar was pressed!");
    }
}
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File |
| :--- | :--- | :--- |
| **Concurrency Infrastructure** | [concurrency.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/std/concurrency.hpp) | N/A (Header-only thread pool) |
| **Memory Runtime & RefCount** | [zenith_memory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/memory/zenith_memory.h) | N/A |
| **Dynamic Library Loading** | [zenith_ffi_dynamic_lib.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ffi/zenith_ffi_dynamic_lib.h) | N/A |
| **Python FFI Interop** | [zenith_ffi_python.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ffi/zenith_ffi_python.h) | N/A |
| **Windows Platform Bridge** | [zenith_windows.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/windows/zenith_windows.h) | N/A |
| **Android Headers** | [include/zenith/android/](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/android/) | N/A |
| **iOS Headers** | [include/zenith/ios/](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ios/) | N/A |
