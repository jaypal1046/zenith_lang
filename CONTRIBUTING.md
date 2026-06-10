# Contributing to Zenith Language

Thank you for your interest in contributing to Zenith! This document provides guidelines and instructions for contributing to the project.

## 🌟 How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check existing issues as the problem might already be reported. When creating a bug report, include:

- **Clear title and description**
- **Steps to reproduce** the behavior
- **Expected vs actual behavior**
- **Environment details** (OS, compiler version, etc.)
- **Code samples** that demonstrate the issue
- **Screenshots or logs** if applicable

**Example:**
```markdown
**Bug**: Compiler crashes on nested async functions

**Steps to Reproduce:**
1. Create a file with nested async functions
2. Run `zenith compile file.zen`
3. See error

**Expected:** Successful compilation
**Actual:** Segmentation fault

**Environment:** Windows 11, zenith v1.0.0
```

### Suggesting Features

Feature suggestions are welcome! Please provide:

- **Use case**: Why is this feature needed?
- **Proposed solution**: How should it work?
- **Alternatives considered**: What other approaches exist?
- **Additional context**: Any relevant information

### Pull Requests

1. **Fork the repository**
2. **Create a branch** from `main`:
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. **Make your changes** following our coding standards
4. **Test thoroughly** - ensure all tests pass
5. **Commit with clear messages** following conventional commits
6. **Push and create a PR**

## 📝 Development Setup

### Prerequisites

- C++17 compatible compiler (GCC 9+, Clang 9+, MSVC 2019+)
- CMake 3.15+
- Python 3.8+ (for build scripts)
- Git

### Building from Source

```bash
# Clone the repository
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang

# Linux/macOS
make

# Windows (MinGW)
build.bat

# Windows (MSVC)
# Use Visual Studio Developer Command Prompt
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Running Tests

```bash
# Run all tests
python tests/run_tests.py

# Run specific test
zenith tests/test_widgets.zen -target cpp

# Run web target tests
zenith tests/gallery.zen -target web
```

## 💻 Coding Standards

### C++ Code Style

- **Indentation**: 4 spaces (no tabs)
- **Naming conventions**:
  - Classes/Structs: `PascalCase`
  - Functions/Methods: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: `m_` prefix
- **Braces**: Allman style (opening brace on new line)
- **Includes**: Order by scope (project, external, system)

**Example:**
```cpp
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    void processNode() {
        // Implementation
    }

private:
    int m_lineNumber;
    std::string m_value;
};
```

### Zenith Language Code Style

When writing Zenith code examples or tests:

- **Indentation**: 4 spaces
- **Class names**: `PascalCase`
- **Function names**: `camelCase`
- **Variable names**: `camelCase` or `snake_case`
- **String literals**: Double quotes

**Example:**
```zenith
class UserService() {
    async User getUser(String id) {
        let user = await database.find(id);
        return user;
    }
}
```

### Commit Message Guidelines

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting)
- `refactor`: Code refactoring
- `test`: Adding tests
- `chore`: Build/config changes

**Examples:**
```
feat(parser): add support for async/await expressions

fix(codegen): resolve memory leak in WASM output

docs(readme): update installation instructions
```

## 🧪 Testing Guidelines

### Writing Tests

1. **Unit tests**: Test individual components in isolation
2. **Integration tests**: Test component interactions
3. **End-to-end tests**: Test complete workflows

**Test file structure:**
```zenith
// tests/test_feature.zen
import std.io;
import std.test;

class TestFeature() {
    Void testBasicFunctionality() {
        let result = someFunction();
        assert(result == expected);
    }
    
    Void testEdgeCases() {
        // Test boundary conditions
    }
}
```

### Test Coverage

Aim for high test coverage, especially for:
- Core compiler functionality
- Standard library functions
- UI components
- Cross-platform compatibility

## 📚 Documentation

### Writing Documentation

- **Clarity**: Use clear, concise language
- **Examples**: Include working code examples
- **Formatting**: Use Markdown properly
- **Updates**: Keep documentation in sync with code

### Documentation Structure

```
docs/
├── getting-started/
│   ├── installation.md
│   ├── quickstart.md
│   └── hello-world.md
├── language-guide/
│   ├── syntax.md
│   ├── types.md
│   ├── functions.md
│   └── classes.md
├── advanced/
│   ├── concurrency.md
│   ├── agentic-ai.md
│   └── ffi.md
├── cookbook/
│   ├── ui-examples.md
│   ├── web-apps.md
│   └── cli-tools.md
└── api-reference/
    ├── stdlib.md
    └── ui-components.md
```

## 🎯 Areas Needing Contribution

### High Priority

- [ ] WASM optimization
- [ ] iOS/Mobile runtime improvements
- [ ] LSP performance enhancements
- [ ] Additional UI components
- [ ] Package registry infrastructure

### Good First Issues

Look for issues labeled:
- `good first issue`
- `help wanted`
- `beginner-friendly`

## 🔀 Pull Request Process

1. **Ensure tests pass**: All existing tests must pass
2. **Add tests**: For new features or bug fixes
3. **Update documentation**: Reflect any changes
4. **Code review**: At least one maintainer approval required
5. **Squash commits**: Clean up commit history before merging
6. **Merge**: Maintainers will merge approved PRs

## 🌍 Community

### Communication

- **GitHub Issues**: For bug reports and feature requests
- **Discussions**: For questions and general discussion
- **Discord/Slack**: [INSERT LINK] for real-time chat

### Code of Conduct

Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md) to maintain a welcoming and inclusive community.

## 📖 Additional Resources

- [Architecture Guide](ARCHITECTURE_GUIDE.md)
- [Implementation Summary](IMPLEMENTATION_SUMMARY.md)
- [Game Engine Guide](game_engine_guide.md)
- [README](README.md)

## ❓ Questions?

Feel free to open an issue for any questions about contributing. We're here to help!

---

*Thank you for contributing to Zenith Language!* 🚀
