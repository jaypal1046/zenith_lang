# Zenith Language: Detailed File-by-File Audit (`src/` & `include/`)

> **Document Purpose**: This document provides an exhaustive file-by-file audit of every single source file in `src/` and every single header in `include/`.

---

## 📂 Section 1: All Source Files in `src/`

### 1. [main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp) (203 KB)
- **Subsystem**: CLI Binary Dispatcher & Subcommand Router
- **Purpose**: Main entrypoint for the `zenith.exe` binary.
- **Implemented Features**: `create` project generator, `serve` dev preview server with hot reload watcher, `build` multi-target compiler, `assets` metadata sidecar importer & registry list, `lsp` server daemon launcher.
- **Missing / Gaps**: Remote package registry downloader (`zenith install`).

### 2. [runner.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/runner.cpp) (425 Bytes)
- **Subsystem**: Lightweight Test Runner
- **Purpose**: Minimal binary runner for executing test suites.

### 3. [src/frontend/lexer.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/lexer.cpp) (6.2 KB)
- **Subsystem**: Compiler Lexer & Tokenizer
- **Purpose**: Lexical scanner converting raw `.zen` source text into tokens.
- **Implemented Features**: Tokenizes primitive types (`Int`, `Float`, `Bool`, `String`, `Void`, `List`, `Map`), keywords (`class`, `implements`, `interface`, `foreign`, `let`, `async`, `await`, `struct`, `import`), and `@` annotations (`@library`, `@managed`).

### 4. [src/frontend/parser.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/parser.cpp) (50.2 KB)
- **Subsystem**: AST Parser
- **Purpose**: Recursive descent parser converting token streams into AST nodes.
- **Implemented Features**: Class parsing with parameter lists and interface realizations (`implements`), foreign C blocks (`foreign "C" { ... }`), list and map literal parsing, lambda expression parsing (`fn(x) { ... }`), UI component nodes.
- **Missing / Gaps**: Pattern/struct destructuring inside `match` arms.

### 5. [src/frontend/semantic.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/semantic.cpp) (113.8 KB)
- **Subsystem**: Semantic Analyzer & Type Checker
- **Purpose**: Type checking, symbol table construction, scope resolution, and AST validation.

### 6. [src/frontend/formatter.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/formatter.cpp) (14.1 KB)
- **Subsystem**: Code Formatter
- **Purpose**: AST-driven code formatting engine that emits canonical Zenith code layouts.

### 7. [src/backend/codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/codegen.cpp) (91.1 KB)
- **Subsystem**: Native C++ Code Generator
- **Purpose**: Transpiles Zenith AST nodes into performance-optimized C++17 source code linking runtime headers in `include/zenith/`.

### 8. [src/backend/js_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/js_codegen.cpp) (460.7 KB)
- **Subsystem**: JavaScript Web Generator
- **Purpose**: Transpiles Zenith AST nodes into ES6 JavaScript for browser execution and Node.js. Translates `onDraw(canvas, alpha)` scene callbacks into HTML5 Canvas 2D operations.

### 9. [src/backend/wasm_codegen.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/backend/wasm_codegen.cpp) (100.7 KB)
- **Subsystem**: WebAssembly Binary Generator
- **Purpose**: Emits WebAssembly binary modules (`.wasm`) for near-native web execution.

### 10. [src/lsp/lsp.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/lsp/lsp.cpp) (33.2 KB)
- **Subsystem**: Language Server Protocol (LSP) Daemon
- **Purpose**: Implements JSON-RPC over stdio for IDE integration. Handles diagnostics, autocompletion, hover tooltips, and go-to-definition.

### 11. [src/ui/win32_window.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/ui/win32_window.cpp) (17.1 KB)
- **Subsystem**: Win32 Native Desktop Backend
- **Purpose**: Windows OS window creation (`HWND`), Windows message loop (`GetMessage`/`DispatchMessage`), and native controls (`BS_PUSHBUTTON`, `ES_AUTOHSCROLL`, `BS_AUTOCHECKBOX`, `TRACKBAR_CLASS`).

### 12. [src/zenith/ui/yoga_layout.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/zenith/ui/yoga_layout.cpp) (31.2 KB)
- **Subsystem**: Yoga Flexbox Layout Adapter
- **Purpose**: Bridges the Facebook Yoga Flexbox C++ calculation engine to Zenith UIElements.

---

## 📂 Section 2: All Header Files in `include/`

### 1. [include/ast/ast.h](file:///c:/Jay/_Plugin/zenith_lang/include/ast/ast.h) (14.2 KB)
- **Subsystem**: AST Node Declarations
- **Purpose**: Declares AST node structs (`TypeNode`, `VarDeclNode`, `FunctionDeclNode`, `ClassDeclNode`, `UIComponentNode`, `LambdaNode`, etc.).

### 2. [include/frontend/lexer.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/lexer.h) (750 Bytes)
- **Subsystem**: Lexer Interface & Token Types

### 3. [include/frontend/parser.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/parser.h) (1.8 KB)
- **Subsystem**: Parser Class Signatures & Parser Rules

### 4. [include/frontend/semantic.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/semantic.h) (3.0 KB)
- **Subsystem**: Semantic Analyzer & Symbol Table Interfaces

### 5. [include/frontend/formatter.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/formatter.h) (435 Bytes)
- **Subsystem**: Code Formatter Header

### 6. [include/frontend/type_inference.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/type_inference.hpp) (15.0 KB)
- **Subsystem**: Hindley-Milner Type Inference Engine
- **Purpose**: Type inferencer algorithm resolving implicit generic and variable types.

