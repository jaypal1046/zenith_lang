/**
 * Example Dynamic Route Page
 * Demonstrates dynamic routing with [slug].js pattern (like Next.js)
 */

// Server-side data fetching for dynamic routes
export async function getServerSideProps(context) {
  const { params, query } = context;
  
  // Mock documentation content - in production, fetch from database or file system
  const docContent = {
    'getting-started': {
      title: 'Getting Started with Zenith',
      slug: 'getting-started',
      category: 'Introduction',
      updatedAt: '2024-01-15',
      content: `
        <h2>Introduction</h2>
        <p>Welcome to Zenith! This guide will help you get started with building modern web applications.</p>
        
        <h3>Prerequisites</h3>
        <ul>
          <li>Node.js 18.0 or later</li>
          <li>Basic understanding of JavaScript/TypeScript</li>
          <li>Familiarity with React concepts (optional)</li>
        </ul>
        
        <h3>Installation</h3>
        <pre><code>npm install -g @zenith/cli
zenith create my-app</code></pre>
        
        <h3>Project Structure</h3>
        <p>Zenith uses a file-based routing system similar to Next.js:</p>
        <pre><code>src/
├── pages/
│   ├── index.js
│   └── docs/
│       └── [slug].js
├── components/
├── layouts/
└── api/</code></pre>
      `
    },
    'installation': {
      title: 'Installation Guide',
      slug: 'installation',
      category: 'Introduction',
      updatedAt: '2024-01-14',
      content: `
        <h2>Installation Options</h2>
        <p>Zenith can be installed in several ways depending on your needs.</p>
        
        <h3>Option 1: CLI Installation</h3>
        <pre><code>npm install -g @zenith/cli
zenith create my-app</code></pre>
        
        <h3>Option 2: Manual Setup</h3>
        <pre><code>npm install @zenith/core @zenith/router
mkdir -p src/pages src/components</code></pre>
        
        <h3>Verification</h3>
        <p>Run <code>zenith --version</code> to verify installation.</p>
      `
    },
    'ssr-csr': {
      title: 'SSR and CSR Explained',
      slug: 'ssr-csr',
      category: 'Rendering',
      updatedAt: '2024-01-16',
      content: `
        <h2>Understanding Rendering Strategies</h2>
        <p>Zenith supports both Server-Side Rendering (SSR) and Client-Side Rendering (CSR).</p>
        
        <h3>Server-Side Rendering (SSR)</h3>
        <p>Use <code>getServerSideProps</code> to fetch data on the server:</p>
        <pre><code>export async function getServerSideProps(context) {
  const data = await fetchData();
  return { data };
}

export default function Page({ data }) {
  return &lt;div&gt;{data}&lt;/div&gt;;
}</code></pre>
        
        <h3>Client-Side Rendering (CSR)</h3>
        <p>For client-only data fetching, use useEffect or similar hooks:</p>
        <pre><code>export default function Page() {
  const [data, setData] = useState(null);
  
  useEffect(() => {
    fetchData().then(setData);
  }, []);
  
  return &lt;div&gt;{data}&lt;/div&gt;;
}</code></pre>
      `
    },
    'routing': {
      title: 'File-Based Routing',
      slug: 'routing',
      category: 'Routing',
      updatedAt: '2024-01-13',
      content: `
        <h2>File-Based Routing System</h2>
        <p>Zenith automatically creates routes based on your file structure.</p>
        
        <h3>Basic Routes</h3>
        <pre><code>src/pages/index.js      → /
src/pages/about.js      → /about
src/pages/docs/index.js → /docs</code></pre>
        
        <h3>Dynamic Routes</h3>
        <p>Use square brackets for dynamic parameters:</p>
        <pre><code>src/pages/docs/[slug].js  → /docs/:slug
src/pages/users/[id].js   → /users/:id</code></pre>
        
        <h3>Nested Dynamic Routes</h3>
        <pre><code>src/pages/blog/[year]/[month]/[slug].js
→ /blog/:year/:month/:slug</code></pre>
      `
    }
  };

  const doc = docContent[params?.slug] || docContent['getting-started'];
  
  if (!doc) {
    return {
      notFound: true
    };
  }

  return {
    doc,
    relatedDocs: Object.values(docContent).filter(d => 
      d.slug !== params?.slug && d.category === doc.category
    ).slice(0, 2),
    lastUpdated: new Date(doc.updatedAt).toLocaleDateString('en-US', {
      year: 'numeric',
      month: 'long',
      day: 'numeric'
    })
  };
}

// Page component
export default function DocPage(props) {
  const { doc, relatedDocs, lastUpdated } = props;
  
  if (!doc) {
    return '<div class="error-page"><h1>Document Not Found</h1></div>';
  }

  return `
    <div class="doc-page">
      <nav class="breadcrumb" aria-label="Breadcrumb">
        <a href="/" data-zenith-link>Home</a>
        <span>/</span>
        <a href="/docs" data-zenith-link>Documentation</a>
        <span>/</span>
        <span>${doc.title}</span>
      </nav>

      <article class="doc-content">
        <header>
          <h1 data-page-title>${doc.title}</h1>
          <div class="meta">
            <span class="category">${doc.category}</span>
            <span class="updated">Last updated: ${lastUpdated}</span>
          </div>
        </header>

        <div class="content">
          ${doc.content}
        </div>

        <footer class="doc-footer">
          <div class="edit-links">
            <a href="#" class="btn btn-sm">Edit this page</a>
            <a href="#" class="btn btn-sm">View source</a>
          </div>
        </footer>
      </article>

      ${relatedDocs && relatedDocs.length > 0 ? `
        <aside class="related-docs">
          <h2>Related Documentation</h2>
          <ul>
            ${relatedDocs.map(doc => `
              <li>
                <a href="/docs/${doc.slug}" data-zenith-link>
                  <span class="category">${doc.category}</span>
                  <span class="title">${doc.title}</span>
                </a>
              </li>
            `).join('')}
          </ul>
        </aside>
      ` : ''}

      <nav class="pagination">
        <a href="/docs" class="btn" data-zenith-link>← Back to Docs</a>
      </nav>
    </div>
  `;
}

// Layout specification
export const layout = 'docs';

// Page metadata
export const metadata = {
  description: 'Zenith Documentation',
  ogType: 'article'
};
