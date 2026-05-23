# ⚡ Zenith Language

> **The World's First Native Agentic Systems Language**

Zenith is a general-purpose, high-performance systems language that natively combines **C++ execution speed**, **universal cross-platform UI**, and **native Agentic (AI) primitives** — all at the compiler level. Write once, run everywhere: Desktop, Android, iOS, Web, and WebAssembly.

---

## ✨ What Makes Zenith Unique

| Feature | Zenith | Flutter | React Native | Mojo |
|---------|--------|---------|--------------|------|
| Native AI (`agentic`) keyword | ✅ | ❌ | ❌ | ❌ |
| Transpiles to C++17 | ✅ | ❌ | ❌ | ✅ |
| Cross-platform UI (all 6 targets) | ✅ | ✅ | ⚠️ | ❌ |
| WebAssembly output | ✅ | ⚠️ | ❌ | ❌ |
| Hybrid RC + GC memory | ✅ | ❌ | ❌ | ❌ |
| LSP + completion support | ✅ | ❌ | ❌ | ❌ |
| Built-in package manager | ✅ | ❌ | ✅ | ❌ |
| Compiler daemon / hot reload | ✅ | ✅ | ✅ | ❌ |

---

## 🚀 Quick Start

### Install (from source)
```bash
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang

# Linux / macOS
make

# Windows (MinGW)
build.bat
```

### Hello World
```Zenith
import std.io;

class HelloApp() {
    UI build() {
        return Column(
            Text("Hello from Zenith!", fontWeight: "bold", color: "cyan"),
            Button("Click Me", onClick: handleClick)
        );
    }

    Void handleClick() {
        println("Zenith is alive!");
    }
}

Void main() {
    HelloApp app = HelloApp();
    zenith::runInteractiveLoop(app);
}
```

### Compile & Run
```bash
# Transpile to C++ and run on desktop
zenith hello.zen -target cpp

# Transpile to Web (HTML + JS)
zenith hello.zen -target web

# Transpile to WebAssembly
zenith hello.zen -target wasm
```

---

## 📐 Language Syntax

### Classes & Types
```Zenith
class User(String name, Int age) {
    String greet() {
        return "Hello, " + name;
    }
}

// Type inference — no annotation needed
let x = 42;          // → Int
let msg = "Zenith";  // → String
let nums = [1,2,3];  // → List<Int>
```

### Agentic AI Primitives
```Zenith
import std.io;

class AIAssistant() {
    agentic String summarize(String text) {
        prompt: "Summarize this in bullet points: {text}"
        streaming: true
    }

    agentic String translate(String text, String lang) {
        prompt: "Translate to {lang}: {text}"
    }
}
```

### Async / Concurrency
```Zenith
class DataService() {
    async String fetchUser(String id) {
        String result = await httpGet("https://api.example.com/users/" + id);
        return result;
    }
}
```

### Option & Result Types
```Zenith
Option<Int> safeDivide(Int a, Int b) {
    if (b == 0) { return None; }
    return Some(a / b);
}

Result<String, String> loadFile(String path) {
    // returns Ok(content) or Err(message)
}
```

### UI Components
```Zenith
UI build() {
    return Column(
        Text("Dashboard", fontWeight: "bold", color: "cyan"),
        Row(
            Card(
                Text("Checkbox:"),
                Checkbox("Enable Feature", onChange: handleCheck, checked: "false"),
                padding: 1
            ),
            Card(
                Text("Volume:"),
                Slider(onChange: handleVolume, value: "50", min: "0", max: "100"),
                padding: 1
            )
        ),
        Card(
            Text("Theme:"),
            Toggle("Dark Mode", onChange: handleToggle, isOn: "false"),
            Dropdown("Option 1,Option 2,Option 3", onChange: handlePick, value: "Option 1"),
            padding: 1
        )
    );
}
```

### Agent Orchestration
```Zenith
orchestration MyPipeline {
    mode: "sequential"
    agents: [SummaryAgent, TranslateAgent, FormatAgent]
}
```

