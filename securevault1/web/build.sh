#!/bin/bash
# =====================================================
#   ZENITH RUN WEB (Mac/Linux) - Flutter-style
# =====================================================
GREEN='\033[0;92m'; RED='\033[0;91m'; YELLOW='\033[0;93m'
BOLD='\033[1m'; DIM='\033[2m'; RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |          Zenith  *  Web Runner                  |"
echo "  +================================================+"
echo ""

if   [ -f "../zenith" ];    then COMPILER="../zenith"
elif [ -f "../../zenith" ]; then COMPILER="../../zenith"
else COMPILER="zenith"; fi
echo -e "  ${DIM}Using Zenith compiler: $COMPILER${RESET}"
echo ""

echo -e "  ${BOLD}[1/2]${RESET}   Transpiling lib/main.zen -> main.html ..."
$COMPILER ../lib/main.zen -target web -o main.html
if [ $? -ne 0 ]; then
    echo -e "  ${RED}[ERROR]${RESET} Transpile failed."
    exit 1
fi
echo -e "  ${GREEN}[OK]${RESET}    HTML + JS output ready: web/main.html"

echo -e "  ${BOLD}[2/2]${RESET}   Launching  in default browser ..."
if [[ "$OSTYPE" == "darwin"* ]]; then
    open main.html
else
    xdg-open main.html 2>/dev/null || sensible-browser main.html 2>/dev/null
fi

echo ""
echo -e "  ${GREEN}[OK]${RESET} Browser launched"
echo ""
echo -e "  ${YELLOW}[TIP]${RESET} Serve with a local HTTP server for full AJAX support:"
echo -e "  ${DIM}        python3 -m http.server 8080${RESET}"
echo ""