### 7. [include/backend/codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/codegen.h) (1.3 KB)
- **Subsystem**: Native C++ CodeGen Interface

### 8. [include/backend/js_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/js_codegen.h) (1.2 KB)
- **Subsystem**: JS CodeGen Interface

### 9. [include/backend/wasm_codegen.h](file:///c:/Jay/_Plugin/zenith_lang/include/backend/wasm_codegen.h) (1.9 KB)
- **Subsystem**: WebAssembly CodeGen Interface

### 10. [include/lsp/lsp.h](file:///c:/Jay/_Plugin/zenith_lang/include/lsp/lsp.h) (86 Bytes)
- **Subsystem**: LSP Daemon Interface Header

### 11. [include/zenith_runtime.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith_runtime.h) (1.1 KB)
- **Subsystem**: Top-Level Runtime Composite Header

### 12. [include/zenith/common/zenith_common.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/common/zenith_common.h) (50.3 KB)
- **Subsystem**: Common Runtime & Math Primitives
- **Purpose**: Mathematical vector types (`Vec2`, `Vec3`, `Vec4`, `Mat4`), matrix transformations, string formatting, and core data types.

### 13. [include/zenith/memory/zenith_memory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/memory/zenith_memory.h) (14.2 KB)
- **Subsystem**: Memory Management & Smart Pointers
- **Purpose**: Reference counting (`Ref<T>`, `Weak<T>`), arena allocators, and garbage collection root management (`@gc_root`).

### 14. [include/zenith/ffi/zenith_ffi_dynamic_lib.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ffi/zenith_ffi_dynamic_lib.h) (7.2 KB)
- **Subsystem**: Dynamic Library FFI Loader
- **Purpose**: Cross-platform shared library (`.dll`, `.so`, `.dylib`) symbol loading.

### 15. [include/zenith/ffi/zenith_ffi_python.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ffi/zenith_ffi_python.h) (11.7 KB)
- **Subsystem**: CPython C-API FFI Interop

### 16. [include/zenith/std/concurrency.hpp](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/std/concurrency.hpp) (11.6 KB)
- **Subsystem**: Multi-threaded Concurrency Runtime
- **Purpose**: Thread pools, work-stealing job queues, and async future primitives.

### 17. [include/zenith/ui/native_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_window.h) (2.8 KB)
- **Subsystem**: Abstract Native Window Interface

### 18. [include/zenith/ui/native_widget_factory.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/native_widget_factory.h) (3.4 KB)
- **Subsystem**: Abstract Native Control Factory

### 19. [include/zenith/ui/win32_window.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/win32_window.h) (3.4 KB)
- **Subsystem**: Win32 Window Implementation Header

### 20. [include/zenith/ui/yoga_layout.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ui/yoga_layout.h) (13.0 KB)
- **Subsystem**: Yoga Flexbox Layout Adapter Header

### 21. Platform Bridge Headers
- **[include/zenith/desktop/windows/zenith_windows.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/windows/zenith_windows.h)** (24.2 KB): WinHTTP client, system console, and native window runner.
- **[include/zenith/desktop/posix/zenith_posix.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/posix/zenith_posix.h)** (22.6 KB): POSIX socket networking and system calls.
- **[include/zenith/android/zenith_android.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/android/zenith_android.h)** (8.0 KB): Android JNI platform bindings.
- **[include/zenith/desktop/mac/zenith_mac.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/mac/zenith_mac.h)**: macOS platform stub.
- **[include/zenith/desktop/linux/zenith_linux.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/linux/zenith_linux.h)**: Linux platform stub.
- **[include/zenith/ios/zenith_ios.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/ios/zenith_ios.h)**: iOS Metal platform stub.
- **[include/zenith/web/zenith_web.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/web/zenith_web.h)**: WebAssembly DOM interop.

### 22. Game Runtime Headers in `include/zenith/game/`
- **[zenith_world.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_world.h)** (63.7 KB): Core ECS world managing 21 dense component pools (`ComponentPool<T>`).
- **[zenith_physics.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_physics.h)** (35.1 KB): 2D/3D RigidBody dynamics, Euler solver, spatial hash, raycast/sweep mask queries (`raycast2DMask`, `raycast3DMask`).
- **[zenith_scene.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_scene.h)** (227.3 KB): Scene lifecycle, view proxies, prefabs (`instantiatePrefab2D/3D`), material variants (`cloneVariant`).
- **[zenith_resource.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_resource.h)** (71.3 KB): Typed numeric handles (`TextureHandle`, `MeshHandle`, etc.), asset database, RAM/VRAM memory budget tracking.
- **[zenith_game.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/game/zenith_game.h)** (67.3 KB): Composite game engine header.
- **Gameplay Mechanics Subsystems (100+ Headers)**: `zenith_behavior_tree.h`, `zenith_dialogue.h`, `zenith_ik2d.h`, `zenith_inventory.h`, `zenith_pathfinding.h`, `zenith_particles.h`, `zenith_audio.h`, `zenith_crafting2d.h`, `zenith_dungeon_gen.h`, `zenith_quest.h`, `zenith_save_system.h`, `zenith_skill_tree2d.h`, `zenith_steering2d.h`, etc.
- **Shader FX & Post-Processing Suite (20 Shader Headers)**: `zenith_bloom2d.h`, `zenith_blur2d.h`, `zenith_crt2d.h`, `zenith_digital_glitch2d.h`, `zenith_distortion2d.h`, `zenith_hologram2d.h`, `zenith_night_vision2d.h`, `zenith_oil_paint2d.h`, `zenith_pixel_sort2d.h`, `zenith_shockwave2d.h`, `zenith_vhs2d.h`, `zenith_vignette2d.h`, etc.
