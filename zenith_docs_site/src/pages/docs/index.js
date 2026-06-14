/**
 * Documentation Index Page
 * Lists all available documentation pages
 */

// Server-side data fetching
export async function getServerSideProps(context) {
  const { query } = context;
  
  // Mock documentation index - in production, fetch from file system or database
  const docsIndex = [
    {
      id: 'getting-started',
      title: 'Getting Started',
      slug: 'getting-started',
      category: 'Introduction',
      description: 'Learn the basics of Zenith and set up your first project',
      updatedAt: '2024-01-15',
      readTime: '5 min'
    },
    {
      id: 'installation',
      title: 'Installation Guide',
      slug: 'installation',
      category: 'Introduction',
      description: 'Step-by-step installation instructions for all platforms',
      updatedAt: '2024-01-14',
      readTime: '3 min'
    },
    {
      id: 'ssr-csr',
      title: 'SSR and CSR',
      slug: 'ssr-csr',
      category: 'Rendering',
      description: 'Understanding server-side and client-side rendering strategies',
      updatedAt: '2024-01-16',
      readTime: '8 min'
    },
    {
      id: 'routing',
      title: 'File-Based Routing',
      slug: 'routing',
      category: 'Routing',
      description: 'How automatic routing works with file structure',
      updatedAt: '2024-01-13',
      readTime: '6 min'
    },
    {
      id: 'components',
      title: 'Building Components',
      slug: 'components',
      category: 'Components',
      description: 'Create reusable components with props and state',
      updatedAt: '2024-01-17',
      readTime: '10 min'
    },
    {
      id: 'styling',
      title: 'Styling Guide',
      slug: 'styling',
      category: 'Styling',
      description: 'CSS modules, global styles, and theming options',
      updatedAt: '2024-01-12',
      readTime: '7 min'
    },
    {
      id: 'api-routes',
      title: 'API Routes',
      slug: 'api-routes',
      category: 'Backend',
      description: 'Build backend endpoints with API routes',
      updatedAt: '2024-01-18',
      readTime: '9 min'
    },
    {
      id: 'deployment',
      title: 'Deployment',
      slug: 'deployment',
      category: 'Deployment',
      description: 'Deploy your Zenith app to production',
      updatedAt: '2024-01-11',
      readTime: '12 min'
    }
  ];

  // Group by category
  const categories = {};
  docsIndex.forEach(doc => {
    if (!categories[doc.category]) {
      categories[doc.category] = [];
    }
    categories[doc.category].push(doc);
  });

  // Filter by search query if provided
  let filteredDocs = docsIndex;
  if (query.search) {
    const searchTerm = query.search.toLowerCase();
    filteredDocs = docsIndex.filter(doc => 
      doc.title.toLowerCase().includes(searchTerm) ||
      doc.description.toLowerCase().includes(searchTerm) ||
      doc.category.toLowerCase().includes(searchTerm)
    );
  }

  return {
    docs: filteredDocs,
    categories,
    totalDocs: docsIndex.length,
    searchQuery: query.search || ''
  };
}

// Page component
export default function DocsIndexPage(props) {
  const { docs, categories, totalDocs, searchQuery } = props;

  return `
    <div class="docs-index-page">
      <header class="docs-header">
        <h1 data-page-title>Documentation</h1>
        <p class="docs-subtitle">Everything you need to know about Zenith</p>
        
        <!-- Search Bar -->
        <form class="docs-search" role="search">
          <input 
            type="search" 
            name="search" 
            placeholder="Search documentation..." 
            value="${searchQuery}"
            aria-label="Search documentation"
          />
          <button type="submit" class="btn btn-icon">
            <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <circle cx="11" cy="11" r="8"/>
              <path d="m21 21-4.35-4.35"/>
            </svg>
          </button>
        </form>
      </header>

      <main class="docs-content">
        <!-- Sidebar Navigation -->
        <aside class="docs-sidebar">
          <nav class="docs-nav">
            <h3>Categories</h3>
            <ul>
              ${Object.keys(categories).map(cat => `
                <li>
                  <a href="#${cat.toLowerCase()}" data-zenith-link>
                    ${cat}
                    <span class="count">${categories[cat].length}</span>
                  </a>
                </li>
              `).join('')}
            </ul>
            
            <div class="sidebar-extra">
              <h4>Resources</h4>
              <ul>
                <li><a href="/api/docs" data-zenith-link>API Reference</a></li>
                <li><a href="/examples" data-zenith-link>Examples</a></li>
                <li><a href="https://github.com/zenith" target="_blank">GitHub</a></li>
              </ul>
            </div>
          </nav>
        </aside>

        <!-- Main Content -->
        <div class="docs-main">
          ${Object.entries(categories).map(([category, categoryDocs]) => `
            <section id="${category.toLowerCase()}" class="docs-section">
              <h2>${category}</h2>
              <div class="docs-grid">
                ${categoryDocs.map(doc => `
                  <article class="doc-card">
                    <div class="doc-card-header">
                      <h3>
                        <a href="/docs/${doc.slug}" data-zenith-link>${doc.title}</a>
                      </h3>
                      <span class="read-time">${doc.readTime}</span>
                    </div>
                    <p class="doc-description">${doc.description}</p>
                    <div class="doc-meta">
                      <span class="updated">Updated ${new Date(doc.updatedAt).toLocaleDateString()}</span>
                    </div>
                  </article>
                `).join('')}
              </div>
            </section>
          `).join('')}

          ${docs.length === 0 ? `
            <div class="no-results">
              <h3>No results found</h3>
              <p>Try adjusting your search terms or browse all documentation.</p>
              <a href="/docs" class="btn" data-zenith-link>View All Docs</a>
            </div>
          ` : ''}
        </div>
      </main>

      <!-- Quick Access Footer -->
      <footer class="docs-footer">
        <div class="container">
          <div class="footer-grid">
            <div class="footer-col">
              <h4>Getting Started</h4>
              <ul>
                <li><a href="/docs/installation" data-zenith-link>Installation</a></li>
                <li><a href="/docs/getting-started" data-zenith-link>Quick Start</a></li>
                <li><a href="/examples" data-zenith-link>Examples</a></li>
              </ul>
            </div>
            <div class="footer-col">
              <h4>Core Concepts</h4>
              <ul>
                <li><a href="/docs/routing" data-zenith-link>Routing</a></li>
                <li><a href="/docs/ssr-csr" data-zenith-link>Rendering</a></li>
                <li><a href="/docs/components" data-zenith-link>Components</a></li>
              </ul>
            </div>
            <div class="footer-col">
              <h4>Advanced</h4>
              <ul>
                <li><a href="/docs/api-routes" data-zenith-link>API Routes</a></li>
                <li><a href="/docs/deployment" data-zenith-link>Deployment</a></li>
                <li><a href="/docs/styling" data-zenith-link>Styling</a></li>
              </ul>
            </div>
            <div class="footer-col">
              <h4>Community</h4>
              <ul>
                <li><a href="https://github.com/zenith" target="_blank">GitHub</a></li>
                <li><a href="https://discord.gg/zenith" target="_blank">Discord</a></li>
                <li><a href="https://twitter.com/zenith" target="_blank">Twitter</a></li>
              </ul>
            </div>
          </div>
        </div>
      </footer>
    </div>
  `;
}

// Layout specification
export const layout = 'docs';

// Page metadata
export const metadata = {
  title: 'Documentation',
  description: 'Complete documentation for Zenith framework',
  keywords: ['documentation', 'guide', 'tutorial', 'api']
};
