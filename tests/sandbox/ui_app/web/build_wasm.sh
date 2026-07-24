#!/bin/bash
# =====================================================
#   ZENITH RUN WASM (Mac/Linux) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  WASM Runner                |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

echo -e "  ${BOLD}[1/3]${RESET}   Transpiling lib/main.zen -> main_wasm.html ..."
$COMPILER ../lib/main.zen -target wasm -o main_wasm.html
if [ $? -ne 0 ]; then
    echo -e "  ${RED}[ERROR]${RESET} Transpile failed."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    WASM wrapper output ready: web/main_wasm.html"

echo -e "  ${BOLD}[2/3]${RESET}   Starting Python HTTP server on port 8000 ..."
python3 -m http.server 8000 > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

# Check if the server is still running
if ps -p $SERVER_PID > /dev/null; then
    echo -e "  ${GREEN}[OK]${RESET}    Server started on port 8000 (PID: $SERVER_PID)"
else
    # Fallback to python
    python -m http.server 8000 > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    if ps -p $SERVER_PID > /dev/null; then
        echo -e "  ${GREEN}[OK]${RESET}    Server started on port 8000 (PID: $SERVER_PID)"
    else
        echo -e "  ${YELLOW}[WARN]${RESET}  Failed to launch HTTP server automatically."
    fi
fi

echo -e "  ${BOLD}[3/3]${RESET}   Launching  in default browser ..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    open http://localhost:8000/main_wasm.html
else
    xdg-open http://localhost:8000/main_wasm.html 2>/dev/null || sensible-browser http://localhost:8000/main_wasm.html 2>/dev/null
fi

echo ""
echo -e "  ${GREEN}[OK]${RESET} Browser launched to http://localhost:8000/main_wasm.html"
echo ""
