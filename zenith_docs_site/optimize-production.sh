#!/bin/bash

# Zenith Framework - Production Optimization Script
# This script optimizes assets for production deployment

set -e

echo "🚀 Starting production optimization..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if Node.js is installed
if ! command -v node &> /dev/null; then
    echo -e "${RED}❌ Node.js is not installed. Please install Node.js first.${NC}"
    exit 1
fi

# Install optimization tools if not present
echo -e "${YELLOW}📦 Checking for optimization tools...${NC}"

if ! command -v terser &> /dev/null; then
    echo "Installing Terser (JS minifier)..."
    npm install -g terser
fi

if ! command -g clean-css-cli &> /dev/null; then
    echo "Installing Clean-CSS (CSS minifier)..."
    npm install -g clean-css-cli
fi

# Create dist directory structure
echo -e "${YELLOW}📁 Creating distribution directory...${NC}"
rm -rf dist
mkdir -p dist/{server,src/{pages,layouts,lib,components},public/{css,js,images}}

# Copy server files
echo "Copying server files..."
cp server/ssr-server.js dist/server/

# Copy pages
echo "Copying page components..."
cp -r src/pages/* dist/src/pages/ 2>/dev/null || echo "No pages to copy"

# Copy layouts
echo "Copying layout components..."
cp -r src/layouts/* dist/src/layouts/ 2>/dev/null || echo "No layouts to copy"

# Copy lib files
echo "Copying library files..."
cp src/lib/runtime.js dist/src/lib/ 2>/dev/null || echo "No runtime to copy"

# Copy and optimize CSS
echo -e "${YELLOW}🎨 Optimizing CSS files...${NC}"
if [ -f "src/styles/globals.css" ]; then
    cleancss -o dist/public/css/globals.css src/styles/globals.css
    echo -e "${GREEN}✓ globals.css optimized${NC}"
fi

if [ -f "src/components/components.css" ]; then
    cleancss -o dist/public/css/components.css src/components/components.css
    echo -e "${GREEN}✓ components.css optimized${NC}"
fi

# Copy and optimize JavaScript
echo -e "${YELLOW}⚡ Optimizing JavaScript files...${NC}"

# Optimize runtime
if [ -f "src/lib/runtime.js" ]; then
    terser src/lib/runtime.js -o dist/public/js/runtime.min.js --compress --mangle
    echo -e "${GREEN}✓ runtime.js optimized${NC}"
fi

# Optimize components
for file in src/components/*.js; do
    if [ -f "$file" ]; then
        filename=$(basename "$file" .js)
        terser "$file" -o "dist/public/js/${filename}.min.js" --compress --mangle
        echo -e "${GREEN}✓ ${filename}.js optimized${NC}"
    fi
done

# Optimize pages
for file in src/pages/**/*.js src/pages/*.js; do
    if [ -f "$file" ]; then
        # Preserve directory structure
        relpath=${file#src/pages/}
        outdir="dist/src/pages/$(dirname "$relpath")"
        mkdir -p "$outdir"
        filename=$(basename "$file" .js)
        terser "$file" -o "$outdir/${filename}.min.js" --compress --mangle
        echo -e "${GREEN}✓ $relpath optimized${NC}"
    fi
done

# Copy static assets
echo -e "${YELLOW}📸 Copying static assets...${NC}"
if [ -d "public/images" ]; then
    cp -r public/images dist/public/
    echo -e "${GREEN}✓ Images copied${NC}"
fi

# Create package.json for production
echo -e "${YELLOW}📄 Creating production package.json...${NC}"
cat > dist/package.json << 'EOF'
{
  "name": "zenith-docs-production",
  "version": "1.0.0",
  "description": "Zenith Documentation Site - Production Build",
  "main": "server/ssr-server.js",
  "scripts": {
    "start": "node server/ssr-server.js",
    "health": "curl -f http://localhost:3000/api/health || exit 1"
  },
  "dependencies": {
    "express": "^4.18.2"
  },
  "engines": {
    "node": ">=18.0.0"
  }
}
EOF

# Create .env.example
echo -e "${YELLOW}🔐 Creating environment configuration...${NC}"
cat > dist/.env.example << 'EOF'
NODE_ENV=production
PORT=3000
HOST=0.0.0.0
ENABLE_CACHE=true
CACHE_TTL=3600
LOG_LEVEL=warn
ENABLE_COMPRESSION=true
EOF

# Create README for deployment
cat > dist/README.md << 'EOF'
# Zenith Docs - Production Build

## Quick Start

1. Install dependencies:
   ```bash
   npm install --production
   ```

2. Configure environment:
   ```bash
   cp .env.example .env
   # Edit .env with your settings
   ```

3. Start server:
   ```bash
   npm start
   ```

## Health Check

```bash
curl http://localhost:3000/api/health
```

## Logs

Logs are written to stdout/stderr. Use a process manager like PM2 for log management.
EOF

# Calculate statistics
echo ""
echo -e "${GREEN}==================================${NC}"
echo -e "${GREEN}✅ Production build complete!${NC}"
echo -e "${GREEN}==================================${NC}"
echo ""

# Show file sizes
echo "📊 Build Statistics:"
echo "-------------------"
ORIGINAL_SIZE=$(du -sh src 2>/dev/null | cut -f1)
DIST_SIZE=$(du -sh dist 2>/dev/null | cut -f1)
echo "Original size: $ORIGINAL_SIZE"
echo "Dist size:     $DIST_SIZE"
echo ""

# List generated files
echo "📁 Generated Files:"
echo "-------------------"
find dist -type f | wc -l | xargs -I {} echo "Total files: {}"
echo ""

echo -e "${YELLOW}Next Steps:${NC}"
echo "1. Review dist/.env.example and configure your environment"
echo "2. Run 'cd dist && npm install --production'"
echo "3. Start with 'npm start' or use PM2: 'pm2 start server/ssr-server.js'"
echo ""
echo -e "${GREEN}🎉 Ready for deployment!${NC}"
