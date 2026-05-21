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

## Phase 4: Production Runtime, Keyboard Interaction, and Real Networking (IN PROGRESS)

### 4.1 Cross-Platform Network Runtime
* **Goal:** Upgrade the C++ runtime to perform real, asynchronous HTTP requests using `libcurl` and parse LLM JSON payloads.
* **Action Items:**
  * Implement general `libcurl` bindings inside `zenith_runtime.h` for platform-agnostic HTTP POST/GET.
  * Add support for header-only JSON parsing for incoming agentic completions.

### 4.2 Interactive Terminal Loop
* **Goal:** Support keyboard input and rendering updates inside the ANSI terminal application.
* **Action Items:**
  * Implement a main event loop that catches keyboard keystrokes (`w`, `a`, `s`, `d`, `enter`, `escape`).
  * Propagate button click events (`onClick: increment`) from terminal actions.

### 4.3 Styling Attributes Compilation
* **Goal:** Support dynamic styling attributes (`color`, `backgroundColor`, `margin`, `padding`).
* **Action Items:**
  * Update AST and Parser to support color attributes.
  * Map layout styles to ANSI colors in terminal and CSS values in JS/Wasm code generators.

