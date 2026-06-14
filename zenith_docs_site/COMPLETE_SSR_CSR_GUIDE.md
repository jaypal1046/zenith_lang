# Zenith Framework - Complete SSR/CSR Implementation Guide

## Overview

This document provides a complete guide to the Zenith framework's Server-Side Rendering (SSR) and Client-Side Rendering (CSR) implementation, similar to Next.js.

## Architecture

```
zenith_docs_site/
├── server/
│   └── ssr-server.js          # Main SSR server (like Next.js dev server)
├── src/
│   ├── pages/                 # Page components (file-based routing)
│   │   ├── index.js           # Home page (/)
│   │   └── docs/
│   │       ├── index.js       # Docs index (/docs)
│   │       └── [slug].js      # Dynamic routes (/docs/:slug)
│   ├── components/            # Reusable UI components
│   │   ├── Layout.js          # Main layout wrapper
│   │   ├── Navigation.js      # Navigation component
│   │   ├── FeatureCard.js     # Feature card component
│   │   └── CodeBlock.js       # Syntax-highlighted code
│   ├── api/                   # API routes (like Next.js API routes)
│   │   └── docs.js            # /api/docs endpoint
│   ├── lib/
│   │   └── runtime.js         # Client-side runtime for hydration
│   └── styles/
│       ├── globals.css        # Global styles & CSS variables
│       └── components.css     # Component-specific styles
├── public/                    # Static assets
├── dist/                      # Build output
├── build.sh                   # Build commands
└── config.zenith              # Configuration file
```

## Key Features

### 1. Server-Side Rendering (SSR)

Pages are rendered on the server for fast initial page loads and SEO:

```javascript
// src/pages/index.js
export async function getServerSideProps(context) {
  const { req, query, params } = context;
  
  // Fetch data from API, database, etc.
  const data = await fetchData();
  
  return {
    data,
    metadata: {
      title: 'Home',
      description: 'Welcome to Zenith'
    }
  };
}

export default function HomePage(props) {
  const { data, metadata } = props;
  
  return `
    <div class="home-page">
      <h1>${metadata.title}</h1>
      <p>${data.message}</p>
    </div>
  `;
}
```

### 2. Client-Side Hydration

After SSR, the client takes over for SPA-like navigation:

```javascript
// Automatically handled by runtime.js
window.ZenithApp.hydrate();

// Client-side navigation
<a href="/docs" data-zenith-link>Documentation</a>
```

### 3. File-Based Routing

Routes are automatically created based on file structure:

| File | Route |
|------|-------|
| `pages/index.js` | `/` |
| `pages/docs/index.js` | `/docs` |
| `pages/docs/[slug].js` | `/docs/:slug` |
| `pages/blog/[year]/[month]/[slug].js` | `/blog/:year/:month/:slug` |

### 4. Dynamic Routes

Use bracket notation for dynamic parameters:

```javascript
// src/pages/docs/[slug].js
export async function getServerSideProps(context) {
  const { slug } = context.params;
  
  const doc = await getDocBySlug(slug);
  
  if (!doc) {
    return { notFound: true };
  }
  
  return { doc };
}

export default function DocPage({ doc }) {
  return `
    <article>
      <h1>${doc.title}</h1>
      <div>${doc.content}</div>
    </article>
  `;
}
```

### 5. API Routes

Build backend endpoints alongside your frontend:

```javascript
// src/api/docs.js
export default async function handler(context) {
  const { method, query, body } = context;
  
  if (method === 'GET') {
    const docs = await getAllDocs();
    return {
      statusCode: 200,
      body: { success: true, data: docs }
    };
  }
  
  if (method === 'POST') {
    const doc = await createDoc(body);
    return {
      statusCode: 201,
      body: { success: true, data: doc }
    };
  }
  
  return {
    statusCode: 405,
    body: { error: 'Method not allowed' }
  };
}
```

## Component System

### Creating Reusable Components

```javascript
// src/components/FeatureCard.js
export default function FeatureCard({ title, description, icon }) {
  return `
    <div class="feature-card">
      ${icon ? `<div class="icon">${icon}</div>` : ''}
      <h3>${title}</h3>
      <p>${description}</p>
    </div>
  `;
}

// Usage in pages
import FeatureCard from '../components/FeatureCard.js';

export default function Page() {
  return `
    <div>
      ${FeatureCard({ 
        title: 'Fast', 
        description: 'Blazing fast', 
        icon: '🚀' 
      })}
    </div>
  `;
}
```

### Layouts

Wrap pages with consistent layouts:

