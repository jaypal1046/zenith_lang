#!/bin/bash

# Zenith Development Server with Hot Reload
# Similar to `next dev` but with file watching

PORT=${PORT:-3000}
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVER_PID=""

echo "⚡ Zenith Development Server"
echo "============================"
echo ""

# Function to start/restart server
start_server() {
    if [ ! -z "$SERVER_PID" ]; then
        echo "🔄 Restarting server..."
        kill $SERVER_PID 2>/dev/null
        wait $SERVER_PID 2>/dev/null
    fi
    
    echo "🚀 Starting server on port $PORT..."
    cd "$ROOT_DIR"
    node server/ssr-server.js --port $PORT &
    SERVER_PID=$!
    echo "✓ Server started (PID: $SERVER_PID)"
    echo ""
}

# Function to handle file changes
handle_change() {
    local file=$1
    local ext="${file##*.}"
    
    case $ext in
        js)
            echo "📄 JavaScript changed: $file"
            start_server
            ;;
        css)
            echo "🎨 CSS changed: $file"
            # No restart needed for CSS, browser will reload
            ;;
        zenith)
            echo "🔧 Zenith config changed: $file"
            start_server
            ;;
    esac
}

# Start initial server
start_server

echo "👀 Watching for file changes..."
echo "   - src/pages/**/*.js"
echo "   - src/components/**/*.js"
echo "   - src/api/**/*.js"
echo "   - src/styles/**/*.css"
echo ""
echo "Press Ctrl+C to stop"
echo ""

# Watch files using find and inotifywait (Linux) or fswatch (Mac)
if command -v inotifywait &> /dev/null; then
    # Linux with inotify-tools
    while true; do
        inotifywait -r -e modify,create,delete \
            --exclude 'node_modules|dist|\.git' \
            --format '%w%f' \
            src/pages src/components src/api src/styles server 2>/dev/null | read file
        
        if [ ! -z "$file" ]; then
            handle_change "$file"
        fi
    done
elif command -v fswatch &> /dev/null; then
    # Mac with fswatch
    fswatch -o -r --exclude='node_modules|dist|\.git' \
        src/pages src/components src/api src/styles server | \
    while read; do
        echo "📁 File change detected"
        start_server
    done
else
    # Fallback: simple polling
    echo "⚠️  No file watcher found (install inotify-tools or fswatch for hot reload)"
    echo "    Using simple polling every 5 seconds..."
    
    last_check=$(date +%s)
    
    while true; do
        sleep 5
        current_check=$(date +%s)
        
        # Check if any JS/CSS files were modified recently
        recent_files=$(find src/pages src/components src/api src/styles server \
            -name "*.js" -o -name "*.css" \
            -newermt "-10 seconds" 2>/dev/null)
        
        if [ ! -z "$recent_files" ]; then
            echo "📁 Files changed:"
            echo "$recent_files"
            start_server
        fi
    done
fi

# Cleanup on exit
cleanup() {
    echo ""
    echo "🛑 Stopping server..."
    if [ ! -z "$SERVER_PID" ]; then
        kill $SERVER_PID 2>/dev/null
    fi
    echo "✓ Server stopped"
    exit 0
}

trap cleanup SIGINT SIGTERM
