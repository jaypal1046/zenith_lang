# Zenith CLI - Command Line Interface

The Zenith CLI provides commands for running, building, and managing Zenith Language projects across multiple platforms.

## Installation

```bash
# Make the CLI executable
chmod +x tools/zenith_cli.py

# Add to your PATH (optional)
export PATH=$PATH:/path/to/zenith/tools
```

Or create an alias in your shell configuration:
```bash
alias zenith='python3 /path/to/zenith/tools/zenith_cli.py'
```

## Commands

### `zenith run` - Run a Zenith Application

Run a Zenith application on your chosen platform.

```bash
zenith run [file.zen] [OPTIONS]
zenith run <platform> [file.zen] [OPTIONS]
```

**Arguments:**
- `file.zen`: Zenith source file (optional, defaults to `main.zen` in project root)

**Options:**
- `-t, --target`: Target platform (`desktop`, `android`, `ios`, `web`, `wasm`)
- `-d, --device`: Device ID to run on (skips interactive selection)
- `-v, --verbose`: Enable verbose output

**Examples:**
```bash
# Run main.zen from project root (auto-detected)
zenith run

# Run on desktop (default)
zenith run myapp.zen

# Run specific platform using subcommand
zenith run web
zenith run android
zenith run ios

# Run with target option
zenith run myapp.zen --target android

# Run on specific device
zenith run myapp.zen --target android --device emulator-5554

# Run web app in browser
zenith run web
```

**Default Entry Point:**
If no file is specified, the CLI automatically searches for `main.zen`:
1. In the current directory
2. In parent directories up to the project root
3. A project root is identified by the presence of `main.zen` or `.zenith` folder

This allows you to simply run `zenith run` from anywhere within your project!

**Device Selection:**
When you run a command without specifying a device, the CLI will:
1. Check if a default device is configured for the target platform
2. If found, use it automatically
3. If not found, show an interactive device selection menu
4. Optionally save your selection as the default

### `zenith build` - Build a Zenith Application

Build a Zenith application for deployment.

```bash
zenith build [file.zen] [OPTIONS]
zenith build <platform> [file.zen] [OPTIONS]
```

**Arguments:**
- `file.zen`: Zenith source file (optional, defaults to `main.zen` in project root)

**Options:**
- `-t, --target`: Target platform (`desktop`, `android`, `ios`, `web`, `wasm`)
- `-o, --output`: Output directory
- `-r, --release`: Build in release mode (optimized)
- `-v, --verbose`: Enable verbose output

**Examples:**
```bash
# Build main.zen from project root (auto-detected)
zenith build

# Build for desktop
zenith build myapp.zen

# Build specific platform using subcommand
zenith build web
zenith build android

# Build release version for Android
zenith build myapp.zen --target android --release

# Build for web with custom output
zenith build web --output ./dist/web
```

### `zenith device-manager` - Manage Connected Devices

View and manage connected devices across all platforms.

```bash
zenith device-manager [OPTIONS]
```

**Options:**
- `-q, --quiet`: Don't prompt for changes (list only)

**Features:**
- List all detected devices for each platform
- View current default device
- Set new default device interactively
- Auto-detect Android devices via ADB
- Auto-detect iOS devices via ideviceinstaller

**Examples:**
```bash
# List all devices
zenith device-manager

# List devices without prompts
zenith device-manager --quiet
```

## Device Configuration

Default device settings are stored in `~/.zenith/devices.json`.

**Example configuration:**
```json
{
  "default_device": {
    "id": "emulator-5554",
    "name": "Pixel 6 (emulator-5554)",
    "type": "android"
  }
}
```

## Platform Support

| Platform | Target Name | Device Detection |
|----------|-------------|------------------|
| Desktop | `desktop` | Local system |
| Android | `android` | ADB (Android Debug Bridge) |
| iOS | `ios` | ideviceinstaller |
| Web | `web` | Default browser |
| WebAssembly | `wasm` | WASM runtime |

## Workflow Examples

### Quick Desktop Development
```bash
# From anywhere in project - auto-finds main.zen
zenith run

# First run - select device if needed
zenith run myapp.zen

# Subsequent runs - uses saved default
zenith run
```

### Multi-Platform Testing
```bash
# Test on desktop (uses main.zen by default)
zenith run

# Test on Android emulator
zenith run android

# Test in browser
zenith run web

# Test on iOS
zenith run ios
```

### Production Build
```bash
# Build optimized release for all platforms
zenith build --target desktop --release
zenith build android --release
zenith build web --release
```

### Working from Subdirectories
```bash
# Navigate to any subdirectory in your project
cd src/components

# Still works - CLI finds main.zen in project root
zenith run

# Or specify a different file
zenith run ../utils/helper.zen
```

## Troubleshooting

### No devices detected for Android
Ensure ADB is installed and your device/emulator is running:
```bash
adb devices
```

### No devices detected for iOS
Ensure libimobiledevice is installed:
```bash
brew install libimobiledevice  # macOS
idevice_id -l
```

### Compiler not found
Ensure the Zenith compiler is in your PATH:
```bash
export PATH=$PATH:/path/to/zenith
```

## Integration with VS Code

The Zenith CLI integrates with the VS Code extension for seamless development:

1. Install the Zenith Language Support extension
2. Configure the CLI path in VS Code settings:
```json
{
  "zenith.cliPath": "/path/to/zenith/tools/zenith_cli.py"
}
```

## License

MIT License - See LICENSE file for details
