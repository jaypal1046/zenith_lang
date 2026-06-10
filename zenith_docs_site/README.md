# Zenith Language - Official Documentation & Cookbook

## 🌟 Overview

Zenith is a next-generation programming language designed for building cross-platform applications with 100% Zenith code. No other languages required!

### Supported Platforms
- 🎮 **Games** - Full game development support (2D/3D)
- 📱 **Mobile** - iOS & Android native apps
- 🖥️ **Desktop** - Windows, macOS, Linux
- 🌐 **Web** - Modern web applications (SPA/PWA)
- ⚡ **WebAssembly** - High-performance web modules

### Package Ecosystem
- 🦀 **Rust Bridge** - Seamless Rust package integration via `zenith_rust`
  - Direct FFI bindings to 10,000+ Rust crates
  - Zero-copy data sharing
  - Async/await support across boundaries
- 🎯 **Dart Bridge** - Dart package compatibility through auto-binding generators
  - Access to 20,000+ packages from pub.dev
  - Auto-generated bindings
  - Flutter widget compatibility

## 📚 Documentation Structure

```
zenith_docs_site/
├── lib/                    # Website source code (Zenith)
│   ├── pages/             # Page components
│   ├── components/        # Reusable UI components
│   └── utils/             # Utility functions
├── content/               # Markdown content
│   ├── cookbook/          # Code examples & recipes
│   ├── docs/              # Documentation guides
│   └── legal/             # Legal documents
├── packages/              # Bridge packages
│   ├── rust_bridge/       # Rust integration
│   └── dart_bridge/       # Dart integration
└── assets/                # Images, icons, etc.
```

## 🚀 Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/zenith-lang/zenith_docs_site.git
cd zenith_docs_site

# Install dependencies
zenith pub get

# Run development server
zenith run serve
```

### Build for Production

```bash
# Build for web
zenith build web --release

# Build for desktop
zenith build desktop --release

# Build for mobile
zenith build ios --release
zenith build android --release
```

## 📖 Cookbook Highlights

Our cookbook includes 25+ practical examples:

### Getting Started
- Hello World
- Variables & Data Types
- Control Flow
- Functions & Methods

### Applications
- Calculator App
- Todo List
- Weather Dashboard
- Chat Application

### Games
- Tic-Tac-Toe
- Snake Game
- Platformer Example
- 3D Cube Renderer

### Advanced
- Rust Integration Example
- Dart Package Usage
- AI-Powered App
- Real-time Database
- WebSocket Chat

## 🔗 Package Integration

### Using Rust Packages

```zenith
import 'package:zenith_rust/zenith_rust.dart';

// Initialize Rust bridge
final rustBridge = RustBridge();

// Use Rust packages directly
final result = await rustBridge.call('my_rust_function', params);
```

### Using Dart Packages

```zenith
// Auto-generated bindings allow Dart package usage
import 'package:http/http.dart' as http;

// Works seamlessly in Zenith
final response = await http.get(Uri.parse('https://api.example.com'));
```

## 📄 Legal & Governance

- [License](content/legal/LICENSE.md) - MIT License
- [Code of Conduct](content/legal/CODE_OF_CONDUCT.md)
- [Terms & Conditions](content/legal/TERMS.md)
- [Contributing Guide](content/legal/CONTRIBUTING.md)

## 🤝 Community

- 💬 Join our Discord
- 🐛 Report issues on GitHub
- 📝 Contribute to documentation
- 🎨 Share your projects

## 📞 Support

For questions and support:
- Email: support@zenith-lang.dev
- Twitter: @ZenithLang
- Stack Overflow: [zenith-lang]

---

Built with ❤️ using 100% Zenith Code
