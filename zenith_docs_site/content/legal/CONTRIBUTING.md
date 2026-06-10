# Contributing to Zenith Language

Thank you for your interest in contributing to Zenith Language! This document provides guidelines and instructions for contributing.

## 🌟 Code of Conduct

Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before participating.

## 📋 How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported
2. Use the GitHub issue template
3. Include:
   - Clear description
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment details (OS, Zenith version)
   - Code samples if applicable

### Suggesting Features

1. Check existing feature requests
2. Create a new issue with:
   - Feature description
   - Use case
   - Examples of how it would work
   - Potential challenges

### Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Write tests if applicable
5. Ensure all tests pass
6. Update documentation
7. Commit with clear messages
8. Push to your fork
9. Open a Pull Request

## 💻 Coding Standards

### Zenith Code Style

```zenith
// Use meaningful variable names
final userName = 'John';  // ✅ Good
final n = 'John';         // ❌ Bad

// Use const for compile-time constants
const int maxUsers = 100;

// Prefer final for variables that don't change
final count = 5;

// Use trailing commas for better diffs
Widget build(BuildContext context) {
  return Column(
    children: [
      Text('Hello'),
      Text('World'),  // Trailing comma
    ],
  );
}

// Organize imports
import 'package:zenith_ui/zenith_ui.dart';
import 'dart:async';
import 'dart:io';

import '../models/user.dart';
import '../utils/helpers.dart';
```

### Documentation

- Document all public APIs
- Use dartdoc style comments
- Include examples for complex functions
- Keep README files up to date

```zenith
/// Calculates the factorial of a number.
///
/// Example:
/// ```zenith
/// final result = factorial(5); // 120
/// ```
///
/// @param n The number to calculate factorial for
/// @returns The factorial of n
int factorial(int n) {
  if (n <= 1) return 1;
  return n * factorial(n - 1);
}
```

### Testing

```zenith
import 'package:test/test.dart';

void main() {
  group('MathUtils', () {
    test('add returns correct sum', () {
      expect(MathUtils.add(2, 3), equals(5));
    });
    
    test('divide handles zero', () {
      expect(() => MathUtils.divide(10, 0), throwsA(isA<DivisionByZeroException>()));
    });
  });
}
```

## 🔧 Development Setup

### Prerequisites

- Zenith SDK 1.0+
- Git
- Code editor (VS Code recommended)

### Setup

```bash
# Clone the repository
git clone https://github.com/zenith-lang/zenith.git
cd zenith

# Install dependencies
zenith pub get

# Run tests
zenith test

# Build documentation
zenith doc

# Run examples
zenith run web --example hello_world
```

## 📝 Commit Message Guidelines

Follow [Conventional Commits](https://www.conventionalcommits.org/):

```
feat: add new game engine module
fix: resolve memory leak in renderer
docs: update installation guide
style: format code according to standards
refactor: improve error handling
test: add unit tests for parser
chore: update dependencies
```

## 🎯 Areas Needing Contribution

- Documentation improvements
- Example applications
- Performance optimizations
- Bug fixes
- New features
- Test coverage
- Translation/localization

## 📚 Resources

- [Zenith Documentation](../INDEX.md)
- [Cookbook](../cookbook/README.md)
- [API Reference](../api/README.md)
- [Community Discord](https://discord.gg/zenith-lang)

## ❓ Questions?

- Check existing issues and discussions
- Ask in Discord #contributing channel
- Email: contribute@zenith-lang.dev

Thank you for contributing to Zenith Language! 🚀
