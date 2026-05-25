# Zenith Language Support for VS Code

This extension provides syntax highlighting, code diagnostics, completions, and hover information for the **Zenith** programming language.

## Features

- **Syntax Highlighting**: Beautiful TextMate grammar highlighting for all keywords (`setState`, `agentic`, `orchestration`), types, comments, functions, and decorators (`@export`, `@managed`).
- **Autocompletions**: Live autocomplete suggestions for keywords, types, standard library functions (`println`, `httpGet`), and built-in UI components (`Column`, `Row`, `Button`, `Slider`, `Toggle`, etc.).
- **Diagnostics**: Real-time syntax and semantic checking as you type (displays red error squiggles).
- **Hover Information**: Tooltips showing type signatures for functions, classes, and variables.

## Getting Started

1. Make sure you have compiled the `zenith.exe` compiler in your project root.
2. Open VS Code and open your Zenith workspace folder.
3. Install this extension (via the packaged `.vsix` file).
4. Create or open any `.zen` file. The extension will automatically detect the local compiler and activate the LSP server.

## Settings

This extension contributes the following settings:

- `zenith.compilerPath`: Absolute path to the Zenith compiler executable. By default, it searches the workspace root for `zenith.exe` or `zenith`.
