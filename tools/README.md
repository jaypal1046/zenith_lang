# Zenith Language Developer Tooling

This directory contains the VS Code extension and Debug Adapter Protocol (DAP) server for Zenith language development.

## VS Code Extension (`vscode-extension/`)

Provides syntax highlighting, code snippets, and language server integration for `.zen` files.

### Features

- **Syntax Highlighting**: Full TextMate grammar support for Zenith language constructs
- **Code Snippets**: Quick templates for common Zenith patterns (fn, struct, enum, impl, etc.)
- **Language Server Integration**: Connects to the Zenith LSP for advanced features
- **Auto-closing Pairs**: Automatic bracket, quote, and brace completion
- **Folding Support**: Code folding based on regions and structural blocks

### Installation

1. Open the `vscode-extension` folder in VS Code
2. Press `F5` to launch the Extension Development Host
3. Or run:
   ```bash
   npm install
   npm run compile
   # Package for distribution
   npx vsce package
   ```

### Configuration

Set the `ZENITH_LSP_PATH` environment variable to point to your Zenith language server binary:

```bash
export ZENITH_LSP_PATH=/path/to/zenith-lsp
```

### File Structure

```
vscode-extension/
├── package.json              # Extension manifest
├── tsconfig.json            # TypeScript configuration
├── language-configuration.json  # Language-specific settings
├── src/
│   └── extension.ts         # Extension entry point
├── syntaxes/
│   └── zenith.tmLanguage.json  # TextMate grammar
├── snippets/
│   └── zenith-snippets.json    # Code snippets
└── icons/                   # Extension icons (optional)
```

## Debug Adapter Protocol Server (`dap-server/`)

Implements the Microsoft Debug Adapter Protocol to enable step-by-step debugging of Zenith programs using GDB or LLDB.

### Features

- **GDB/LLDB Integration**: Automatically detects and uses available debugger
- **Breakpoint Support**: Set line breakpoints with optional conditions
- **Stepping Controls**: Step over, step in, step out functionality
- **Stack Traces**: View call stack during debugging sessions
- **Expression Evaluation**: Evaluate expressions at breakpoints
- **Thread Support**: Basic thread enumeration and selection

### Usage

Start the DAP server:

```bash
python3 zenith_dap_server.py --port 4711
```

### VS Code Integration

Add a `.vscode/launch.json` to your project:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Zenith Debug",
      "type": "zenith",
      "request": "launch",
      "program": "${file}",
      "cwd": "${workspaceFolder}",
      "preLaunchTask": "build-zenith"
    }
  ]
}
```

Or use the provided `launch.json.example` as a template.

### Supported DAP Commands

- `initialize` - Initialize debug session
- `launch` - Start program execution
- `setBreakpoints` - Set breakpoints in source files
- `continue` - Resume execution
- `next` - Step over
- `stepIn` - Step into function
- `stepOut` - Step out of function
- `threads` - List threads
- `stackTrace` - Get call stack
- `evaluate` - Evaluate expression
- `disconnect` - End debug session

### Architecture

The DAP server acts as a bridge between VS Code's debugging UI and native debuggers:

```
VS Code UI ←→ DAP Server (Python) ←→ GDB/LLDB ←→ Compiled Binary
```

## Requirements

- **VS Code Extension**: Node.js 18+, TypeScript 5+
- **DAP Server**: Python 3.8+, GDB or LLDB installed
- **Debugging**: Compiled Zenith binaries with debug symbols (`-g` flag)

## Development

### Adding New Syntax Highlighting

Edit `syntaxes/zenith.tmLanguage.json` to add new language constructs to the TextMate grammar.

### Adding New Snippets

Edit `snippets/zenith-snippets.json` to add code snippet templates.

### Extending DAP Support

Add new command handlers in `zenith_dap_server.py` by implementing the appropriate DAP request/response pattern.

## License

MIT License - See main Zenith project for details.