---

## 🏗️ Compiler Architecture

```
Source (.zen)
    │
    ▼
┌─────────┐    ┌──────────┐    ┌──────────────────┐
│  Lexer  │───▶│  Parser  │───▶│ Semantic Analyzer │
│(lexer.h)│    │(parser.h)│    │  + Type Inference │
└─────────┘    └──────────┘    └──────────────────┘
                                        │
               ┌────────────────────────┤
               ▼                        ▼                        ▼
        ┌────────────┐         ┌──────────────┐         ┌─────────────┐
        │  C++ Gen   │         │   JS/HTML Gen │         │  WAT Gen    │
        │(codegen.cpp)│        │(js_codegen.cpp)│        │(wasm_codegen)│
        └────────────┘         └──────────────┘         └─────────────┘
               │                        │                        │
               ▼                        ▼                        ▼
         Desktop/Mobile            Web Browser            WebAssembly
      (g++ → native exe)         (.html + JS)          (.wat → .wasm)
```

### Key Components

| File | Purpose |
|------|---------|
| `src/frontend/lexer.cpp` | Zero-allocation tokenizer using `std::string_view` |
| `src/frontend/parser.cpp` | Full recursive-descent parser → AST |
| `src/frontend/semantic.cpp` | Type checker + Hindley-Milner type inference |
| `src/frontend/formatter.cpp` | AST-based code formatter (`zenith format`) |
| `src/backend/codegen.cpp` | C++17 transpiler with Yoga layout integration |
| `src/backend/js_codegen.cpp` | HTML/JavaScript web target generator |
| `src/backend/wasm_codegen.cpp` | WebAssembly Text (WAT) generator |
| `src/lsp/lsp.cpp` | Full JSON-RPC Language Server Protocol |
| `include/zenith/common/` | Runtime: memory, concurrency, UI base |
| `include/zenith/desktop/windows/` | Windows-specific runtime + interactive loop |
| `lib/yoga/` | Facebook Yoga C++ flexbox layout engine |

---

## 🛠️ CLI Reference

### Compiler
```bash
zenith <file.zen> [-target cpp|web|wasm] [-o output]
```

### Project
```bash
zenith create <name|.>          # Scaffold a new project
zenith run <desktop|web|wasm>   # Build & run for platform
zenith format [-w] <file.zen>   # Format source file
```

### Package Manager
```bash
zenith install <git-url>        # Install a package
zenith install                  # Install all from zenith.json
zenith list                     # List installed packages
zenith search [query]           # Search package registry
zenith update [package]         # Update one or all packages
zenith remove <package>         # Uninstall a package
zenith publish                  # Publish your package
```

### Developer Tools
```bash
zenith lsp                      # Start LSP server (stdio, for editors)
zenith daemon start             # Start background compiler daemon
zenith daemon stop              # Stop daemon
zenith daemon status            # Show daemon status + recent log
zenith watch <file.zen>         # Hot-reload watch mode
```

---

## 📦 Package Manager

Zenith has a built-in package manager backed by Git. Packages are declared in `zenith.json`:

```json
{
  "name": "my-zenith-app",
  "version": "1.0.0",
  "dependencies": {
    "zenith-ui": "https://github.com/zenith-lang/zenith-ui.git",
    "zenith-http": "https://github.com/zenith-lang/zenith-http.git"
  }
}
```

Install all dependencies with:
```bash
zenith install
```

**Registry** — available packages:
| Package | Description |
|---------|-------------|
| `zenith-ui` | Core UI component library |
| `zenith-http` | Async HTTP client |
| `zenith-auth` | Authentication & JWT utilities |
| `zenith-db` | SQLite/Postgres database layer |
| `zenith-charts` | Data visualization widgets |
| `zenith-i18n` | Internationalization support |
| `zenith-router` | SPA client-side router |
| `zenith-forms` | Form validation & inputs |
| `zenith-state` | Global state management |
| `zenith-test` | Unit testing framework |

