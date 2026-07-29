#!/usr/bin/env python3
"""
Zenith Engine — Hot Reload & Hot Refresh Dev Watch Pipeline

This script watches .zen source files for changes and automatically
triggers a fast rebuild+relaunch cycle using the zenith compiler.
Works like `cargo watch`, `flutter run`, or `nodemon`.

Usage:
    python scripts/dev_watch.py [project_dir] [--platform desktop|web]
    
    # From project root:
    python scripts/dev_watch.py .
    
    # From zenith_lang root targeting a project:
    python scripts/dev_watch.py examples/fantasy_survival
    
    # Web target with auto-refresh:
    python scripts/dev_watch.py apps/my_desktop_app --platform web
"""

import os
import sys
import time
import subprocess
import shutil
import signal

# ── Configuration ────────────────────────────────────────────────────────────

WATCH_INTERVAL   = 0.5    # seconds between file scans
DEBOUNCE_DELAY   = 0.3    # seconds to wait after last change before rebuilding
CLEAR_ON_REBUILD = True   # clear terminal on each rebuild

# ANSI colors
CYAN    = "\033[96m"
GREEN   = "\033[92m"
YELLOW  = "\033[93m"
RED     = "\033[91m"
DIM     = "\033[2m"
BOLD    = "\033[1m"
RESET   = "\033[0m"


def find_zenith_compiler():
    """Find zenith.exe - check local dir, parent dirs, PATH."""
    candidates = [
        "zenith.exe", "zenith",
        os.path.join("..", "zenith.exe"),
        os.path.join("..", "..", "zenith.exe"),
    ]
    for c in candidates:
        if os.path.isfile(c):
            return os.path.abspath(c)
    # Check PATH
    which = shutil.which("zenith") or shutil.which("zenith.exe")
    if which:
        return which
    return None


def find_entry_file(project_dir):
    """Find the entry .zen file in a project directory."""
    for candidate in ["lib/main.zen", "src/main.zen", "main.zen"]:
        path = os.path.join(project_dir, candidate)
        if os.path.isfile(path):
            return candidate
    return None


def get_zen_files(directory):
    """Recursively find all .zen files and their modification times."""
    mtimes = {}
    for root, dirs, files in os.walk(directory):
        # Skip build/ and .git/ directories
        dirs[:] = [d for d in dirs if d not in ("build", ".git", "node_modules", "__pycache__")]
        for f in files:
            if f.endswith(".zen"):
                path = os.path.join(root, f)
                try:
                    mtimes[path] = os.path.getmtime(path)
                except OSError:
                    pass
    return mtimes


def clear_screen():
    """Clear terminal screen."""
    if CLEAR_ON_REBUILD:
        os.system("cls" if os.name == "nt" else "clear")


def print_banner(project_dir, entry_file, platform, compiler):
    """Print the dev watcher banner."""
    proj_name = os.path.basename(os.path.abspath(project_dir))
    print()
    print(f"  {BOLD}{CYAN}+==================================================+{RESET}")
    print(f"  {BOLD}{CYAN}|   Zenith Dev  *  Hot Reload Watcher              |{RESET}")
    print(f"  {BOLD}{CYAN}+==================================================+{RESET}")
    print()
    print(f"  {DIM}Project  :{RESET}  {proj_name}")
    print(f"  {DIM}Entry    :{RESET}  {entry_file}")
    print(f"  {DIM}Platform :{RESET}  {platform}")
    print(f"  {DIM}Compiler :{RESET}  {compiler}")
    print(f"  {DIM}Watching :{RESET}  *.zen files (poll every {WATCH_INTERVAL}s)")
    print()
    print(f"  {DIM}Press Ctrl+C to stop.{RESET}")
    print()
    print(f"  {DIM}{'─' * 50}{RESET}")
    print()


def run_build(compiler, project_dir, platform):
    """Run zenith run <platform> in the project directory."""
    cmd = [compiler, "run", platform]
    print(f"  {BOLD}[rebuild]{RESET} {DIM}{' '.join(cmd)}{RESET}")
    print()
    
    try:
        result = subprocess.run(
            cmd,
            cwd=project_dir,
            timeout=30,
        )
        if result.returncode == 0:
            print(f"\n  {GREEN}[OK]{RESET} Build & run succeeded.")
        else:
            print(f"\n  {RED}[x]{RESET}  Build failed (exit code {result.returncode}).")
    except subprocess.TimeoutExpired:
        print(f"\n  {YELLOW}[timeout]{RESET} Build timed out after 30s.")
    except FileNotFoundError:
        print(f"\n  {RED}[error]{RESET} Compiler not found: {compiler}")
    
    print()
    print(f"  {DIM}{'─' * 50}{RESET}")
    print(f"  {DIM}Watching for changes...{RESET}")
    print()


def main():
    # Parse arguments
    project_dir = "."
    platform = "desktop"
    
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--platform" and i + 1 < len(args):
            platform = args[i + 1]
            i += 2
        elif args[i] == "--help" or args[i] == "-h":
            print(__doc__)
            sys.exit(0)
        elif not args[i].startswith("-"):
            project_dir = args[i]
            i += 1
        else:
            i += 1
    
    project_dir = os.path.abspath(project_dir)
    
    if not os.path.isdir(project_dir):
        print(f"  {RED}Error:{RESET} Directory not found: {project_dir}")
        sys.exit(1)
    
    # Find compiler
    compiler = find_zenith_compiler()
    if not compiler:
        print(f"  {RED}Error:{RESET} zenith compiler not found.")
        print(f"  Make sure zenith.exe is in your PATH or project directory.")
        sys.exit(1)
    
    # Find entry file
    entry_file = find_entry_file(project_dir)
    if not entry_file:
        print(f"  {RED}Error:{RESET} No entry point found in {project_dir}")
        print(f"  Expected: lib/main.zen, src/main.zen, or main.zen")
        sys.exit(1)
    
    # Print banner
    clear_screen()
    print_banner(project_dir, entry_file, platform, compiler)
    
    # Initial build
    print(f"  {YELLOW}[initial]{RESET} Running first build...\n")
    run_build(compiler, project_dir, platform)
    
    # Watch loop
    last_mtimes = get_zen_files(project_dir)
    last_change_time = 0
    pending_rebuild = False
    
    try:
        while True:
            time.sleep(WATCH_INTERVAL)
            
            current_mtimes = get_zen_files(project_dir)
            changed_files = []
            
            # Detect new or modified files
            for path, mtime in current_mtimes.items():
                if path not in last_mtimes or mtime > last_mtimes[path]:
                    changed_files.append(path)
            
            # Detect deleted files
            for path in last_mtimes:
                if path not in current_mtimes:
                    changed_files.append(path)
            
            if changed_files:
                last_change_time = time.time()
                pending_rebuild = True
                
                for cf in changed_files:
                    rel = os.path.relpath(cf, project_dir)
                    if cf in current_mtimes:
                        print(f"  {YELLOW}[changed]{RESET} {rel}")
                    else:
                        print(f"  {RED}[deleted]{RESET} {rel}")
                
                last_mtimes = current_mtimes
            
            # Debounce: wait for DEBOUNCE_DELAY after last change before rebuilding
            if pending_rebuild and (time.time() - last_change_time) >= DEBOUNCE_DELAY:
                pending_rebuild = False
                print()
                print(f"  {CYAN}⚡ Hot Reload triggered!{RESET}")
                print()
                run_build(compiler, project_dir, platform)
    
    except KeyboardInterrupt:
        print(f"\n\n  {GREEN}[OK]{RESET} Dev watcher stopped.\n")


if __name__ == "__main__":
    main()
