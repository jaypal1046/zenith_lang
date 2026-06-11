#!/usr/bin/env python3
"""
Zenith CLI - Command Line Interface for Zenith Language

Provides commands for running, building, and managing Zenith projects.

Usage:
    zenith run <file.zen> [--target <platform>]
    zenith run web <file.zen>
    zenith run android <file.zen>
    zenith run ios <file.zen>
    zenith run desktop <file.zen>
    zenith run wasm <file.zen>
    zenith build <file.zen> [--target <platform>]
    zenith build web <file.zen>
    zenith build android <file.zen>
    zenith device-manager
"""

import argparse
import subprocess
import sys
import os
import json
from pathlib import Path
from typing import List, Optional, Dict


# Device configuration storage path
DEVICE_CONFIG_PATH = Path.home() / ".zenith" / "devices.json"

# Valid target platforms
VALID_TARGETS = ["desktop", "android", "ios", "web", "wasm"]


def get_available_devices() -> List[Dict]:
    """Get list of available devices for each platform."""
    devices = {
        "desktop": [
            {"id": "local", "name": "Local Desktop", "type": "desktop"},
        ],
        "android": [],
        "ios": [],
        "web": [
            {"id": "browser", "name": "Default Browser", "type": "web"},
        ],
        "wasm": [
            {"id": "wasm-runtime", "name": "WASM Runtime", "type": "wasm"},
        ],
    }
    
    # Detect Android devices using adb
    try:
        result = subprocess.run(
            ["adb", "devices", "-l"],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            lines = result.stdout.strip().split("\n")[1:]  # Skip header
            for line in lines:
                if line.strip() and "unauthorized" not in line:
                    parts = line.split()
                    if len(parts) >= 2:
                        device_id = parts[0]
                        model = "Android Device"
                        for part in parts:
                            if part.startswith("model:"):
                                model = part.split(":")[1]
                                break
                        devices["android"].append({
                            "id": device_id,
                            "name": f"{model} ({device_id})",
                            "type": "android"
                        })
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    
    # Detect iOS devices using ideviceinstaller (if available)
    try:
        result = subprocess.run(
            ["idevice_id", "-l"],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            device_ids = result.stdout.strip().split("\n")
            for device_id in device_ids:
                if device_id.strip():
                    devices["ios"].append({
                        "id": device_id.strip(),
                        "name": f"iOS Device ({device_id.strip()[:8]}...)",
                        "type": "ios"
                    })
    except (subprocess.SubprocessError, FileNotFoundError):
        pass
    
    return devices


def load_default_device() -> Optional[Dict]:
    """Load the default device from configuration."""
    if DEVICE_CONFIG_PATH.exists():
        try:
            config = json.loads(DEVICE_CONFIG_PATH.read_text())
            return config.get("default_device")
        except (json.JSONDecodeError, IOError):
            pass
    return None


def save_default_device(device: Dict) -> None:
    """Save the default device to configuration."""
    DEVICE_CONFIG_PATH.parent.mkdir(parents=True, exist_ok=True)
    config = {"default_device": device}
    DEVICE_CONFIG_PATH.write_text(json.dumps(config, indent=2))


def select_device(target: str) -> Optional[Dict]:
    """Interactive device selection."""
    devices = get_available_devices()
    target_devices = devices.get(target, [])
    
    if not target_devices:
        print(f"No devices found for target: {target}")
        return None
    
    if len(target_devices) == 1:
        return target_devices[0]
    
    print(f"\nSelect a device for {target}:")
    print("-" * 40)
    for i, device in enumerate(target_devices, 1):
        print(f"{i}. {device['name']} (ID: {device['id']})")
    print("-" * 40)
    
    while True:
        try:
            choice = input(f"Enter device number (1-{len(target_devices)}): ")
            idx = int(choice) - 1
            if 0 <= idx < len(target_devices):
                device = target_devices[idx]
                
                # Ask if user wants to set as default
                save_as_default = input("Set as default device? (y/N): ").lower().strip()
                if save_as_default == 'y':
                    save_default_device(device)
                    print(f"✓ Default device saved: {device['name']}")
                
                return device
            else:
                print("Invalid selection. Please try again.")
        except ValueError:
            print("Please enter a valid number.")
        except KeyboardInterrupt:
            print("\nOperation cancelled.")
            return None


def find_project_root(start_path: Path) -> Optional[Path]:
    """Find the project root by looking for main.zen/main.zenith or a .zenith marker."""
    current = start_path.absolute()
    
    # First, check if current directory has main.zen or main.zenith
    for entry_point in ["main.zen", "main.zenith"]:
        if (current / entry_point).exists():
            return current
    
    # Also check lib/ subdirectory (common Flutter/Dart structure)
    if (current / "lib").is_dir():
        for entry_point in ["main.zen", "main.zenith"]:
            if (current / "lib" / entry_point).exists():
                return current
    
    # Walk up the directory tree
    while current != current.parent:
        for entry_point in ["main.zen", "main.zenith"]:
            if (current / entry_point).exists():
                return current
        
        # Check lib/ subdirectory at each level
        if (current / "lib").is_dir():
            for entry_point in ["main.zen", "main.zenith"]:
                if (current / "lib" / entry_point).exists():
                    return current
        
        if (current / ".zenith").exists():
            return current
        current = current.parent
    
    return None


def get_default_entry_point() -> Optional[Path]:
    """Get the default entry point file (main.zen or main.zenith) from the project root."""
    cwd = Path.cwd()
    project_root = find_project_root(cwd)
    
    if project_root:
        # Prefer main.zen, fallback to main.zenith
        for entry_point in ["main.zen", "main.zenith"]:
            main_file = project_root / entry_point
            if main_file.exists():
                return main_file
        
        # Check lib/ subdirectory
        if (project_root / "lib").is_dir():
            for entry_point in ["main.zen", "main.zenith"]:
                main_file = project_root / "lib" / entry_point
                if main_file.exists():
                    return main_file
    
    return None


def run_command(args: argparse.Namespace) -> int:
    """Execute the 'run' command."""
    # Determine the file to run
    if hasattr(args, 'file') and args.file:
        file_path = Path(args.file)
    else:
        # Try to find default main.zen
        file_path = get_default_entry_point()
        if not file_path:
            print("Error: No input file specified and no 'main.zen' found in project.")
            print("Usage: zenith run <file.zen> OR place main.zen in your project root")
            return 1
    
    if not file_path.exists():
        print(f"Error: File not found: {file_path}")
        return 1
    
    target = args.target or "desktop"
    
    if target not in VALID_TARGETS:
        print(f"Error: Invalid target '{target}'. Valid targets: {', '.join(VALID_TARGETS)}")
        return 1
    
    # Check for default device
    default_device = load_default_device()
    device = None
    
    if default_device and default_device.get("type") == target:
        device = default_device
        print(f"Using default device: {device['name']}")
    elif args.device:
        # Device specified via command line
        all_devices = get_available_devices()
        for d in all_devices.get(target, []):
            if d["id"] == args.device:
                device = d
                break
        if not device:
            print(f"Error: Device not found: {args.device}")
            return 1
    else:
        # Interactive selection
        device = select_device(target)
        if not device:
            return 1
    
    print(f"\n🚀 Running {file_path.name} on {target} ({device['name']})...")
    print("-" * 60)
    
    # Build the compiler command
    cmd = ["zenith-compiler", str(file_path), "--target", target]
    
    if device:
        cmd.extend(["--device", device["id"]])
    
    if args.verbose:
        cmd.append("--verbose")
        print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=False)
        return result.returncode
    except FileNotFoundError:
        print("Error: zenith-compiler not found. Please ensure it's in your PATH.")
        return 1
    except KeyboardInterrupt:
        print("\nBuild interrupted by user.")
        return 130


def create_target_subcommand(parent_parser, target_name, command_type):
    """Create a subcommand for a specific target (e.g., 'zenith run web')."""
    subparser = parent_parser.add_parser(target_name, help=f"{command_type.capitalize()} for {target_name}")
    subparser.add_argument("file", nargs="?", help="Zenith source file (.zen) [default: main.zen]")
    subparser.add_argument(
        "-d", "--device",
        help="Device ID to run on"
    )
    subparser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    if command_type == "build":
        subparser.add_argument(
            "-o", "--output",
            help="Output directory"
        )
        subparser.add_argument(
            "-r", "--release",
            action="store_true",
            help="Build in release mode"
        )
    # Set defaults for handling
    subparser.set_defaults(
        target=target_name,
        command_type=command_type,
        func=handle_target_command
    )
    return subparser


def build_command(args: argparse.Namespace) -> int:
    """Execute the 'build' command."""
    # Determine the file to build
    if hasattr(args, 'file') and args.file:
        file_path = Path(args.file)
    else:
        # Try to find default main.zen
        file_path = get_default_entry_point()
        if not file_path:
            print("Error: No input file specified and no 'main.zen' found in project.")
            print("Usage: zenith build <file.zen> OR place main.zen in your project root")
            return 1
    
    if not file_path.exists():
        print(f"Error: File not found: {file_path}")
        return 1
    
    target = args.target or "desktop"
    
    print(f"\n🔨 Building {file_path.name} for {target}...")
    print("-" * 60)
    
    cmd = ["zenith-compiler", str(file_path), "--target", target, "--build-only"]
    
    if hasattr(args, 'output') and args.output:
        cmd.extend(["--output", args.output])
    
    if hasattr(args, 'release') and args.release:
        cmd.append("--release")
    
    if args.verbose:
        cmd.append("--verbose")
        print(f"Command: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=False)
        if result.returncode == 0:
            print(f"\n✅ Build successful!")
        return result.returncode
    except FileNotFoundError:
        print("Error: zenith-compiler not found. Please ensure it's in your PATH.")
        return 1
    except KeyboardInterrupt:
        print("\nBuild interrupted by user.")
        return 130


def handle_target_command(args: argparse.Namespace) -> int:
    """Handle target-specific commands like 'zenith run web' or 'zenith build android'."""
    command_type = getattr(args, 'command_type', None)
    
    if command_type == "run":
        return run_command(args)
    elif command_type == "build":
        return build_command(args)
    else:
        print(f"Error: Unknown command type: {command_type}")
        return 1


def device_manager_command(args: argparse.Namespace) -> int:
    """Execute the 'device-manager' command."""
    print("\n📱 Zenith Device Manager")
    print("=" * 60)
    
    devices = get_available_devices()
    
    for platform, platform_devices in devices.items():
        print(f"\n{platform.upper()}:")
        print("-" * 40)
        if platform_devices:
            for device in platform_devices:
                print(f"  • {device['name']} (ID: {device['id']})")
        else:
            print("  No devices detected")
    
    print("\n" + "=" * 60)
    
    # Show current default device
    default_device = load_default_device()
    if default_device:
        print(f"\nDefault device: {default_device['name']} ({default_device['id']})")
    else:
        print("\nNo default device set")
    
    # Offer to change default
    if not args.quiet:
        print("\nWould you like to set a default device? (y/N): ", end="")
        try:
            choice = input().lower().strip()
            if choice == 'y':
                target = input("Enter target platform (desktop/android/ios/web/wasm): ").strip()
                if target in devices and devices[target]:
                    device = select_device(target)
                    if device:
                        save_default_device(device)
                        print(f"✓ Default device updated: {device['name']}")
        except KeyboardInterrupt:
            print("\nOperation cancelled.")
    
    return 0


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        prog="zenith",
        description="Zenith Language CLI - Build and run cross-platform applications"
    )
    
    subparsers = parser.add_subparsers(dest="command", help="Available commands")
    
    # Run command with target-specific subcommands
    # Note: We need to define subcommands first, then the catch-all file argument
    run_parser = subparsers.add_parser("run", help="Run a Zenith application")
    run_subparsers = run_parser.add_subparsers(dest="run_target", help="Target platforms")
    
    # Create target-specific run subcommands (zenith run web, zenith run android, etc.)
    for target in VALID_TARGETS:
        create_target_subcommand(run_subparsers, target, "run")
    
    # Add fallback options for run command without specific target subcommand
    run_parser.add_argument("file", nargs="?", help="Zenith source file (.zen) [default: main.zen]")
    run_parser.add_argument(
        "-t", "--target",
        choices=VALID_TARGETS,
        help="Target platform (default: desktop)"
    )
    run_parser.add_argument(
        "-d", "--device",
        help="Device ID to run on"
    )
    run_parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    run_parser.set_defaults(func=handle_target_command, command_type="run")
    
    # Build command with target-specific subcommands
    build_parser = subparsers.add_parser("build", help="Build a Zenith application")
    build_subparsers = build_parser.add_subparsers(dest="build_target", help="Target platforms")
    
    # Create target-specific build subcommands (zenith build web, zenith build android, etc.)
    for target in VALID_TARGETS:
        create_target_subcommand(build_subparsers, target, "build")
    
    # Add fallback options for build command without specific target subcommand
    build_parser.add_argument("file", nargs="?", help="Zenith source file (.zen) [default: main.zen]")
    build_parser.add_argument(
        "-t", "--target",
        choices=VALID_TARGETS,
        help="Target platform (default: desktop)"
    )
    build_parser.add_argument(
        "-o", "--output",
        help="Output directory"
    )
    build_parser.add_argument(
        "-r", "--release",
        action="store_true",
        help="Build in release mode"
    )
    build_parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    build_parser.set_defaults(func=handle_target_command, command_type="build")
    
    # Device Manager command
    device_parser = subparsers.add_parser("device-manager", help="Manage connected devices")
    device_parser.add_argument(
        "-q", "--quiet",
        action="store_true",
        help="Don't prompt for changes"
    )
    device_parser.set_defaults(func=device_manager_command)
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 0
    
    # Handle target-specific subcommands (e.g., 'zenith run web file.zen')
    if hasattr(args, 'func') and args.func == handle_target_command:
        return args.func(args)
    
    # Handle device-manager command
    if hasattr(args, 'func'):
        return args.func(args)
    
    parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
