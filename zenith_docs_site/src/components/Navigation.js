/**
 * Navigation Component
 * Reusable navigation with active state and responsive menu
 */

export default function Navigation({ items = [], currentPage = '' }) {
  return `
    <nav class="component-nav">
      <ul class="nav-list">
        ${items.map(item => `
          <li class="nav-item ${item.href === currentPage ? 'active' : ''}">
            <a href="${item.href}" ${item.external ? 'target="_blank" rel="noopener"' : 'data-zenith-link'}>
              ${item.icon ? `<span class="nav-icon">${item.icon}</span>` : ''}
              <span class="nav-label">${item.label}</span>
              ${item.badge ? `<span class="nav-badge">${item.badge}</span>` : ''}
            </a>
          </li>
        `).join('')}
      </ul>
    </nav>
  `;
}

// Example usage data
export const defaultNavItems = [
  { label: 'Home', href: '/', icon: '🏠' },
  { label: 'Docs', href: '/docs', icon: '📚' },
  { label: 'API', href: '/api', icon: '🔌' },
  { label: 'Blog', href: '/blog', icon: '✍️' },
  { label: 'GitHub', href: 'https://github.com/zenith', external: true, icon: '🐙' }
];