---

## 🔌 LSP Integration (VS Code / Neovim)

The Zenith LSP server provides full editor intelligence:

- **Hover** — type signatures, variable info
- **Diagnostics** — real-time parser & semantic error squiggles
- **Completions** — 35+ keywords, types, stdlib, UI components + live class/method symbols
- **Trigger characters** — `.` `(` `:` `space`

### VS Code Setup (`settings.json`)
```json
{
  "zenith.lsp.command": ["path/to/zenith.exe", "lsp"]
}
```

### Neovim (via `nvim-lspconfig`)
```lua
vim.api.nvim_create_autocmd("FileType", {
  pattern = "zenith",
  callback = function()
    vim.lsp.start({
      name = "zenith",
      cmd = { "zenith", "lsp" },
    })
  end,
})
```

---

## 🧠 Memory Model

Zenith uses a **Hybrid Reference Counting + Garbage Collection** model:

```
Objects marked @managed
       │
       ▼
┌──────────────────┐       ┌──────────────────────┐
│  Ref<T> (RC)     │──────▶│  GcHeap (background) │
│  Weak<T> (weak)  │       │  Tri-color mark+sweep │
│  Zero-cost RAII  │       │  Cycle detection      │
└──────────────────┘       └──────────────────────┘
```

- **Primary:** `Ref<T>` / `Weak<T>` smart pointers manage acyclic objects at zero cost
- **Secondary:** `GcHeap` background thread catches reference cycles every 5s
- **Zero boilerplate:** memory is entirely invisible to the developer

---

## 📊 Feature Status

| Feature | Status | Notes |
|---------|--------|-------|
| **Type System** (Hindley-Milner inference) | 🟢 Complete | Lambda, return, bidirectional inference |
| **Yoga Layout Engine** | 🟢 Complete | Full flexbox, 14 tests pass, terminal renderer |
| **Standard Library** | 🟢 Complete | Async/await, concurrency, 14 UI widgets |
| **Developer Experience** | 🟢 Complete | LSP + completions, daemon, full pkg manager |
| **AI/Agentic** | 🟢 Complete | Streaming, multi-modal, orchestration |
| **Memory Management** | 🟢 Complete | Hybrid RC + GC, background cycle collector |
| **Error Handling** | 🟢 Complete | `Option<T>`, `Result<T,E>`, match expressions |
| **Interop** (C/C++ FFI, JS bridge) | 🔴 Planned | Next milestone |

---

## 🗂️ Project Structure

```
zenith_lang/
├── src/
│   ├── frontend/          # Lexer, Parser, Semantic Analyzer, Formatter
│   ├── backend/           # C++, JS/Web, WASM code generators
│   ├── lsp/               # JSON-RPC Language Server
│   └── main.cpp           # CLI entry point (all subcommands)
├── include/
│   ├── zenith/
│   │   ├── common/        # Runtime: memory, concurrency, UI base
│   │   └── desktop/       # Platform-specific runtime (Windows/Linux/Mac)
│   └── lsp/               # LSP header
├── lib/
│   └── yoga/              # Facebook Yoga flexbox engine (C++)
├── tests/                 # Test suite (.zen source + compiled results)
├── zenith.exe             # Compiled Zenith compiler (Windows)
├── zenith.json            # Package manifest
├── build.bat              # Windows build script
└── Makefile               # Linux/macOS build script
```

---

## 🤝 Contributing

```bash
# Clone and build
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang && build.bat   # or: make

# Run the test suite
zenith tests/test_widgets.zen -target cpp
zenith tests/gallery.zen -target web

# Install a package and verify
zenith install https://github.com/octocat/Spoon-Knife.git
zenith list
```

---

## 📄 License

MIT License — see [LICENSE](LICENSE) for details.

---

<div align="center">
  <strong>Built with ⚡ by the Zenith team</strong><br>
  <em>"Write once. Think once. Ship everywhere."</em>
</div>