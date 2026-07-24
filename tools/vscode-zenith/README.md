# VS Code Extension for Zenith Language

A comprehensive VS Code extension providing full language support for the Zenith programming language.

## Features

- **Syntax Highlighting**: Full TextMate grammar support for `.zen` files
- **Auto-start LSP**: Automatically launches the Zenith language server daemon
- **Language Server Protocol**: Provides IntelliSense, go-to-definition, find references, and more
- **Code Snippets**: Common Zenith code patterns
- **Bracket Matching**: Auto-closing pairs and bracket highlighting
- **Code Folding**: Region-based folding support
- **Configuration Options**: Customizable LSP path and auto-start behavior

## Installation

### From VSIX (Development)
```bash
npm install
npm run compile
vsce package
# Install the generated .vsix file in VS Code
```

### From Marketplace (Future)
Search for "Zenith Language Support" in the VS Code Extensions marketplace.

## Configuration

Add these settings to your `settings.json`:

```json
{
  "zenith.lsp.path": "/path/to/zenith-lsp",  // Optional, auto-detected if empty
  "zenith.lsp.autoStart": true,              // Auto-start LSP on file open
  "zenith.compiler.path": "zenith"           // Path to compiler executable
}
```

## Commands

- `Zenith: Start Language Server` - Manually start the LSP
- `Zenith: Restart Language Server` - Restart the LSP daemon
- `Zenith: Stop Language Server` - Stop the running LSP

## Requirements

- VS Code 1.85.0 or higher
- Zenith compiler and LSP installed and available in PATH

## Development

```bash
# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Watch mode for development
npm run watch

# Lint code
npm run lint

# Package extension
vsce package
```

## License

MIT
