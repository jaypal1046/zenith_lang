/**
 * Example Home Page Component
 * Demonstrates SSR with getServerSideProps (like Next.js)
 */

// Server-side data fetching (executes only on server)
export async function getServerSideProps(context) {
  // This runs on the server before rendering
  const { req, query, params } = context;
  
  // Fetch data from API, database, etc.
  const features = [
    {
      title: 'Server-Side Rendering',
      description: 'Fast initial page loads with full HTML rendered on the server',
      icon: '🚀'
    },
    {
      title: 'Client-Side Hydration',
      description: 'Interactive SPA experience after initial load',
      icon: '⚡'
    },
    {
      title: 'File-Based Routing',
      description: 'Automatic routes based on file structure',
      icon: '📁'
    },
    {
      title: 'API Routes',
      description: 'Build full-stack applications with backend endpoints',
      icon: '🔌'
    }
  ];

  const metadata = {
    title: 'Home',
    description: 'Zenith Documentation - Modern Framework for Rust & Dart',
    buildTime: new Date().toISOString(),
    isServer: true
  };

  return {
    features,
    metadata,
    serverInfo: {
      userAgent: req?.headers?.['user-agent'] || 'Unknown',
      timestamp: Date.now()
    }
  };
}

// Page component (renders on both server and client)
export default function HomePage(props) {
  const { features, metadata, serverInfo } = props;

  return `
    <div class="home-page">
      <!-- Hero Section -->
      <section class="hero">
        <div class="container">
          <h1 data-page-title>${metadata.title}</h1>
          <p class="hero-subtitle">${metadata.description}</p>
          <div class="hero-cta">
            <a href="/docs/getting-started" class="btn btn-primary" data-zenith-link>Get Started</a>
            <a href="/docs/api" class="btn btn-secondary" data-zenith-link>View API</a>
          </div>
          ${serverInfo ? `<p class="server-info"><small>Rendered on server at ${new Date(serverInfo.timestamp).toLocaleTimeString()}</small></p>` : ''}
        </div>
      </section>

      <!-- Features Grid -->
      <section class="features-section">
        <div class="container">
          <h2>Why Zenith?</h2>
          <div class="features-grid">
            ${features.map(feature => `
              <div class="feature-card">
                <div class="feature-icon">${feature.icon}</div>
                <h3>${feature.title}</h3>
                <p>${feature.description}</p>
              </div>
            `).join('')}
          </div>
        </div>
      </section>

      <!-- Quick Start Code -->
      <section class="quickstart-section">
        <div class="container">
          <h2>Quick Start</h2>
          <div class="code-block">
            <pre><code class="language-bash"># Install Zenith CLI
npm install -g @zenith/cli

# Create new project
zenith create my-app

# Start development server
cd my-app
zenith dev</code></pre>
          </div>
        </div>
      </section>

      <!-- Ecosystem -->
      <section class="ecosystem-section">
        <div class="container">
          <h2>Built for Rust & Dart</h2>
          <div class="ecosystem-grid">
            <div class="ecosystem-card rust">
              <h3>🦀 Rust Bridge</h3>
              <p>High-performance native modules with zero-cost abstractions</p>
              <ul>
                <li>WebAssembly compilation</li>
                <li>FFI bindings</li>
                <li>Memory safety guarantees</li>
              </ul>
            </div>
            <div class="ecosystem-card dart">
              <h3>🎯 Dart Integration</h3>
              <p>Seamless Flutter and web app development</p>
              <ul>
                <li>Hot reload support</li>
                <li>Type-safe interop</li>
                <li>Rich widget ecosystem</li>
              </ul>
            </div>
          </div>
        </div>
      </section>

      <!-- Call to Action -->
      <section class="cta-section">
        <div class="container">
          <h2>Ready to Build?</h2>
          <p>Join thousands of developers building with Zenith</p>
          <div class="cta-buttons">
            <a href="/docs/installation" class="btn btn-large btn-primary" data-zenith-link>Start Building</a>
            <a href="https://github.com/zenith" class="btn btn-large btn-outline" target="_blank">View on GitHub</a>
          </div>
        </div>
      </section>
    </div>
  `;
}

// Layout specification (optional)
export const layout = 'default';

// Page metadata for SEO
export const metadata = {
  title: 'Zenith - Modern Web Framework',
  description: 'Build fast, scalable web applications with Rust and Dart',
  keywords: ['rust', 'dart', 'webassembly', 'framework', 'ssr'],
  ogImage: '/images/og-home.png'
};
