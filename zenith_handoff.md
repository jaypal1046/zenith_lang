# Zenith Language — Complete Project Handoff

> Everything currently in the project, what each piece does, and how it all connects.

---

## 📍 Big Picture

Zenith is a **compiler written in C++** that reads `.zen` source files and transpiles them to three targets:

| Target | Output | How to run |
|--------|--------|------------|
| `cpp` | `.cpp` → compiled to native `.exe` | `zenith file.zen -target cpp` |
| `web` | `.html` (semantic HTML + JS) | `zenith file.zen -target web` |
| `wasm` | `.wat` + HTML loader | `zenith file.zen -target wasm` |

The compiler binary itself (`zenith.exe`) is also a **CLI tool** that wraps a package manager, LSP server, formatter, compiler daemon, and hot-reload watch mode — all in one executable.

---

## 🗂️ Full Directory Map

```
zenith_lang/
│
├── src/                    ← All C++ compiler source code
│   ├── main.cpp            ← Entry point + ALL CLI subcommand routing
│   ├── frontend/           ← Phase 1-3: tokenize → parse → analyze
│   │   ├── lexer.cpp
│   │   ├── parser.cpp
│   │   ├── semantic.cpp
│   │   └── formatter.cpp
│   ├── backend/            ← Phase 4: code generation
│   │   ├── codegen.cpp     ← C++17 transpiler
│   │   ├── js_codegen.cpp  ← HTML/Web transpiler
│   │   └── wasm_codegen.cpp← WebAssembly Text (WAT) transpiler
│   ├── lsp/
│   │   └── lsp.cpp         ← Full JSON-RPC Language Server
│   └── zenith/ui/
│       └── yoga_layout.cpp ← Yoga flexbox wrapper implementation
│
├── include/                ← All C++ headers (compiler + runtime)
│   ├── zenith_runtime.h    ← Master include (used by generated C++ apps)
│   ├── ast/
│   │   └── ast.h           ← All 40+ AST node definitions
│   ├── frontend/
│   │   ├── lexer.h
│   │   ├── parser.h
│   │   ├── semantic.h
│   │   └── formatter.h
│   ├── backend/
│   │   ├── codegen.h
│   │   ├── js_codegen.h
│   │   └── wasm_codegen.h
│   ├── lsp/
│   │   └── lsp.h
│   └── zenith/             ← Runtime library (linked by generated apps)
│       ├── common/
│       │   └── zenith_common.h     ← Core runtime: UI engine, HTTP, rendering
│       ├── std/
│       │   └── concurrency.hpp     ← Future, Promise, Channel, Actors
│       ├── memory/
│       │   └── zenith_memory.h     ← RC smart pointers + GC heap
│       ├── ui/
│       │   └── yoga_layout.h       ← Yoga layout wrapper header
│       ├── desktop/
│       │   ├── windows/            ← Windows runtime + interactive loop
│       │   ├── linux/              ← Linux runtime
│       │   ├── mac/                ← macOS runtime
│       │   └── posix/              ← Shared POSIX runtime
│       ├── android/                ← Android NDK runtime
│       ├── ios/                    ← iOS runtime
│       └── web/                    ← Web/WASM runtime
│
├── lib/                    ← External dependencies
│   ├── yoga/               ← Facebook Yoga flexbox engine (C++ source)
│   └── Spoon-Knife/        ← Example installed package
│
├── tests/                  ← Test suite (.zen sources + manual .cpp harnesses)
│   ├── *.zen               ← Zenith source test programs
│   ├── test_yoga_layout.cpp← Manual C++ Yoga layout test suite (14 tests)
│   ├── test_main.cpp       ← Runtime integration tests
│   ├── test_memory.cpp     ← GC/RC memory tests
│   └── test_concurrency.cpp← Concurrency primitives tests
│
├── zenith.exe              ← The compiled Zenith compiler (Windows)
├── zenith.json             ← Package manifest for this project
├── build.bat               ← Windows: build zenith.exe from source
├── build_android.bat       ← Android NDK build helper
├── build_ios.sh            ← iOS cross-compile helper
├── Makefile                ← Linux/macOS: build zenith compiler
└── .gitignore
```

---

## ⚙️ The Compilation Pipeline

Every `.zen` file goes through 4 sequential phases:

