/**
 * Default Layout Component
 * Wraps all pages with consistent header, navigation, and footer
 */

export default function Layout({ children, title = 'Zenith Docs', description = '' }) {
  return `
    <div class="layout-wrapper">
      <!-- Header -->
      <header class="site-header">
        <div class="container header-content">
          <a href="/" class="logo" data-zenith-link>
            <span class="logo-icon">⚡</span>
            <span class="logo-text">Zenith</span>
          </a>
          
          <nav class="main-nav">
            <a href="/docs" data-zenith-link>Documentation</a>
            <a href="/docs/getting-started" data-zenith-link>Getting Started</a>
            <a href="/api/docs" target="_blank">API</a>
            <a href="https://github.com/zenith" target="_blank" rel="noopener">GitHub</a>
          </nav>
          
          <div class="header-actions">
            <button class="theme-toggle" aria-label="Toggle theme">
              <svg width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor">
                <circle cx="12" cy="12" r="5"/>
                <path d="M12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42"/>
              </svg>
            </button>
          </div>
        </div>
      </header>

      <!-- Main Content -->
      <main class="site-main">
        ${children}
      </main>

      <!-- Footer -->
      <footer class="site-footer">
        <div class="container">
          <div class="footer-grid">
            <div class="footer-section">
              <h4>Zenith</h4>
              <p>Modern web framework for Rust & Dart developers</p>
            </div>
            <div class="footer-section">
              <h4>Resources</h4>
              <ul>
                <li><a href="/docs" data-zenith-link>Documentation</a></li>
                <li><a href="/docs/getting-started" data-zenith-link>Getting Started</a></li>
                <li><a href="/docs/api" data-zenith-link>API Reference</a></li>
              </ul>
            </div>
            <div class="footer-section">
              <h4>Community</h4>
              <ul>
                <li><a href="https://github.com/zenith" target="_blank">GitHub</a></li>
                <li><a href="https://discord.gg/zenith" target="_blank">Discord</a></li>
                <li><a href="https://twitter.com/zenith" target="_blank">Twitter</a></li>
              </ul>
            </div>
          </div>
          <div class="footer-bottom">
            <p>&copy; ${new Date().getFullYear()} Zenith Framework. MIT License.</p>
          </div>
        </div>
      </footer>
    </div>
  `;
}

// Layout metadata
export const layoutName = 'default';
