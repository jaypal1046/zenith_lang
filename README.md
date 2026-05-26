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
| Built-in SEO Static Pre-rendering | ✅ | ❌ (requires SSR host) | ❌ | ❌ |
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

### 💻 Compiler & Core
```bash
# Transpiles a Zenith source file to a target platform
zenith <file.zen> [-target cpp|web|wasm] [-o <output_file>]
```

### 📁 Project Management
```bash
# Scaffold a new project or library
zenith create <name|.> [--template=app|package]

# Build and run the project for a specific target platform
# Supported platforms: desktop, windows, linux, mac, web, wasm, android, ios
zenith run <platform>

# Format a Zenith source file using the AST-based formatter
zenith format [-w] <file.zen>   # Use -w / --write to edit the file in-place
```

### ⚡ SSR Web Server (Next.js Style)
Zenith features a built-in high-performance Server-Side Rendering (SSR) web server with file-system routing, public static asset support, and hot module reloading.
```bash
# Start a dynamic SSR server
zenith serve <file.zen|directory/> [--port 8080] [--target web|wasm]
```
- **File-System Routing**: If a directory is served, files under `pages/` are auto-mapped to routes (e.g., `pages/index.zen` → `/`, `pages/about.zen` → `/about`, `pages/blog/post.zen` → `/blog/post`). If a single file is served, it routes `/`.
- **Static Assets**: Automatically serves any static files in the `public/` directory (e.g., `public/logo.png` is served at `/logo.png`).
- **Live HMR**: Automatically injects a lightweight Hot Module Replacement client script that listens via Server-Sent Events (SSE) at `/__zenith_hmr` and instantly reloads the browser when any page source code changes.

### 📦 Package Manager & FFI Bridges
Zenith's package manager handles native Zenith dependencies, npm/CDN frontend integrations, and low-level FFI bridges.
```bash
# Dependency Management
zenith install <git-url>        # Clone a git package into lib/ and register in zenith.json
zenith install                  # Install all missing dependencies from zenith.json
zenith list                     # List all installed packages and registered details
zenith search [query]           # Search the curated Zenith package registry
zenith update [package]         # Pull/update one or all libraries from git remotes
zenith remove <package>         # Uninstall and delete library + remove from zenith.json
zenith publish                  # Print instructions to register your package

# npm CDN Bridge
# Installs an npm package as a CDN import, creating a .zen wrapper stub with `foreign "js"` bindings
zenith add <npm-package-name>   # e.g., zenith add chart.js

# Low-Level FFI Bridge
# Scaffolds native Rust or Dart compiled library stubs and FFI declarations
zenith bridge <dart|rust> <package_name> "<exported_signatures>"
```
- **npm CDN Bridge**: Resolves and maps the library to a jsDelivr CDN source in `zenith.json`.
- **Low-Level FFI**: Scaffolds FFI wrapper projects for Rust and Dart to import arbitrary packages from **Crates.io** and **Pub.dev**. Both support compiling to **native shared libraries** (`.dll`, `.so`, `.dylib` on desktop loaded via `import native`) as well as **WASM targets** (`.wasm` loaded via CDN on web).

### 🔌 Developer Tools
```bash
# Language Server Protocol (LSP)
zenith lsp                      # Starts a JSON-RPC 2.0 LSP server on stdin/stdout for IDEs

# Background Compiler Daemon
zenith daemon start [-d <dir>]  # Start background daemon file watcher & semantic parser cache
zenith daemon stop              # Stop the daemon
zenith daemon status            # View PID, log location, and recent worker log output

# Hot-Reload Watch Mode
zenith watch <file.zen> [-target cpp|web|wasm]
```
- **Hot-Reload Watch**: Watches the source directory for changes. On save, it compiles, compiles the generated native target, terminates the running process, and starts a fresh instance automatically.

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

### 🦀 Crates.io & 🎯 Pub.dev Integration

Zenith supports integrating arbitrary, non-UI backend and utility libraries directly from **Crates.io** (Rust) or **Pub.dev** (Dart). 

Using the `zenith bridge` command, Zenith automatically scaffolds a local target environment, downloads dependencies via the native package managers (`cargo` / `pub`), compiles the packages into shared native binaries or WASM payloads, and connects them directly back to your Zenith application via compiled bindings:

#### Example: Integrating Rust's `uuid` crate from Crates.io
1. **Scaffold the Bridge**:
   ```bash
   zenith bridge rust uuid "String generate_uuid();"
   ```
   This scaffolds a Cargo package inside `lib/uuid/` pre-configured to fetch `uuid` from Crates.io, exports a C FFI wrapper function in `lib/uuid/src/lib.rs`, and generates the Zenith FFI mapping file in `lib/uuid/main.zen`.

2. **Build the Target (Native or WASM)**:
   - **For Desktop (Native Shared Library)**:
     ```bash
     cd lib/uuid && cargo build --release
     ```
     This compiles the Rust crate to a shared library (e.g., `bridge.dll` on Windows, or `bridge.so` / `bridge.dylib` on POSIX).
   - **For Web (WebAssembly)**:
     ```bash
     cd lib/uuid && cargo build --target wasm32-unknown-unknown --release
     ```
     This compiles to `bridge.wasm` for web browser environments.

3. **Import and Connect**:
   Import the bridge inside your Zenith code:
   ```Zenith
   import "uuid/main.zen";
   
   Void main() {
       let my_uuid = generate_uuid();
       println("Generated UUID from Rust crate: " + my_uuid);
   }
   ```

#### Example: Integrating Dart's `crypto` package from Pub.dev
1. **Scaffold the Bridge**:
   ```bash
   zenith bridge dart crypto "String sha256(String input);"
   ```
2. **Build the Target (Native or WASM)**:
   - **For Desktop (Native Shared Library)**:
     ```bash
     cd lib/crypto && dart pub get && dart compile shared-library main.dart -o bridge.dll # or bridge.so / bridge.dylib
     ```
     This compiles the Dart package to a native shared library for desktop platforms.
   - **For Web (WebAssembly)**:
     ```bash
     cd lib/crypto && dart pub get && dart compile wasm main.dart
     ```
     This compiles to `bridge.wasm` for web environments.
3. **Import and Connect**:
   Import the bridge inside your Zenith code:
   ```Zenith
   import "crypto/main.zen";
   ```

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
| **Web & WASM SEO Pre-rendering** | 🟢 Complete | Statically compiles declarative layout trees to static HTML for SEO crawlability |
| **Developer Experience** | 🟢 Complete | LSP + completions, daemon, full pkg manager |
| **AI/Agentic** | 🟢 Complete | Streaming, multi-modal, orchestration |
| **Memory Management** | 🟢 Complete | Hybrid RC + GC, background cycle collector |
| **Error Handling** | 🟢 Complete | `Option<T>`, `Result<T,E>`, match expressions |
| **Interop** (C/C++ FFI, JS/Python bridges) | 🟢 Complete | Native FFI, popen Python bridge, WASM JS Proxy imports |

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