```
.zen file
    │
    ▼ Phase 1 ─────────────────────────────────────────
    Lexer (lexer.cpp)
    • Reads raw text, produces a flat list of Token structs
    • Token has: type (enum TokenType), value (string), line, column
    • Uses std::string_view — zero allocation, very fast
    • Handles: keywords, identifiers, strings, numbers, operators, agentic blocks
    │
    ▼ Phase 2 ─────────────────────────────────────────
    Parser (parser.cpp)
    • Recursive descent parser — consumes token stream
    • Builds a heap-allocated AST (Abstract Syntax Tree)
    • Every construct maps to a node in ast.h (see AST section below)
    • Entry point: parser.parseProgram() → ProgramNode*
    │
    ▼ Phase 3 ─────────────────────────────────────────
    Semantic Analyzer (semantic.cpp)
    • Walks the AST, performs type checking
    • Runs Hindley-Milner type inference engine
    • Validates: undeclared variables, type mismatches, return types
    • Enriches AST nodes with inferred type_hint fields
    │
    ▼ Phase 4 ─────────────────────────────────────────
    Code Generator (codegen.cpp / js_codegen.cpp / wasm_codegen.cpp)
    • Walks the type-enriched AST
    • Emits target language text (C++17 / HTML+JS / WAT)
    • Writes output file (e.g., file.cpp, file.html, file.wat)
    │
    ▼ (C++ target only)
    g++ compiles the emitted .cpp → native .exe
```

---

## 📁 File-by-File Breakdown

### `src/main.cpp` — The CLI Brain (2600+ lines)
The single entry point for everything. Contains:

**CLI Routing** — dispatches on `argv[1]`:
| Subcommand | What it does |
|------------|-------------|
| `create <name>` | Scaffolds full project: `lib/`, `desktop/`, `android/`, `ios/`, `web/`, `mac/`, `linux/` with build scripts for all platforms |
| `run <platform>` | Reads platform build script and executes it |
| `format [-w] <file>` | Lexes → Parses → Formatter → prints or writes in-place |
| `install <url>` | `git clone <url> lib/<name>` + writes to zenith.json |
| `install` | Reads zenith.json, `git clone` any missing packages |
| `list` | Shows packages in zenith.json + scans lib/ directory |
| `search [query]` | Fuzzy searches 11-package curated registry |
| `update [pkg]` | `git pull --ff-only` in lib/<pkg> |
| `remove <pkg>` | `rm -rf lib/<pkg>` + cleans zenith.json entry |
| `publish` | Reads git remote, prints install instructions |
| `lsp` | Starts JSON-RPC LSP server on stdin/stdout |
| `daemon start` | Starts background compiler daemon (PID file + log) |
| `daemon stop` | Kills daemon by PID file |
| `daemon status` | Shows running/stopped + last 10 log lines |
| `_daemon_worker` | Internal: file-watcher loop (re-lexes/parses/analyzes on .zen change) |
| `watch <file>` | Hot-reload: watches .zen files, recompiles + restarts exe |
| `<file.zen>` | Default: runs full compile pipeline |

**`compileProject()` function** — the core compile function called by both the default path and watch mode. Handles import resolution (recursive, deduplication), runs all 4 phases, writes output.

**`resolveImports()`** — recursively loads imported `.zen` files, inlines their ASTs into the main ProgramNode before compilation.

---

### `include/ast/ast.h` — The AST Node Hierarchy
Defines every node type as a C++ struct inheriting from `ASTNode`. Key nodes:

| Node | What it represents |
|------|--------------------|
| `ProgramNode` | Root — list of top-level statements |
| `ClassDeclNode` | `class Foo(args) { ... }` — has fields, methods, primary constructor args |
| `InterfaceDeclNode` | `interface IFoo { ... }` — virtual method signatures |
| `FunctionNode` | `ReturnType name(params) { body }` — free function |
| `VarDeclNode` | `Type name = expr` — variable declaration |
| `IdentifierNode` | A bare variable reference — carries `type_hint` after semantic pass |
| `LiteralNode` | String/Int/Float/Bool literal value |
| `BinaryExprNode` | `a + b`, `a == b`, `a && b`, etc. |
| `FunctionCallNode` | `foo(args)` |
| `MethodCallNode` | `obj.method(args)` |
| `PropertyAccessNode` | `obj.field` |
| `IfStmtNode` | `if (cond) { } else { }` |
| `WhileStmtNode` | `while (cond) { }` |
| `ReturnStmtNode` | `return expr` |
| `SetStateStmtNode` | `setState { ... }` — reactive state update |
| `UIComponentNode` | `Button(...)`, `Text(...)`, etc. — UI widget call |
| `AgenticFunctionNode` | `agentic Ret name(params) { prompt: "..." }` |
| `AgentOrchestrationNode` | `orchestration Name { mode: ... agents: [...] }` |
| `LambdaNode` | `(x, y) => expr` |
| `MatchExprNode` | `match expr { pattern => result }` |
| `AwaitExprNode` | `await expr` |
| `TryExprNode` | `try expr` |
| `OptionExprNode` | `Some(val)` / `None` |
| `ResultExprNode` | `Ok(val)` / `Err(val)` |
| `ListLiteralNode` | `[a, b, c]` |
| `MapLiteralNode` | `{k: v, ...}` |
| `ImportNode` | `import std.io` / `import "file.zen"` |
| `TypeNode` | Wraps a type name string (e.g. `"String"`, `"List<Int>"`) |

