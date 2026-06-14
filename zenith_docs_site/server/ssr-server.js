/**
 * Zenith Server-Side Rendering (SSR) Server
 * Similar to Next.js server - handles SSR, API routes, and static files
 */

const http = require('http');
const fs = require('fs');
const path = require('path');
const url = require('url');

class ZenithServer {
  constructor(config = {}) {
    this.config = {
      port: config.port || 3000,
      hostname: config.hostname || 'localhost',
      rootDir: config.rootDir || path.join(__dirname, '..'),
      distDir: config.distDir || path.join(__dirname, '..', 'dist'),
      enableSSR: config.enableSSR !== false,
      enableAPI: config.enableAPI !== false,
      ...config
    };
    
    this.pages = new Map();
    this.apiRoutes = new Map();
    this.middleware = [];
    this.server = null;
    
    this.loadPages();
    this.loadAPIRoutes();
  }

  // Load page modules
  loadPages() {
    const pagesDir = path.join(this.config.rootDir, 'src', 'pages');
    
    if (!fs.existsSync(pagesDir)) {
      console.warn('[Zenith] Pages directory not found:', pagesDir);
      return;
    }

    this.scanDirectory(pagesDir, (filePath, relativePath) => {
      const route = this.filePathToRoute(relativePath);
      try {
        const pageModule = require(filePath);
        this.pages.set(route, pageModule);
        console.log(`[Zenith] Loaded page: ${route}`);
      } catch (error) {
        console.error(`[Zenith] Error loading page ${route}:`, error.message);
      }
    });
  }

  // Load API routes
  loadAPIRoutes() {
    const apiDir = path.join(this.config.rootDir, 'src', 'api');
    
    if (!fs.existsSync(apiDir)) {
      return;
    }

    this.scanDirectory(apiDir, (filePath, relativePath) => {
      // Fix: Remove double slash issue
      const route = `/api/${this.filePathToRoute(relativePath).replace(/^\//, '')}`;
      try {
        const apiModule = require(filePath);
        this.apiRoutes.set(route, apiModule);
        console.log(`[Zenith] Loaded API route: ${route}`);
      } catch (error) {
        console.error(`[Zenith] Error loading API route ${route}:`, error.message);
      }
    });
  }

  // Scan directory recursively
  scanDirectory(dir, callback, baseDir = dir) {
    const files = fs.readdirSync(dir);
    
    files.forEach(file => {
      const filePath = path.join(dir, file);
      const stat = fs.statSync(filePath);
      
      if (stat.isDirectory()) {
        this.scanDirectory(filePath, callback, baseDir);
      } else if (file.endsWith('.js') && !file.startsWith('_')) {
        const relativePath = path.relative(baseDir, filePath);
        callback(filePath, relativePath);
      }
    });
  }

  // Convert file path to route
  filePathToRoute(filePath) {
    let route = filePath
      .replace(/\.js$/, '')
      .replace(/\\/g, '/')
      .replace(/index$/, '');
    
    if (!route.startsWith('/')) {
      route = '/' + route;
    }
    
    if (route.endsWith('/') && route.length > 1) {
      route = route.slice(0, -1);
    }
    
    if (route === '') {
      route = '/';
    }
    
    return route;
  }

  // Add middleware
  use(middleware) {
    this.middleware.push(middleware);
  }

  // Handle HTTP requests
  async handleRequest(req, res) {
    const parsedUrl = url.parse(req.url, true);
    const pathname = parsedUrl.pathname;
    const method = req.method;

    console.log(`[${method}] ${pathname}`);

    // Run middleware
    for (const mw of this.middleware) {
      try {
        await mw(req, res, () => {});
        if (res.writableEnded) return;
      } catch (error) {
        console.error('[Zenith] Middleware error:', error);
        res.writeHead(500, { 'Content-Type': 'text/plain' });
        res.end('Internal Server Error');
        return;
      }
    }

    // Handle API routes
    if (pathname.startsWith('/api/') && this.config.enableAPI) {
      return this.handleAPIRoute(req, res, pathname, parsedUrl.query);
    }

    // Handle static files
    if (pathname.startsWith('/_static/') || 
        pathname.startsWith('/js/') || 
        pathname.startsWith('/css/') ||
        /\.[css|js|png|jpg|svg|ico|woff|woff2]$/.test(pathname)) {
      return this.handleStaticFile(req, res, pathname);
    }

    // Handle pages (SSR or static)
    if (this.config.enableSSR) {
      return this.handlePageSSR(req, res, pathname, parsedUrl.query);
    } else {
      return this.handleStaticPage(req, res, pathname);
    }
  }

