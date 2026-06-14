# Zenith Framework - SSR/CSR Implementation Guide

## Overview

Zenith is a Next.js-inspired framework built for the Zenith documentation site, providing:
- **Server-Side Rendering (SSR)** - Fast initial page loads with full HTML
- **Client-Side Rendering (CSR)** - Interactive SPA experience after hydration
- **File-Based Routing** - Automatic routes from file structure
- **API Routes** - Backend endpoints in the same project
- **Dynamic Routes** - Support for `[param]` style routing

## Project Structure

```
zenith_docs_site/
├── server/
│   └── ssr-server.js      # Main SSR server (like Next.js server)
├── src/
│   ├── pages/             # Page components (file-based routing)
│   │   ├── index.js       # Home page (/)
│   │   └── docs/
│   │       ├── index.js   # Docs index (/docs)
│   │       └── [slug].js  # Dynamic docs (/docs/:slug)
│   ├── api/               # API endpoints
│   │   └── docs.js        # /api/docs endpoint
│   └── lib/
│       └── runtime.js     # Client-side runtime (hydration, router)
├── public/                # Static assets
├── dist/                  # Production build output
├── package.json
└── README.md
```

## Quick Start

### Development Mode
```bash
cd zenith_docs_site
npm run dev
# or
node server/ssr-server.js
```

Server starts at `http://localhost:3000`

### Production Mode
```bash
NODE_ENV=production node server/ssr-server.js
```

## Features

### 1. Server-Side Rendering (SSR)

Pages are rendered on the server for fast initial loads:

```javascript
// src/pages/index.js
export async function getServerSideProps(context) {
  const { req, query, params } = context;
  
  // Fetch data from database, API, etc.
  const data = await fetchData();
  
  return {
    features: [...],
    metadata: {...},
    serverInfo: {...}
  };
}

export default function HomePage(props) {
  const { features, metadata } = props;
  return `<h1>${metadata.title}</h1>...`;
}
```

### 2. Client-Side Hydration

After SSR, the client runtime takes over for SPA-like navigation:

```javascript
// Automatically included in every page
<script src="/js/runtime.js"></script>
<script>
  window.__INITIAL_STATE__ = {...};
</script>
```

The runtime handles:
- Component re-rendering
- Client-side routing
- State management
- Event delegation

### 3. File-Based Routing

Routes are automatically created from file structure:

| File | Route |
|------|-------|
| `src/pages/index.js` | `/` |
| `src/pages/docs/index.js` | `/docs` |
| `src/pages/docs/[slug].js` | `/docs/:slug` |
| `src/pages/users/[id].js` | `/users/:id` |

### 4. Dynamic Routes

Use square brackets for dynamic parameters:

```javascript
// src/pages/docs/[slug].js
export async function getServerSideProps(context) {
  const { params } = context;
  const slug = params.slug; // Extracted from URL
  
  const doc = await getDocBySlug(slug);
  return { doc };
}
```

### 5. API Routes

Create backend endpoints in `src/api/`:

```javascript
// src/api/docs.js
export async function GET(context) {
  const { query } = context;
  
  const docs = await fetchDocs(query);
  
  return {
    statusCode: 200,
    headers: { 'Content-Type': 'application/json' },
    body: { success: true, data: docs }
  };
}

export async function POST(context) {
  const { body } = context;
  
  // Validate and save
  const newDoc = await createDoc(body);
  
  return {
    statusCode: 201,
    body: { success: true, data: newDoc }
  };
}
```

Access at: `http://localhost:3000/api/docs`

### 6. Client-Side Navigation

Links with `data-zenith-link` attribute enable SPA navigation:

```html
<a href="/docs/getting-started" data-zenith-link>Get Started</a>
```

This prevents full page reloads and uses client-side routing.

## Runtime API

### ZenithRuntime (Client)

```javascript
// Access global app instance
const app = window.ZenithApp;

// State management
app.setState('user', { name: 'John' });
const user = app.getState('user');

// Register components
app.registerComponent('MyComponent', MyComponent);

// Render component
const html = app.renderToString('MyComponent', { prop: 'value' });
```

