#!/bin/bash
# =====================================================
#   ZENITH DOCS SITE - Development Server
# =====================================================

GREEN='\033[0;92m'
RED='\033[0;91m'
YELLOW='\033[0;93m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'

echo ""
echo "  +================================================+"
echo "  |       Zenith Docs Site - Dev Server            |"
echo "  +================================================+"
echo ""

# Check if Python is available
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python"
else
    echo -e "${RED}[ERROR]${RESET} Python is not installed. Please install Python 3."
    exit 1
fi

echo -e "${DIM}Starting development server...${RESET}"
echo ""

# Start the server
PORT=${1:-8080}
echo -e "${BOLD}[INFO]${RESET} Serving zenith_docs_site/web/ on port ${PORT}"
echo -e "${DIM}Open http://localhost:${PORT} in your browser${RESET}"
echo ""

cd "$(dirname "$0")/../web" || exit 1

$PYTHON_CMD -m http.server $PORT
