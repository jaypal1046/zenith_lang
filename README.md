# Zenith Language
**The World's First Native Agentic Systems Language**

Zenith is a general-purpose, high-performance systems language that natively combines C++ execution speed, universal UI generation, and native Agentic (AI) primitives at the compiler level.

---

## 1. The Core Philosophy
Modern AI app development is fractured. Developers write UI in TypeScript/Dart, Agent logic in Python, and run LLM inference engines in C++. Zenith unifies this stack into a single, cohesive compiler architecture.

- **Frontend (Syntax & AST):** Custom C++ Parser supporting strictly-typed AI primitives.
- **Middle-end (UI Layout):** Integration with **Yoga** (C++ Flexbox Engine) to compute cross-platform UI layouts mathematically at compile time.
- **Backend (Code Generation):** Binds to **LLVM** to generate bare-metal machine code (Windows, Mac, Linux, iOS, Android) and WebAssembly (Web).

---

## 2. Why Zenith is Better Than the Alternatives

### vs. Flutter
- **Zero-Friction Memory (Modern GC):** Unlike Rust's difficult ownership model, Zenith uses a highly optimized, low-pause Garbage Collector (similar to Go). Developers don't have to worry about memory management; the language handles it invisibly while maintaining extremely high performance.
- **Microscopic Web Payload & Fast Load Times:** Flutter's second biggest flaw is that it compiles the entire app and a massive rendering engine (CanvasKit) into one huge file, causing terrible load times. Zenith fixes this by:
  1. Relying on the browser's native DOM (no 5MB rendering engine required).
  2. **Automatic Code Splitting:** Zenith does not compile "all into one." For the Web, the compiler automatically splits the Wasm binary by UI Route/Page. Users only download the exact code for the page they are viewing.
- **Perfect Web SEO:** Because UI components compile to semantic HTML (`<div>`, `<span>`), Zenith apps are perfectly readable by search engines and screen readers.

### vs. Mojo / Python
- Mojo is built for GPU tensor math. Python is built for data research. Neither has a cross-platform UI kit. Zenith natively binds Agentic primitives directly to user-facing UI components.

---

## 3. The Architecture

### Native UI Wrappers (Compile-Time Mapping)
Zenith does not draw its own pixels. It maps UI primitives to the host OS at compile time:
- `Text("Jaypal")` -> Web: `<span>Jaypal</span>`
- `Text("Jaypal")` -> iOS: `UILabel("Jaypal")`
- `Text("Jaypal")` -> Android: `TextView("Jaypal")`

### 4. The Zenith Fusion Syntax
To ensure mass adoption, Zenith does not clone a single language. It fuses the greatest hits of modern programming:
- **Data (Swift/Rust):** We use lightweight `struct` definitions for pure data.
- **Logic (TypeScript):** We use clean `class` definitions with auto-constructors.
- **Functions (Dart/C++):** We use Return-Type-First function definitions.
- **AI (Zenith Original):** The `agentic` keyword natively binds to the LLM engine.

```dart
// 1. Data Structure (Swift/Rust style)
struct User {
    String name;
    Int age;
}

// 2. Logic & AI (Kotlin / Zenith style)
// The class definition IS the constructor. Zero boilerplate.
class Database(String url) {
    
    // AGENTIC function
    agentic String summarize(String text) {
        prompt: "Extract the bullet points: {text}"
    }
}

// 3. Universal UI (Flutter/React style)
UI ChatScreen() {
    String response = await summarize("User's long email...");
    
    return Column(
        Text("AI Summary:", fontWeight: "bold"),
        Text(response)
    );
}
```

---

## 4. Compiler Roadmap (C++)

1. [x] **Lexer (`lexer.cpp`)**: Zero-allocation token parsing using `std::string_view`.
2. [x] **AST (`ast.h`)**: Defined memory structures including `AgenticFunctionNode` and `UIComponentNode`.
3. [ ] **Parser (`parser.cpp`)**: Converts the token stream into the AST.
4. [ ] **Semantic Analyzer**: Type checking and memory safety validation.
5. [ ] **LLVM IR Generator**: Translates the AST into LLVM Intermediate Representation.
6. [ ] **Yoga UI Bridge**: Translates UI nodes into cross-platform Flexbox instructions.

Feature Category	Status	Completed Elements	Uncompleted / Next Steps
Type System Enhancements	🟢 Partial	Core Hindley-Milner engine (

type_inference.hpp
), literal, collection, Option/Result, binary expressions, and SemanticAnalyzer integration.	Lambda parameter inference, full return type inference, bidirectional/context-driven checks.
Yoga Layout Engine	🟡 Partial	Full C++ API layout node wrappers (

yoga_layout.h
 / 

yoga_layout.cpp
), properties configuration, fluent LayoutBuilder API, tree LayoutContext management, and 14 tests passing.	Linking the physical C-based Facebook Yoga library (conditionally compiled via YOGA_AVAILABLE), mapping calculated layout output coordinates to Zenith's terminal/native renderer.
Standard Library	🟢 Partial	Transpilation support for async/await, concurrency primitives (

concurrency.hpp
) including Future/Promise, channels, thread-pool executor, and Erlang-style actors.	Rich widget UI library (advanced components like Image, Video, Scrolling, and form inputs).
Developer Experience	🟢 Partial	JSON-RPC LSP Server (

lsp.cpp
) supporting hover type signatures and diagnostic publishing, and AST pretty-printer formatter (

formatter.cpp
).	Compiler daemon hot-reloads, package manager client CLI/package registry.