### ZenithRouter (Client)

```javascript
// Access global router
const router = window.ZenithRouter;

// Navigate programmatically
router.navigate('/docs');
router.navigate('/docs', { replace: true });

// Listen for navigation events
router.on('navigate', (path) => {
  console.log('Navigated to:', path);
});
```

## Configuration

Edit `server/ssr-server.js` constructor options:

```javascript
const server = new ZenithServer({
  port: 3000,           // Server port
  hostname: 'localhost',// Server hostname
  rootDir: __dirname,   // Project root
  distDir: './dist',    // Build output directory
  enableSSR: true,      // Enable server-side rendering
  enableAPI: true       // Enable API routes
});
```

## Middleware

Add custom middleware:

```javascript
server.use(async (req, res, next) => {
  console.log(`[${req.method}] ${req.url}`);
  
  // Add custom headers
  res.setHeader('X-Custom-Header', 'value');
  
  await next();
});
```

## Comparison with Next.js

| Feature | Next.js | Zenith |
|---------|---------|--------|
| File-based routing | ✅ | ✅ |
| SSR | ✅ | ✅ |
| CSR/Hydration | ✅ | ✅ |
| API Routes | ✅ | ✅ |
| Dynamic Routes | ✅ | ✅ |
| getServerSideProps | ✅ | ✅ |
| Image Optimization | ✅ | ❌ |
| Built-in CSS | ✅ | ❌ |
| TypeScript | ✅ | Manual |
| Hot Reload | ✅ | Manual |

## Example Pages

### Home Page (`/`)
- Hero section
- Features grid
- Quick start guide
- Ecosystem showcase

### Documentation Index (`/docs`)
- Search functionality
- Category navigation
- Document cards
- Sidebar navigation

### Dynamic Doc Page (`/docs/[slug]`)
- Breadcrumb navigation
- Content rendering
- Related documents
- Edit links

### API Endpoint (`/api/docs`)
- GET: List all docs
- POST: Create new doc
- Query filtering
- JSON responses

## Testing

### Test SSR Pages
```bash
curl http://localhost:3000/
curl http://localhost:3000/docs
curl http://localhost:3000/docs/getting-started
```

### Test API Routes
```bash
# GET request
curl http://localhost:3000/api/docs

# GET with query
curl "http://localhost:3000/api/docs?category=Introduction"

# POST request
curl -X POST http://localhost:3000/api/docs \
  -H "Content-Type: application/json" \
  -d '{"title":"New Doc","content":"Content here"}'
```

## Deployment

### Option 1: Node.js Server
```bash
NODE_ENV=production node server/ssr-server.js
```

### Option 2: Docker
```dockerfile
FROM node:18-alpine
WORKDIR /app
COPY package*.json ./
RUN npm install
COPY . .
EXPOSE 3000
CMD ["node", "server/ssr-server.js"]
```

### Option 3: Static Export
For static sites, pre-render pages to `dist/` and serve with any static host.

## Best Practices

1. **Use SSR for SEO-critical pages** - Home, docs, landing pages
2. **Use CSR for dashboards** - User-specific content
3. **Cache API responses** - Add Cache-Control headers
4. **Optimize bundle size** - Code split large components
5. **Handle errors gracefully** - Implement error boundaries
6. **Use semantic HTML** - Better accessibility and SEO

## Troubleshooting

### "Module not found" errors
- Check file paths are correct
- Ensure files don't start with `_` (reserved)
- Verify module exports

### SSR rendering fails
- Check for browser-only APIs in SSR code
- Use `typeof window !== 'undefined'` guards
- Move client-only code to useEffect

### API routes return 404
- Verify route path matches request
- Check for double slashes in paths
- Ensure handler exports default function

## Future Enhancements

- [ ] Hot module replacement (HMR)
- [ ] Built-in CSS modules support
- [ ] Image optimization
- [ ] Incremental Static Regeneration (ISR)
- [ ] Middleware system
- [ ] Plugin architecture
- [ ] TypeScript support
- [ ] Build optimization

## License

MIT License - See LICENSE file for details