```javascript
// src/components/Layout.js
export default function Layout({ children, title }) {
  return `
    <html>
      <head><title>${title}</title></head>
      <body>
        <header>...</header>
        <main>${children}</main>
        <footer>...</footer>
      </body>
    </html>
  `;
}

// In page
export const layout = 'default';

export default function Page() {
  return `<h1>Content</h1>`;
}
```

## Styling

### CSS Variables

```css
/* src/styles/globals.css */
:root {
  --primary: #6366F1;
  --bg-dark: #0f172a;
  --text-primary: #ffffff;
  --spacing-md: 1rem;
  --radius-lg: 12px;
}
```

### Component Styles

```css
/* src/styles/components.css */
.feature-card {
  background: var(--bg-card);
  border: 1px solid var(--border-color);
  border-radius: var(--radius-lg);
  padding: var(--spacing-lg);
  transition: all var(--transition-base);
}

.feature-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 12px 24px rgba(99, 102, 241, 0.15);
}
```

## Running the Server

### Development Mode

```bash
cd zenith_docs_site
node server/ssr-server.js
```

Server starts at `http://localhost:3000`

### Using Build Script

```bash
./build.sh dev    # Start development server
./build.sh build  # Production build
./build.sh start  # Start production server
```

## Testing Endpoints

### Home Page (SSR)
```bash
curl http://localhost:3000/
```

### Documentation Index
```bash
curl http://localhost:3000/docs
```

### Dynamic Doc Page
```bash
curl http://localhost:3000/docs/getting-started
```

### API Endpoint
```bash
curl http://localhost:3000/api/docs
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
| Hot Reload | ✅ | ⏳ Planned |
| Image Optimization | ✅ | ⏳ Planned |
| TypeScript | ✅ | ⏳ Planned |

## Best Practices

### 1. Use SSR for SEO-Critical Pages
```javascript
// Good: Product pages, blog posts, documentation
export async function getServerSideProps() { ... }

// Good: Dashboard, user settings (client-only)
export default function Dashboard() { ... }
```

### 2. Optimize Data Fetching
```javascript
// Cache expensive operations
const cache = new Map();

export async function getServerSideProps() {
  if (cache.has('data')) {
    return { data: cache.get('data') };
  }
  
  const data = await expensiveFetch();
  cache.set('data', data);
  
  return { data };
}
```

### 3. Component Composition
```javascript
// ✅ Good: Small, reusable components
import FeatureCard from '../components/FeatureCard';
import CodeBlock from '../components/CodeBlock';

// ❌ Avoid: Monolithic page components
```

### 4. Proper Error Handling
```javascript
export async function getServerSideProps(context) {
  try {
    const data = await fetchData();
    return { data };
  } catch (error) {
    console.error('Failed to fetch:', error);
    return { error: 'Failed to load data' };
  }
}
```

## Migration Path from Static HTML

### Phase 1: Setup (Complete ✅)
- [x] Create SSR server
- [x] Implement file-based routing
- [x] Add component system

### Phase 2: Migrate Pages
- [ ] Convert `main.html` to page components
- [ ] Extract reusable components
- [ ] Implement layouts

### Phase 3: Add Features
- [ ] Hot reload for development
- [ ] Code splitting
- [ ] Image optimization

### Phase 4: Production Ready
- [ ] Performance optimization
- [ ] Caching strategies
- [ ] Deployment automation

## Troubleshooting

### Server Won't Start
```bash
# Check Node.js version
node --version  # Should be 14+

# Check port availability
lsof -i :3000

# Kill existing process
pkill -f "node server/ssr-server.js"
```

### Pages Not Loading
1. Verify file structure matches routing conventions
2. Check that pages export a default function
3. Ensure no syntax errors in JavaScript files

### CSS Not Applying
1. Verify CSS files are in `src/styles/`
2. Check that layout includes CSS links
3. Clear browser cache

## Next Steps

1. **Add Hot Reload**: Implement file watching for development
2. **TypeScript Support**: Add `.ts`/`.tsx` compilation
3. **Image Optimization**: Add automatic image optimization
4. **Middleware**: Add request/response middleware support
5. **Internationalization**: Add i18n routing support

## Resources

- [SSR_CSR_GUIDE.md](./SSR_CSR_GUIDE.md) - Detailed SSR/CSR guide
- [NEXTJS_FRAMEWORK.md](./NEXTJS_FRAMEWORK.md) - Framework architecture
- [ARCHITECTURE_PROPOSAL.md](./ARCHITECTURE_PROPOSAL.md) - Long-term roadmap

---

**Zenith Framework** - Modern SSR/CSR framework inspired by Next.js
