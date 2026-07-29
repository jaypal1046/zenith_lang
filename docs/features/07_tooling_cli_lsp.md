# Zenith Feature Audit: Tooling, CLI, LSP & Development Roadmap

## 1. Executive Overview
Zenith developer experience consists of a single binary CLI (`zenith.exe`), a Language Server Protocol implementation (`src/lsp/lsp.cpp`), a pre-built VS Code VSIX extension, and execution roadmap documentation aligned with the game SDK pivot ([action_plan.md](file:///c:/Jay/_Plugin/zenith_lang/plan/action_plan.md), [game_sdk_pivot.md](file:///c:/Jay/_Plugin/zenith_lang/plan/game_sdk_pivot.md)).

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **Single Binary CLI Commands** ([main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp)):
  - `zenith create <name> --template=game` - Project template generator.
  - `zenith serve <file.zen>` - Dev server with live preview & hot reload asset watcher.
  - `zenith build <file.zen> --target=cpp|wasm|js` - Build compiler invocation.
  - `zenith assets import|list` - Asset metadata sidecar generator & database manager.
  - `zenith lsp` - Language Server Protocol daemon launcher.
- **Standalone Runner** ([runner.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/runner.cpp)):
  - Fast execution test runner binary wrapper.
- **Language Server Protocol (LSP)** ([lsp.h](file:///c:/Jay/_Plugin/zenith_lang/include/lsp/lsp.h), [lsp.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/lsp/lsp.cpp)):
  - Stdio JSON-RPC protocol transport.
  - Real-time syntax diagnostics, red-wavy parse errors, autocompletion, hover tooltips, and go-to-definition.
- **VS Code Extension Integration** ([editors/vscode/](file:///c:/Jay/_Plugin/zenith_lang/editors/vscode/)):
  - Pre-packaged VSIX extension ([zenith.vsix](file:///c:/Jay/_Plugin/zenith_lang/zenith.vsix)) with syntax grammar (`.tmLanguage.json`) and LSP client integration.
- **Strategic Execution Plans**:
  - [action_plan.md](file:///c:/Jay/_Plugin/zenith_lang/plan/action_plan.md): Day-to-day execution priorities for language frontend, data-oriented world model, and character physics.
  - [game_sdk_pivot.md](file:///c:/Jay/_Plugin/zenith_lang/plan/game_sdk_pivot.md): Strategic core identity positioning as a pure code-first game engine SDK.

---

## 3. ❌ Missing / Incomplete Features
- **Debug Adapter Protocol (DAP)**: VS Code breakpoints & GDB/LLDB integration.
- **Package Registry CLI**: Remote package registry downloader.

---

## 4. 💻 How to Use
```bash
# Scaffold new Zenith Game Project
zenith create nebula_novel --template=game

# Launch Dev Preview Server with Hot Reload
zenith serve scenes/main.zen

# Start Language Server Protocol Daemon
zenith lsp

# Install VS Code Extension
code --install-extension zenith.vsix
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation / Source |
| :--- | :--- | :--- |
| **CLI & Subcommand Dispatcher** | N/A | [main.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/main.cpp) |
| **Runner Utility** | N/A | [runner.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/runner.cpp) |
| **LSP Daemon** | [lsp.h](file:///c:/Jay/_Plugin/zenith_lang/include/lsp/lsp.h) | [lsp.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/lsp/lsp.cpp) |
| **VS Code VSIX Package** | N/A | [zenith.vsix](file:///c:/Jay/_Plugin/zenith_lang/zenith.vsix) |
| **Action Plan Roadmap** | N/A | [action_plan.md](file:///c:/Jay/_Plugin/zenith_lang/plan/action_plan.md) |
| **Game SDK Pivot Doc** | N/A | [game_sdk_pivot.md](file:///c:/Jay/_Plugin/zenith_lang/plan/game_sdk_pivot.md) |
