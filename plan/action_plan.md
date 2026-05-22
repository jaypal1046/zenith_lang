# Zenith Compiler Action Plan

This document outlines the roadmap for the next development phases of the **Zenith** programming language and compilation toolchain.

---

## Current Status
* **Core Language:** Turing-complete syntax, Kotlin-style primary constructors, and Dart-style UI tree composition.
* **Compiler Pipeline:** Decoupled lexer, recursive-descent AST parser, transpiler mapping directly to standalone C++17.
* **Static Type System:** Compile-time validation of prompt templates, variable assignment constraints, constructor signatures, and list container generics.
* **Agentic Runtime:** Self-contained WinHTTP-based client querying local LLM providers (Ollama) with automatic simulated fallback.
* **Layout Engine:** Recursive flex-box layout engine computing exact absolute bounding boxes (`x`, `y`, `width`, `height`) for nested `Column` and `Row` elements.

---

## Phase 1: Diagnostics and Multi-File Support (COMPLETED)
* **Status:** Fully implemented.
  * **1.1 Error Location Reporting:** Token tracks line/col, syntax errors, and semantic errors report precise diagnostics.
  * **1.2 Multi-File Compiling and Imports:** Added inlining of `.zen` modules recursively, supporting multi-file codebases.

---

## Phase 2: OOP Extensions and Reactive State (COMPLETED)
* **Status:** Fully implemented.
  * **2.1 Interface & Inheritance Checks:** Interfaces are fully parsed, validated for signature completion, tested via polymorphic assignee checks (`isAssignable`), and compiled with virtual/pure-virtual functions.
  * **2.2 Reactive State Binding (`setState`):** Transpiled to automatic re-evaluation of layout rendering blocks when bound state changes.

---

## Phase 3: Graphics Rendering, Web Targets, and Android Native (COMPLETED)

### 3.1 Terminal Frame Buffer Renderer (COMPLETED)
* **Goal:** Draw actual boxes and text inside the terminal window using computed layout coordinates.
* **Action Items:**
  * Create a 2D text buffer grid in C++ (implemented using character-style grid separating styles from symbols).
  * Map layout boxes to grid coordinates and draw double-line borders (`┌`, `─`, `┐`, `│`) for `Column` and `Row` bounds.
  * Applied semantic ANSI escape color palettes for high-fidelity interactive terminal display.

### 3.2 Target-Specific Web Generation (WASM/JS) (COMPLETED)
* **Goal:** Target web platforms by generating JavaScript or WebAssembly.
* **Action Items:**
  * Implement an alternative code generator backend (`WASMCodeGenerator` and `JSCodeGenerator`).
  * Compile Zenith UI components directly to HTML DOM nodes.
  * Implement split-screen interactive logs and live reactivity showcase.

### 3.3 Android Native C++ Target (COMPLETED)
* **Goal:** Cross-compile and run native Zenith apps directly on Android via NDK.
* **Action Items:**
  * Refactored Agentic Runtime to conditionally compile using POSIX sockets (`posix_post`) on Linux/Android.
  * Automated toolchain selection, ABI detection, compilation, deployment, and execution via `build_android.bat`.

---

## Phase 4: Production Runtime, Keyboard Interaction, and Real Networking (COMPLETED)
* **Status:** Fully implemented.
  * **4.1 Cross-Platform Network Runtime:** Integrated `libcurl` conditionally (alongside `WinHTTP` on Windows desktop and custom socket channels on Android) with robust JSON parser logic.
  * **4.2 Interactive Terminal Loop:** Added collection of clickables, keyboard callback selection loops, reactive state re-renders, and TTY validation.
  * **4.3 Styling Attributes Compilation:** Integrated properties (`color`, `fontWeight`, `padding`) into both the C++ terminal renderer and the web CSS outputs.

---

## Phase 5: Production Tooling, Type System, and Standard Library (ROADMAP)

### 5.1 Advanced Type Inference & Generics
* **Goal:** Extend validation capabilities of the compiler frontend and allow type-safe expressions with minimal boilerplate.
* **Action Items:**
  - Implement Hindley-Milner / constraint-based type inference to allow developers to omit type definitions where possible.
  - Implement templates/generics for standard container collections (e.g. `List<T>`, `Map<K, V>`).

### 5.2 Yoga & CSS Flexbox Layout Engine Integration
* **Goal:** Adopt physical flexbox alignment and responsive coordinates on native and web platforms.
* **Action Items:**
  - Link the C-based Yoga layout engine into the C++ runtime for native targets.
  - Expose comprehensive padding, margins, flex-grow, flex-direction, and alignment properties in the Zenith AST.
  - Standardize CSS Flexbox mapping on web targets to align with Yoga's rendering outputs.

### 5.3 Rich Component Library
* **Goal:** Provide a comprehensive set of widgets for rich client user interfaces.
* **Action Items:**
  - Standardize native rendering and web mapping for `Image` and `Video` components.
  - Implement `Scrolling` (scroll view container) components with dynamic boundary constraints.
  - Add standard user input fields (text inputs, checkboxes, forms) with interactive state callbacks.

### 5.4 Developer Tooling (LSP & DAP)
* **Goal:** Build rich IDE development support for modern editors.
* **Action Items:**
  - Build a Language Server Protocol (LSP) implementation for auto-completions, syntax highlighting, diagnostics, and jump-to-definition.
  - Build a Debug Adapter Protocol (DAP) layer to support standard step-by-step debugger engines.

### 5.5 Centralized Package Registry
* **Goal:** Enable an ecosystem of reusable modular code.
* **Action Items:**
  - Create a CLI registry client (similar to cargo/npm) to retrieve, resolve, and link third-party modules.