  // Handle API route
  async handleAPIRoute(req, res, pathname, query) {
    const apiModule = this.apiRoutes.get(pathname);
    
    if (!apiModule) {
      // Try dynamic routes
      for (const [route, module] of this.apiRoutes.entries()) {
        if (this.matchRoute(route, pathname)) {
          return this.executeAPIHandler(module, req, res, query, this.extractParams(route, pathname));
        }
      }
      
      res.writeHead(404, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ error: 'API endpoint not found' }));
      return;
    }

    return this.executeAPIHandler(apiModule, req, res, query, {});
  }

  // Execute API handler
  async executeAPIHandler(module, req, res, query, params) {
    try {
      const handler = module.default || module.handler || module;
      
      if (typeof handler !== 'function') {
        throw new Error('Invalid API handler');
      }

      const context = {
        req,
        res,
        query,
        params,
        method: req.method,
        headers: req.headers,
        body: await this.parseBody(req)
      };

      const result = await handler(context);
      
      if (!res.writableEnded) {
        const statusCode = result.statusCode || 200;
        const headers = result.headers || { 'Content-Type': 'application/json' };
        
        res.writeHead(statusCode, headers);
        
        if (typeof result.body === 'object') {
          res.end(JSON.stringify(result.body));
        } else {
          res.end(result.body || '');
        }
      }
    } catch (error) {
      console.error('[Zenith] API error:', error);
      res.writeHead(500, { 'Content-Type': 'application/json' });
      res.end(JSON.stringify({ error: error.message }));
    }
  }

  // Handle page with SSR
  async handlePageSSR(req, res, pathname, query) {
    let pageModule = this.pages.get(pathname);
    
    if (!pageModule) {
      // Try dynamic routes
      for (const [route, module] of this.pages.entries()) {
        if (this.matchRoute(route, pathname)) {
          return this.renderPage(module, req, res, query, this.extractParams(route, pathname));
        }
      }
      
      // Check for static file
      const staticPath = path.join(this.config.distDir, pathname, 'index.html');
      if (fs.existsSync(staticPath)) {
        return this.serveFile(res, staticPath, 'text/html');
      }
      
      // 404
      res.writeHead(404, { 'Content-Type': 'text/html' });
      res.end(this.render404());
      return;
    }

    return this.renderPage(pageModule, req, res, query, {});
  }

  // Render page with SSR
  async renderPage(pageModule, req, res, query, params) {
    try {
      const pageComponent = pageModule.default || pageModule.render || pageModule;
      
      if (typeof pageComponent !== 'function') {
        throw new Error('Invalid page component');
      }

      const context = {
        req,
        res,
        query,
        params,
        path: req.url,
        isServer: true
      };

      // Get page metadata
      const getServerSideProps = pageModule.getServerSideProps || pageModule.ssr;
      let props = {};
      
      if (getServerSideProps && typeof getServerSideProps === 'function') {
        props = await getServerSideProps(context);
      }

      // Render HTML
      const html = pageComponent({ ...context, ...props });
      const layout = pageModule.layout || 'default';
      
      // Wrap with layout
      const fullHTML = this.wrapWithLayout(html, layout, context);

      res.writeHead(200, { 
        'Content-Type': 'text/html; charset=utf-8',
        'Cache-Control': 'no-cache, no-store, must-revalidate'
      });
      res.end(fullHTML);
    } catch (error) {
      console.error('[Zenith] SSR error:', error);
      res.writeHead(500, { 'Content-Type': 'text/html' });
      res.end(this.render500(error));
    }
  }

  // Handle static page (pre-built)
  handleStaticPage(req, res, pathname) {
    let filePath = path.join(this.config.distDir, pathname);
    
    // Try index.html
    if (!fs.existsSync(filePath)) {
      filePath = path.join(filePath, 'index.html');
    }

    if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
      this.serveFile(res, filePath, 'text/html');
    } else {
      res.writeHead(404, { 'Content-Type': 'text/html' });
      res.end('<h1>404 - Page Not Found</h1>');
    }
  }

  // Handle static files
  handleStaticFile(req, res, pathname) {
    let filePath = null;
    
    // Check in public directory first (for development)
    const publicPath = path.join(this.config.rootDir, 'public', pathname);
    if (fs.existsSync(publicPath) && fs.statSync(publicPath).isFile()) {
      filePath = publicPath;
    }
    // Then check dist directory (for production)
    else {
      filePath = path.join(this.config.distDir, pathname);
      
      // Remove leading /_static/
      if (pathname.startsWith('/_static/')) {
        filePath = path.join(this.config.rootDir, 'public', pathname.replace('/_static/', ''));
      }
    }

    const ext = path.extname(filePath).toLowerCase();
    const mimeTypes = {
      '.html': 'text/html',
      '.js': 'application/javascript',
      '.css': 'text/css',
      '.json': 'application/json',
      '.png': 'image/png',
      '.jpg': 'image/jpeg',
      '.gif': 'image/gif',
      '.svg': 'image/svg+xml',
      '.ico': 'image/x-icon',
      '.woff': 'font/woff',
      '.woff2': 'font/woff2'
    };

    const contentType = mimeTypes[ext] || 'application/octet-stream';

    if (fs.existsSync(filePath) && fs.statSync(filePath).isFile()) {
      this.serveFile(res, filePath, contentType);
    } else {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      res.end('File not found');
    }
  }

  // Serve file
  serveFile(res, filePath, contentType) {
    const content = fs.readFileSync(filePath);
    res.writeHead(200, { 'Content-Type': contentType });
    res.end(content);
  }

  // Parse request body
  parseBody(req) {
    return new Promise((resolve) => {
      let body = '';
      req.on('data', chunk => body += chunk.toString());
      req.on('end', () => {
        try {
          resolve(JSON.parse(body));
        } catch {
          resolve(body);
        }
      });
      req.on('error', () => resolve(null));
    });
  }

  // Match dynamic routes (e.g., /users/[id])
  matchRoute(routePattern, actualPath) {
    const patternParts = routePattern.split('/');
    const actualParts = actualPath.split('/');
    
    if (patternParts.length !== actualParts.length) {
      return false;
    }
    
    for (let i = 0; i < patternParts.length; i++) {
      const patternPart = patternParts[i];
      const actualPart = actualParts[i];
      
      if (patternPart.startsWith('[') && patternPart.endsWith(']')) {
        continue; // Dynamic parameter
      }
      
      if (patternPart !== actualPart) {
        return false;
      }
    }
    
    return true;
  }

  // Extract parameters from dynamic route
  extractParams(routePattern, actualPath) {
    const params = {};
    const patternParts = routePattern.split('/');
    const actualParts = actualPath.split('/');
    
    for (let i = 0; i < patternParts.length; i++) {
      const patternPart = patternParts[i];
      const actualPart = actualParts[i];
      
      if (patternPart.startsWith('[') && patternPart.endsWith(']')) {
        const paramName = patternPart.slice(1, -1);
        params[paramName] = actualPart;
      }
    }
    
    return params;
  }

  // Wrap content with layout
  wrapWithLayout(content, layoutName, context) {
    // For now, use a simple default layout
    // In production, this would load from layouts directory
    
    // Create safe context for JSON (exclude req/res objects)
    const safeContext = {
      path: context.path || context.req?.url,
      query: context.query,
      params: context.params,
      isServer: context.isServer,
      timestamp: Date.now()
    };
    
    return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Zenith Docs</title>
  <link rel="stylesheet" href="/css/globals.css">
</head>
<body>
  <div id="app">${content}</div>
  <script src="/js/runtime.js"></script>
  <script>
    window.__INITIAL_STATE__ = ${JSON.stringify(safeContext)};
  </script>
</body>
</html>`;
  }

  // Render 404 page
  render404() {
    return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>404 - Page Not Found</title>
  <link rel="stylesheet" href="/css/globals.css">
</head>
<body>
  <div id="app">
    <div class="error-page">
      <h1>404</h1>
      <p>Page not found</p>
      <a href="/" data-zenith-link>Go Home</a>
    </div>
  </div>
  <script src="/js/runtime.js"></script>
</body>
</html>`;
  }

  // Render 500 page
  render500(error) {
    return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>500 - Server Error</title>
  <link rel="stylesheet" href="/css/globals.css">
</head>
<body>
  <div id="app">
    <div class="error-page">
      <h1>500</h1>
      <p>Server Error</p>
      <pre>${error ? error.message : ''}</pre>
      <a href="/" data-zenith-link>Go Home</a>
    </div>
  </div>
  <script src="/js/runtime.js"></script>
</body>
</html>`;
  }

  // Start the server
  start(callback) {
    this.server = http.createServer((req, res) => this.handleRequest(req, res));
    
    this.server.listen(this.config.port, this.config.hostname, () => {
      const address = `http://${this.config.hostname}:${this.config.port}`;
      console.log(``);
      console.log(`✓ Zenith server ready at ${address}`);
      console.log(`✓ SSR enabled: ${this.config.enableSSR}`);
      console.log(`✓ API routes enabled: ${this.config.enableAPI}`);
      console.log(`✓ Pages loaded: ${this.pages.size}`);
      console.log(`✓ API routes loaded: ${this.apiRoutes.size}`);
      console.log(``);
      
      if (callback) callback(address);
    });
    
    return this.server;
  }

  // Stop the server
  stop() {
    if (this.server) {
      this.server.close();
      console.log('[Zenith] Server stopped');
    }
  }
}

// Export
module.exports = ZenithServer;

// CLI usage
if (require.main === module) {
  const server = new ZenithServer({
    port: process.env.PORT || 3000,
    hostname: process.env.HOST || 'localhost'
  });
  
  server.start();
}