Every node has: `int line`, `int column` (for LSP hover + error reporting), and `std::string type_hint` (filled in by semantic pass).

---

### `src/frontend/lexer.cpp` + `include/frontend/lexer.h`
- Defines `enum class TokenType` — ~60 token types (KEYWORD_CLASS, OP_PLUS, LITERAL_STRING, etc.)
- `Lexer::tokenize()` — single pass over source string, returns `vector<Token>`
- Uses `std::string_view` for zero-copy substrings
- Handles escaped strings, multi-char operators (`==`, `!=`, `->`, `=>`, `...`)
- Recognizes all Zenith keywords: `class`, `interface`, `agentic`, `orchestration`, `setState`, `async`, `await`, `match`, `import`, `UI`, etc.

---

### `src/frontend/parser.cpp` + `include/frontend/parser.h`
- Full recursive-descent parser
- `Parser::parseProgram()` → `unique_ptr<ProgramNode>` — the top-level entry
- Each grammar rule is its own method: `parseClass()`, `parseFunction()`, `parseIf()`, `parseExpression()`, `parsePrimary()`, etc.
- Handles operator precedence via cascading expression parsing (comparison → additive → multiplicative → unary → primary)
- Parses UI component syntax: `Button("text", onClick: handler)` — positional + keyword args
- Parses agentic blocks: `prompt:`, `streaming:`, `multimodal:` attributes
- Parses orchestration blocks: `mode:`, `agents:` lists
- **Error recovery**: reports parser errors to `cerr` with `line` number for LSP diagnostic parsing

---

### `src/frontend/semantic.cpp` + `include/frontend/semantic.h`
- `SemanticAnalyzer::analyze(ProgramNode*)` — walks every node
- **Symbol table**: `unordered_map<string, string>` mapping name → type_name, scoped per function/class
- **Type inference engine** (Hindley-Milner inspired):
  - Literal inference: `42` → `Int`, `"x"` → `String`, `[1,2]` → `List<Int>`
  - Binary expression inference: `Int + Float` → `Float`, `String + String` → `String`
  - Option/Result inference: `Some(42)` → `Option<Int>`, `Ok("x")` → `Result<String,Error>`
  - Lambda inference: deduces parameter types from call-site context
  - Return type inference: unifies all return paths in a function body
  - Bidirectional: propagates type context downward into collection literals
- Writes inferred types back to `node->type_hint` on `IdentifierNode` / `VarDeclNode`
- Reports errors to `cerr` in the format `[Semantic Error] file:line:col: message` (parsed by LSP)

---

