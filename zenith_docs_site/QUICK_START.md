# 🚀 Quick Start Guide - Zenith SSR Framework

Get your Next.js-like documentation site running in under 2 minutes!

## Prerequisites

- Node.js 16+ installed
- Modern web browser

## Installation & Setup

### 1. Start Development Server

```bash
cd zenith_docs_site
./dev-hot-reload.sh
```

Or manually:
```bash
node server/ssr-server.js
```

Your site will be available at: **http://localhost:3000**

### 2. Explore the Demo

Visit these URLs to see different features:

- **Home Page** (SSR): http://localhost:3000/
- **Documentation** (SSR): http://localhost:3000/docs
- **Dynamic Doc** (SSR): http://localhost:3000/docs/getting-started
- **API Endpoint** (JSON): http://localhost:3000/api/docs
- **Interactive Demo**: http://localhost:3000/web/demo.html

### 3. Test Client-Side Features

1. Open the interactive demo at `/web/demo.html`
2. Click the increment/decrement buttons
3. Notice no page reloads when navigating with "Go to Docs" buttons
4. Check browser console for hydration logs

## Project Structure

```
zenith_docs_site/
├── server/
│   └── ssr-server.js          # Main SSR server
├── src/
│   ├── pages/                 # Your pages (file-based routing)
│   │   ├── index.js           # → /
│   │   └── docs/
│   │       ├── index.js       # → /docs
│   │       └── [slug].js      # → /docs/:slug
│   ├── api/                   # API routes
│   │   └── docs.js            # → /api/docs
│   ├── components/            # Reusable UI components
│   ├── lib/
│   │   └── runtime.js         # Client-side hydration
│   └── styles/                # CSS files
├── web/
│   └── demo.html              # Interactive demo page
└── build.sh                   # Build commands
```

## Create Your First Page

Create a new file `src/pages/about.js`:

```javascript
export default function AboutPage() {
  return `
    <div class="container">
      <h1>About Zenith</h1>
      <p>Built with love and SSR magic ✨</p>
    </div>
  `;
}

export async function getServerSideProps() {
  return {
    props: {
      title: 'About Us'
    }
  };
}
```

Visit: http://localhost:3000/about

## Add Dynamic Routes

Create `src/pages/users/[id].js`:

```javascript
export default function UserPage({ params }) {
  return `
    <div>
      <h1>User Profile: ${params.id}</h1>
      <p>Welcome, user ${params.id}!</p>
    </div>
  `;
}
```

Visit: http://localhost:3000/users/123

## Create API Endpoint

Create `src/api/hello.js`:

```javascript
export default function handler(req, res) {
  res.json({ 
    message: 'Hello from Zenith API!',
    timestamp: new Date().toISOString()
  });
}
```

Test: `curl http://localhost:3000/api/hello`

## Production Build

```bash
# Build for production
./build.sh build

# Start production server
./build.sh start
```

## Hot Reload

In development mode, changes automatically reload:
- Edit any `.js` file in `src/`
- Save the file
- Browser updates instantly!

## Troubleshooting

**Port already in use?**
```bash
lsof -ti:3000 | xargs kill
```

**Module not found?**
```bash
npm install  # if using npm packages
```

**Need help?**
Check `COMPLETE_SSR_CSR_GUIDE.md` for detailed documentation.

---

🎉 You're ready to build amazing documentation with SSR + CSR!
