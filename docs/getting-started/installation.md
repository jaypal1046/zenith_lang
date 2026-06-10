# Installation Guide

This guide will help you install the Zenith Programming Language on your system.

## Prerequisites

Before installing Zenith, ensure you have the following:

- **Git**: For cloning the repository
- **C++17 Compiler**: GCC 9+, Clang 9+, or MSVC 2019+
- **Python 3.8+**: For build scripts (optional but recommended)
- **Make** (Linux/macOS) or **MinGW** (Windows)

## Installation Methods

### Method 1: Build from Source (Recommended)

#### Linux/macOS

```bash
# 1. Clone the repository
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang

# 2. Build the compiler
make

# 3. Add to PATH (optional)
export PATH="$PATH:$(pwd)"

# 4. Verify installation
zenith --version
```

#### Windows (MinGW)

```cmd
:: 1. Clone the repository
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang

:: 2. Build the compiler
build.bat

:: 3. Add to PATH (optional)
:: Add the zenith_lang directory to your system PATH

:: 4. Verify installation
zenith --version
```

#### Windows (MSVC)

```cmd
:: 1. Clone the repository
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang

:: 2. Open Visual Studio Developer Command Prompt
:: 3. Build using CMake
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release

:: 4. The executable will be in build\Release\zenith.exe
```

### Method 2: Pre-built Binaries (Coming Soon)

Pre-built binaries for major platforms will be available soon on the [Releases page](https://github.com/jaypal1046/zenith_lang/releases).

### Method 3: Package Managers (Planned)

Future support for:
- **Homebrew** (macOS): `brew install zenith-lang`
- **Scoop** (Windows): `scoop install zenith-lang`
- **APT** (Ubuntu/Debian): `apt install zenith-lang`
- **Cargo** (Rust ecosystem): `cargo install zenith-lang`

## Platform-Specific Instructions

### Ubuntu/Debian

```bash
# Install prerequisites
sudo apt update
sudo apt install -y build-essential git python3 cmake

# Clone and build
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang
make

# Optional: Install system-wide
sudo cp zenith /usr/local/bin/
```

### Fedora/RHEL

```bash
# Install prerequisites
sudo dnf install -y gcc-c++ git python3 cmake make

# Clone and build
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang
make
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install prerequisites via Homebrew (optional)
brew install cmake python3

# Clone and build
git clone https://github.com/jaypal1046/zenith_lang.git
cd zenith_lang
make
```

### Windows (WSL)

```bash
# If using WSL, follow Linux instructions above
# Ensure WSL is installed and configured
# Then proceed with Linux installation steps
```

## Verification

After installation, verify that Zenith is working correctly:

```bash
# Check version
zenith --version

# Create a test file
echo 'import std.io; Void main() { println("Hello, Zenith!"); }' > test.zen

# Compile and run
zenith test.zen -target cpp
./test  # On Windows: test.exe
```

Expected output:
```
Hello, Zenith!
```

## Updating Zenith

To update to the latest version:

```bash
# Navigate to your zenith_lang directory
cd path/to/zenith_lang

# Pull latest changes
git pull origin main

# Rebuild
make  # or build.bat on Windows
```

## Uninstallation

To remove Zenith from your system:

```bash
# If installed system-wide (Linux/macOS)
sudo rm /usr/local/bin/zenith

# Remove source directory
rm -rf path/to/zenith_lang

# Remove from PATH if you added it manually
# Edit your shell configuration file (.bashrc, .zshrc, etc.)
```

## Troubleshooting

### Common Issues

#### "Command not found: zenith"

Ensure Zenith is in your PATH:
```bash
# Temporarily add to PATH
export PATH="$PATH:/path/to/zenith_lang"

# Permanently add to PATH (Linux/macOS)
echo 'export PATH="$PATH:/path/to/zenith_lang"' >> ~/.bashrc
source ~/.bashrc
```

#### Build fails with C++ errors

Ensure you have a C++17 compatible compiler:
```bash
# Check GCC version
gcc --version  # Should be 9 or higher

# Check Clang version
clang --version  # Should be 9 or higher
```

#### Python build script errors

Ensure Python 3.8+ is installed:
```bash
python3 --version
```

### Getting Help

If you encounter issues:
1. Check existing [GitHub Issues](https://github.com/jaypal1046/zenith_lang/issues)
2. Create a new issue with details about your environment
3. Join our community chat [INSERT LINK]

## Next Steps

Once installed, proceed to:
- [Quick Start Guide](quickstart.md) - Learn the basics
- [Editor Setup](editor-setup.md) - Configure your IDE
- [Your First Program](hello-world.md) - Write Hello World

---

*Last updated: January 2024*