### `src/frontend/formatter.cpp` + `include/frontend/formatter.h`
- `Formatter::format(ProgramNode*)` → `string`
- AST-based pretty printer — walks nodes and re-emits clean source
- Handles: consistent indentation (4 spaces), spacing around operators, brace placement
- Used by `zenith format [-w] <file>` — `-w` writes back in place
- Does NOT parse — it formats from the already-parsed AST (so syntactically invalid files can't be formatted)

---

### `src/backend/codegen.cpp` — C++17 Transpiler
The most complex backend. `CodeGenerator::generate(ProgramNode*)` → `string` of C++ code.

**Class generation:**
- Class fields → C++ member variables
- Primary constructor args → constructor parameters
- Methods → C++ methods
- `setState { ... }` → direct assignment (reactive semantics via re-render calls)
- `build()` method → returns `zenith::UIElement`
- `triggerCallback(name, val)` → dispatcher: `if (name == "handleX") { this->handleX(val); }` — dispatches by type (bool/int/float/string/void)

**UI component generation:**
- `Button("text", onClick: handler)` → `zenith::make_button("text").onClick("handler")...`
- `TextField(placeholder: "x", onChange: handler)` → `zenith::make_textfield("x").onChange(...)`
- `Slider(value: "50", min: "0", max: "100")` → attrs-only widget (no children)
- `Checkbox("label", onChange: h)`, `Toggle("label", isOn: "false")`, `Dropdown("opts", value: "x")`
- Layout: `Column(...)` / `Row(...)` / `Card(...)` / `Container(...)` → nested make calls
- **`is_no_first_param` logic**: Slider has no label/children argument — codegen skips the first `make_children()` parameter

**Function generation:**
- `async` functions → wrapped in `zenith::stdlib::Future<T>`
- `agentic` functions → `LLMClient` call with prompt template + streaming/multimodal flags
- `orchestration` → `std::async` pool with sequential/parallel modes

**Memory management injection:**
- Emits `zenith::mem::GcHeap::instance().start_background_gc(5000)` in `main()`
- Emits `stop_background_gc()` + `collect()` at shutdown
- Emits `#ifdef ZENITH_GC_STATS` stats block

**Import handling:**
- `import std.io` → injects `println`/`print` helper includes
- File imports are resolved before codegen by `resolveImports()` in main.cpp

---

### `src/backend/js_codegen.cpp` — Web/HTML Transpiler
`JsCodeGenerator::generate(ProgramNode*)` → HTML file string.

- Emits a complete `.html` file with embedded `<script>` JavaScript
- Maps Zenith classes → JavaScript classes with `build()` → DOM element creation
- UI components → `document.createElement()` calls with CSS classes
- `setState` → DOM update + re-render
- `Button onClick` → `addEventListener('click', ...)`
- `TextField onChange` → `addEventListener('input', ...)`
- `agentic` functions → `fetch()` calls to LLM API endpoints
- Styles: emits an inline `<style>` block with Zenith's design system (dark mode, gradients, typography)

---

### `src/backend/wasm_codegen.cpp` — WebAssembly Transpiler
`WasmCodeGenerator::generate(ProgramNode*)` → `.wat` (WebAssembly Text Format) file.

- Also emits a companion `_wasm.html` loader file
- Maps Zenith types → WASM types: `Int` → `i32`, `Float` → `f64`, `String` → memory address (`i32`)
- Functions → WASM `func` blocks with param/result types
- Simple arithmetic and control flow (if/while) → WASM opcodes
- String handling → linear memory with offset tracking
- UI `build()` → calls JS host imports (via WASM imports) for DOM manipulation
- The HTML loader uses `WebAssembly.instantiateStreaming()` to load the `.wasm` binary

---

### `src/lsp/lsp.cpp` — Language Server Protocol
A full JSON-RPC 2.0 LSP server that runs on stdin/stdout.

**Internal JSON parser** — hand-written lightweight JSON parser (`JsonParser` class) — no external dependencies. Parses: objects, arrays, strings, numbers, booleans, null.

**AST cache** — `unordered_map<uri, ProgramNode*>` — stores the last-parsed AST per open file for hover queries.

**Handlers implemented:**

| LSP Method | What it does |
|------------|-------------|
| `initialize` | Responds with capabilities: hover + completion + textDocumentSync |
| `initialized` | No-op acknowledgement |
| `textDocument/didOpen` | Parses file, runs semantic analysis, publishes diagnostics |
| `textDocument/didChange` | Re-parses on every keystroke, publishes updated diagnostics |
| `textDocument/didSave` | Re-runs diagnostics on save |
| `textDocument/hover` | Walks AST cache to find node at cursor position, returns type signature |
| `textDocument/completion` | Returns 35+ completions: keywords, types, stdlib, UI widgets + live AST symbols |
| `shutdown` / `exit` | Clean shutdown |

**`publishDiagnostics()`** — redirects `cerr` to a string buffer, runs lex + parse + semantic, captures any error output, parses `[Semantic Error] file:line:col:` and `Parser Error: ... at line N` format, converts to LSP `Diagnostic` JSON objects.

**`handleHover()`** — uses `findNodeAt()` to walk AST nodes by line/column, returns formatted type info:
- `IdentifierNode` → `(variable) name: Type`
- `FunctionNode` → `(function) name(param: Type) -> ReturnType`
- `ClassDeclNode` → `class Name`
- `VarDeclNode` → `(variable) name: Type`

**`handleCompletion()`** — returns `CompletionList` with:
- 14 keywords (`class`, `interface`, `if`, `setState`, `await`, `match`, ...)
- 10 built-in types (`String`, `Int`, `Float`, `Bool`, `Option`, `Result`, ...)
- 5 stdlib functions (`println`, `print`, `httpGet`, `httpPost`, `gcStats`)
- 14 UI components (`Column`, `Row`, `Button`, `Checkbox`, `Slider`, `Toggle`, `Dropdown`, ...)
- Live symbols from AST cache (class names, method names, function names, var names)

---

### `include/zenith/common/zenith_common.h` — Core Runtime (~50KB)
The massive single-header runtime that every generated C++ app includes.

**Sections inside:**
- `zenith::UIElement` struct — the in-memory UI tree node (type, attributes map, text_content, children vector, computed layout coords from Yoga)
- `zenith::make_button()`, `make_textfield()`, `make_checkbox()`, `make_slider()`, `make_toggle()`, `make_dropdown()`, `make_children()` — fluent builder functions for UI elements
- `zenith::render()` — walks the UIElement tree, calls Yoga layout calculation, then renders to terminal with ASCII box-drawing characters + ANSI color codes
- `zenith::renderElement()` — recursive terminal renderer with visual widget representation:
  - Buttons: `[ Label ]`
  - Checkboxes: `[ ] label` or `[X] label`
  - Sliders: `[───────█──────]`
  - Toggles: `(• ) OFF` or `( •) ON`
  - Dropdowns: `┌──────────┐ │ [ opt ▼ ] │`
- `zenith::httpGet()`, `zenith::httpPost()` — HTTP client (WinHTTP on Windows, `curl` subprocess on POSIX)
- `zenith::println()`, `zenith::print()` — stdlib output functions
- `zenith::LLMClient` — LLM inference client (streams tokens from local/remote API)
- `zenith::get_keyboard_char()` — cross-platform single-keypress reader

---

### `include/zenith/desktop/windows/zenith_windows.h` — Windows Interactive Loop
- `zenith::runInteractiveLoop(App& app)` — the main event loop for desktop apps
- Calls `app.build()` → gets UIElement tree → renders Yoga layout → renders terminal UI
- Shows interactive control panel menu listing all interactive widgets
- **Handles all 6 widget types:**
  - `Button` → calls `app.triggerCallback("handlerName")`
  - `TextField` → prompts for input → `app.triggerCallback("handler", value)`
  - `Checkbox` → toggles checked state → `app.triggerCallback("handler", "true"/"false")`
  - `Slider` → prompts for value → `app.triggerCallback("handler", "50")`
  - `Toggle` → flips isOn → `app.triggerCallback("handler", "true"/"false")`
  - `Dropdown` → prompts for selection → `app.triggerCallback("handler", "Option X")`
- After each interaction, calls `app.build()` again → re-renders updated UI

---

### `include/zenith/std/concurrency.hpp` — Concurrency Primitives
- `zenith::stdlib::Future<T>` — represents an async result (backed by `std::future<T>`)
- `zenith::stdlib::Promise<T>` — write end of a Future
- `zenith::stdlib::Channel<T>` — Go-style message-passing channel (thread-safe queue with `send()`/`recv()`)
- `zenith::stdlib::AsyncTaskExecutor` — thread pool (configurable worker count, task queue, `submit()` lambda)
- `zenith::stdlib::Actor<T>` — Erlang-style actor (has its own mailbox Channel + dedicated thread processing messages)
- `zenith::stdlib::Option<T>` — `Some(val)` / `None` — null-safe wrapper
- `zenith::stdlib::Result<T, E>` — `Ok(val)` / `Err(err)` — error-propagation wrapper

---

### `include/zenith/memory/zenith_memory.h` — Memory Management
- `zenith::mem::Ref<T>` — reference-counted smart pointer (like `shared_ptr` but integrated with GcHeap)
- `zenith::mem::Weak<T>` — weak reference (doesn't keep alive, breaks cycles)
- `zenith::mem::Managed` — base class for GC-tracked objects (annotated with `@managed` in .zen)
- `zenith::mem::GcHeap` — singleton background garbage collector
  - `start_background_gc(ms)` — starts a background thread that wakes every N ms
  - `collect()` — manual tri-color mark-and-sweep cycle
  - `stop_background_gc()` — orderly shutdown
  - `gcStatsString()` — returns human-readable GC stats string

---

### `include/zenith/ui/yoga_layout.h` + `src/zenith/ui/yoga_layout.cpp` — Layout Engine
Wraps Facebook's Yoga flexbox library in a clean C++ API.

- `zenith::LayoutNode` — wraps a `YGNodeRef` (Yoga node handle)
  - Fluent setters: `setWidth()`, `setHeight()`, `setFlexDirection()`, `setFlexGrow()`, `setPadding()`, `setMargin()`, `setAlignItems()`, `setJustifyContent()`, `setMinWidth()`, `setMaxWidth()`, `setPosition()`, etc.
  - `addChild(LayoutNode)`, `calculateLayout(width, height)` — runs Yoga algorithm
  - `getLeft()`, `getTop()`, `getWidth()`, `getHeight()` — get computed pixel positions
- `zenith::LayoutBuilder` — fluent builder API:
  ```cpp
  LayoutBuilder().column().width(400).padding(8)
      .child(LayoutBuilder().row().height(40))
      .build()
  ```
- `zenith::LayoutContext` — manages a tree of LayoutNodes, maps them to UIElement children, propagates Yoga-calculated coords back into the `UIElement` tree before rendering
- `lib/yoga/` — the actual Facebook Yoga C++ engine source (~30 files, compiled alongside the app)

---

### `lib/yoga/` — Facebook Yoga Engine
The physical Yoga library source code. Compiled with `-DYOGA_AVAILABLE` flag.
Key files: `Yoga.cpp`, `YGNode.cpp`, `YGLayout.cpp`, `YGStyle.cpp`, `YGConfig.cpp`, `event/event.cpp`, `internal/experiments.cpp`.

Used by: `yoga_layout.cpp` when `YOGA_AVAILABLE` is defined at compile time.

---

## 🧪 Test Suite (`tests/`)

| File | What it tests |
|------|--------------|
| `test_widgets.zen` | All 6 UI widgets: Checkbox, Slider, Toggle, Dropdown, Button, TextField |
| `test_agentic_features.zen` | Agentic functions, streaming, orchestration |
| `test_async.zen` | async/await, Future/Promise |
| `test_memory.zen` | RC + GC, @managed objects, Weak references |
| `test_advanced_inference.zen` | Complex Hindley-Milner inference cases |
| `type_inference_test.zen` | Bidirectional inference, Option/Result inference |
| `type_inference_simple.zen` | Basic literal and binary expression inference |
| `simple_inference.zen` | Minimal inference smoke test |
| `gallery.zen` | Multi-widget UI gallery (demo app) |
| `main.zen` | Multi-platform demo app (imports lib/ platform modules) |
| `website.zen` | Web target demo |
| `helper.zen` | Import helper for main.zen |
| `test_yoga_layout.cpp` | **14 C++ unit tests** for Yoga layout wrapper (LayoutNode, LayoutBuilder, LayoutContext) |
| `test_main.cpp` | Runtime integration — all stdlib functions |
| `test_memory.cpp` | Ref/Weak/GcHeap unit tests |
| `test_concurrency.cpp` | Future/Promise/Channel/Actor unit tests |

**How to run tests:**
```bash
# Transpile a .zen test
zenith tests/test_widgets.zen -target cpp

# Compile generated C++ (with Yoga)
g++ -O3 -std=c++17 -DYOGA_AVAILABLE tests/test_widgets.cpp \
    src/zenith/ui/yoga_layout.cpp lib/yoga/*.cpp lib/yoga/event/*.cpp lib/yoga/internal/*.cpp \
    -I include -I lib -o tests/test_widgets_app.exe -lwinhttp

# Run manual C++ tests (Yoga)
g++ -O3 -std=c++17 -DYOGA_AVAILABLE tests/test_yoga_layout.cpp \
    src/zenith/ui/yoga_layout.cpp lib/yoga/*.cpp ... -I include -o tests/yoga_test.exe
```

---

## 🔗 How Everything Connects — Data Flow Example

**Source: `tests/test_widgets.zen`**

```
Zenith source → Lexer → 278 tokens
                 ↓
              Parser → AST: 1 ClassDeclNode (WidgetDemoApp)
                         ├── 4 VarDeclNode (fields)
                         ├── 1 FunctionNode (build) → 4 UIComponentNode children
                         └── 4 FunctionNode (handleCheckbox, handleSlider, handleToggle, handleDropdown)
                 ↓
        SemanticAnalyzer → type_hint populated on all nodes
                 ↓
         CodeGenerator → test_widgets.cpp (89 lines of C++17)
                 ↓
              g++  → test_widgets_app.exe
                 ↓
          zenith_windows.h → runInteractiveLoop()
                 ↓
         Yoga layout engine → computes x,y,w,h for each UIElement
                 ↓
         Terminal renderer → full ASCII UI with box-drawing characters
```

---

## 📦 Package System — How it Works

**zenith.json** — the manifest:
```json
{
  "dependencies": {
    "Spoon-Knife": "https://github.com/octocat/Spoon-Knife.git"
  }
}
```

**`zenith install <url>`:**
1. Extracts package name from last path segment of URL
2. `git clone <url> lib/<name>`
3. Opens `zenith.json`, appends the entry to `"dependencies"` object

**`zenith install` (no args):**
1. Reads `zenith.json`, regex-extracts all `"name": "url"` pairs
2. For each: if `lib/<name>` doesn't exist → `git clone <url> lib/<name>`

**In a Zenith app:**
```dart
import "Spoon-Knife/main.zen";   // imports from lib/Spoon-Knife/main.zen
```
The import resolver in `resolveImports()` (main.cpp) handles this.

---

## 🔌 LSP Integration — How it Works

The LSP runs as a subprocess with stdin/stdout JSON-RPC:

```
VS Code / Neovim
    │  Content-Length: 123\r\n\r\n{json...}
    ▼
zenith.exe lsp        ← reads from stdin in a loop
    │
    ├─ didOpen/didChange → lexes+parses+semantics the file content
    │                    → publishDiagnostics (squiggles in editor)
    │
    ├─ hover at line:col → walks AST cache → returns type info
    │
    └─ completion       → returns 35+ items + live AST symbols
    │
    ▼  Content-Length: 456\r\n\r\n{json...}
VS Code / Neovim      ← reads from stdout
```

The LSP needs to be configured in the editor. It accepts any file opened as type `zenith`.

---

## 🏗️ How to Build the Compiler

### Windows
```bat
build.bat
```
Which runs:
```bat
g++ -O3 -std=c++17 src/main.cpp src/frontend/lexer.cpp src/frontend/parser.cpp ^
    src/frontend/semantic.cpp src/frontend/formatter.cpp src/lsp/lsp.cpp ^
    src/backend/codegen.cpp src/backend/js_codegen.cpp src/backend/wasm_codegen.cpp ^
    -I include -o zenith.exe
```

### Linux / macOS
```bash
make
```

The resulting `zenith.exe` / `zenith` is the full compiler + CLI tool.

---

## 🚧 What's NOT Done (Next Milestone)

### Interop (0% complete)
- **C/C++ FFI** — call C functions from Zenith, expose Zenith functions to C
- **JavaScript bridge** — call JS from Zenith WASM, call Zenith from JS
- **Python bridge** — embed Zenith in Python / call Python from Zenith

**Design decision needed:** whether to use `libffi`, direct `extern "C"` codegen, or WASM import/export tables.

### Nice-to-haves
- **LLVM backend** — currently transpiles to C++ which g++ compiles; a direct LLVM IR backend would remove the g++ dependency
- **VS Code extension** — `.zen` syntax highlighting grammar + LSP auto-configuration
- **Official package registry** — right now `zenith search` returns a hardcoded list in main.cpp; a real registry at `zenith-lang/registry` would be ideal
- **Rich widget UI library** for Web target — current web codegen generates HTML but doesn't produce the full interactive widget equivalents (Slider/Toggle/Dropdown are placeholder elements)

---

## 🎯 Quick Reference Card

```bash
# Build the compiler
build.bat

# Compile a .zen file (all 3 targets)
zenith app.zen -target cpp
zenith app.zen -target web
zenith app.zen -target wasm

# Development workflow
zenith daemon start           # start background compiler
zenith watch app.zen          # hot-reload loop

# Package management
zenith search                 # browse all 11 packages
zenith install <url>          # install a package
zenith list                   # see what's installed
zenith update                 # update all packages

# Code quality
zenith format -w app.zen      # format in place
zenith lsp                    # start LSP for editor

# Project
zenith create my-app          # scaffold new project
zenith run desktop            # build + run desktop
zenith run web                # build + open web
```
