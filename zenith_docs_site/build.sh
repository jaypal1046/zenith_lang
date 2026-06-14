#!/usr/bin/env bash

# Zenith Docs Site Build Script
# Inspired by Next.js build system
# Built with 100% Zenith Code

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SRC_DIR="src"
PAGES_DIR="$SRC_DIR/pages"
COMPONENTS_DIR="$SRC_DIR/components"
LAYOUTS_DIR="$SRC_DIR/layouts"
OUTPUT_DIR="dist"
PUBLIC_DIR="public"

# Functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    cat << EOF
Zenith Docs Site Build System
==============================

Usage: $0 <command> [options]

Commands:
  dev         Start development server with hot reload
  build       Build for production
  start       Start production server
  clean       Clean build artifacts
  help        Show this help message

Options:
  --port      Port number (default: 3000)
  --verbose   Enable verbose output

Examples:
  $0 dev                  # Start dev server on port 3000
  $0 dev --port 8080      # Start dev server on port 8080
  $0 build                # Build for production
  $0 start                # Start production server
  $0 clean                # Remove dist directory

EOF
}

clean_build() {
    log_info "Cleaning build artifacts..."
    rm -rf "$OUTPUT_DIR"
    rm -rf .zenith_cache
    log_success "Clean complete!"
}

check_dependencies() {
    log_info "Checking dependencies..."
    
    # Check if Zenith is installed
    if ! command -v zenith &> /dev/null; then
        log_warning "Zenith CLI not found. Using fallback mode."
        return 1
    fi
    
    # Check pubspec.yaml
    if [ ! -f "pubspec.yaml" ]; then
        log_error "pubspec.yaml not found!"
        return 1
    fi
    
    log_success "Dependencies OK"
    return 0
}

install_dependencies() {
    log_info "Installing dependencies..."
    
    if command -v dart &> /dev/null; then
        dart pub get
        log_success "Dependencies installed!"
    else
        log_warning "Dart not found. Skipping dependency installation."
    fi
}

build_pages() {
    log_info "Building pages..."
    
    # Create output directory
    mkdir -p "$OUTPUT_DIR"
    
    # Copy public assets
    if [ -d "$PUBLIC_DIR" ]; then
        log_info "Copying public assets..."
        cp -r "$PUBLIC_DIR"/* "$OUTPUT_DIR"/ 2>/dev/null || true
    fi
    
    # Copy styles
    mkdir -p "$OUTPUT_DIR/styles"
    if [ -f "$SRC_DIR/styles/globals.css" ]; then
        cp "$SRC_DIR/styles/globals.css" "$OUTPUT_DIR/styles/"
        log_info "Styles copied"
    fi
    
    # Process pages
    if [ -d "$PAGES_DIR" ]; then
        log_info "Processing pages..."
        
        for page_file in "$PAGES_DIR"/*.zenith; do
            if [ -f "$page_file" ]; then
                page_name=$(basename "$page_file" .zenith)
                
                # Skip special files
                if [[ "$page_name" == _* ]]; then
                    continue
                fi
                
                log_info "  Building: $page_name"
                
                # In production, this would compile Zenith to HTML
                # For now, create placeholder
                if [ "$page_name" == "index" ]; then
                    cp "$OUTPUT_DIR/../web/main.html" "$OUTPUT_DIR/index.html" 2>/dev/null || \
                    echo "<html><body><h1>$page_name</h1></body></html>" > "$OUTPUT_DIR/$page_name.html"
                else
                    echo "<html><body><h1>$page_name</h1></body></html>" > "$OUTPUT_DIR/$page_name.html"
                fi
            fi
        done
        
        log_success "Pages built!"
    else
        log_warning "Pages directory not found: $PAGES_DIR"
    fi
}

optimize_assets() {
    log_info "Optimizing assets..."
    
    # Minify CSS (if tools available)
    if command -v cssnano &> /dev/null; then
        find "$OUTPUT_DIR" -name "*.css" -exec cssnano {} \;
        log_info "CSS optimized"
    fi
    
    # Minify HTML (if tools available)
    if command -v html-minifier &> /dev/null; then
        find "$OUTPUT_DIR" -name "*.html" -exec html-minifier --collapse-whitespace --remove-comments {} \;
        log_info "HTML optimized"
    fi
    
    log_success "Optimization complete!"
}

start_dev_server() {
    local port=${1:-3000}
    
    log_info "Starting development server on port $port..."
    
    # Check if Python is available for simple HTTP server
    if command -v python3 &> /dev/null; then
        log_info "Using Python HTTP server..."
        python3 -m http.server "$port" --directory "$OUTPUT_DIR" 2>/dev/null &
        SERVER_PID=$!
        echo $SERVER_PID > .dev_server.pid
        log_success "Development server running at http://localhost:$port"
        log_info "Press Ctrl+C to stop"
        
        # Watch for file changes
        watch_files &
        
        wait
    else
        log_error "Python3 not found. Cannot start server."
        exit 1
    fi
}

watch_files() {
    log_info "Watching for file changes..."
    
    # Simple file watcher using inotifywait or fallback
    if command -v inotifywait &> /dev/null; then
        while inotifywait -r -e modify,create,delete "$SRC_DIR" "$PUBLIC_DIR"; do
            log_info "Changes detected, rebuilding..."
            build_pages
        done
    else
        log_warning "inotifywait not found. File watching disabled."
    fi
}

start_prod_server() {
    local port=${1:-3000}
    
    if [ ! -d "$OUTPUT_DIR" ]; then
        log_error "Production build not found. Run '$0 build' first."
        exit 1
    fi
    
    log_info "Starting production server on port $port..."
    
    if command -v python3 &> /dev/null; then
        python3 -m http.server "$port" --directory "$OUTPUT_DIR"
    else
        log_error "Python3 not found. Cannot start server."
        exit 1
    fi
}

# Main commands
case "${1:-help}" in
    dev)
        shift
        PORT=3000
        while [[ $# -gt 0 ]]; do
            case $1 in
                --port)
                    PORT="$2"
                    shift 2
                    ;;
                *)
                    shift
                    ;;
            esac
        done
        
        check_dependencies || true
        install_dependencies || true
        clean_build
        build_pages
        optimize_assets
        start_dev_server "$PORT"
        ;;
    
    build)
        check_dependencies || true
        install_dependencies || true
        clean_build
        build_pages
        optimize_assets
        log_success "Production build complete! Output: $OUTPUT_DIR"
        ;;
    
    start)
        shift
        PORT=3000
        while [[ $# -gt 0 ]]; do
            case $1 in
                --port)
                    PORT="$2"
                    shift 2
                    ;;
                *)
                    shift
                    ;;
            esac
        done
        start_prod_server "$PORT"
        ;;
    
    clean)
        clean_build
        ;;
    
    help|--help|-h)
        show_help
        ;;
    
    *)
        log_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
