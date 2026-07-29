# Zenith Feature Audit: Compiler & Code Generation

## 1. Executive Overview
The Zenith compiler backend converts validated Zenith AST nodes into target source code or executable binary modules across C++17, JavaScript, and WebAssembly targets.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Native C++ Code Generator** ([codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/codegen.h), [codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/codegen.cpp)):
  - Emits modern C++17 code linking directly with runtime headers under `include/zenith/`.
  - Maps `Int`, `Float`, `Bool`, `String` into C++ type equivalents (`int32_t`, `float`, `bool`, `std::string`).
- **JavaScript Web Generator** ([js_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/js_codegen.h), [js_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/js_codegen.cpp)):
  - Emits ES6 JavaScript code suitable for browser execution and Node.js.
  - Translates scene lifecycle calls (`onLoad`, `onUpdate`, `onDraw`) into HTML canvas context operations.
- **WebAssembly Codegen Target** ([wasm_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/wasm_codegen.h), [wasm_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/wasm_codegen.cpp)):
  - Generates WebAssembly binary modules (`.wasm`) for near-native game execution on the web.
- **Profile-Aware Build Flow** ([main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp)):
  - Subcommand `zenith build <file.zen> --target=cpp|wasm|js`.

---

## 3. ❌ Missing / Incomplete Features
- **Direct LLVM IR Backend**: Transpiles to C++ source rather than emitting direct LLVM IR.
- **JIT Compiler Engine**: No in-memory Just-In-Time execution without codegen files.

---

## 4. 💻 How to Use
```bash
# Compile to C++ Native Desktop Binary
zenith build scenes/main.zen --target=cpp -o app.exe

# Compile to WebAssembly Binary Module
zenith build scenes/main.zen --target=wasm -o dist/app.wasm

# Transpile to JavaScript
zenith build scenes/main.zen --target=js -o dist/app.js
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File |
| :--- | :--- | :--- |
| **Native C++ Codegen** | [codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/codegen.h) | [codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/codegen.cpp) |
| **JavaScript Codegen** | [js_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/js_codegen.h) | [js_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/js_codegen.cpp) |
| **WebAssembly Codegen** | [wasm_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/wasm_codegen.h) | [wasm_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/wasm_codegen.cpp) |
| **Build CLI Dispatcher** | N/A | [main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp) |
