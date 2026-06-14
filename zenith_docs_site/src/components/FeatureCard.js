/**
 * Feature Card Component
 * Reusable card for displaying features with icons
 */

export default function FeatureCard({ title, description, icon, href = null }) {
  const content = `
    <div class="feature-card-component">
      ${icon ? `<div class="feature-icon">${icon}</div>` : ''}
      ${title ? `<h3 class="feature-title">${title}</h3>` : ''}
      ${description ? `<p class="feature-description">${description}</p>` : ''}
      ${href ? `<a href="${href}" class="feature-link" data-zenith-link>Learn more →</a>` : ''}
    </div>
  `;

  if (href) {
    return `<a href="${href}" class="feature-card-wrapper" data-zenith-link>${content}</a>`;
  }

  return content;
}

// Feature card grid component
export function FeatureGrid({ features = [] }) {
  return `
    <div class="feature-grid-component">
      ${features.map(feature => FeatureCard(feature)).join('')}
    </div>
  `;
}

// Example features
export const exampleFeatures = [
  {
    icon: '🚀',
    title: 'Fast Performance',
    description: 'Blazing fast page loads with server-side rendering and optimized bundling'
  },
  {
    icon: '⚡',
    title: 'Hot Reload',
    description: 'Instant feedback during development with hot module replacement'
  },
  {
    icon: '📦',
    title: 'Zero Config',
    description: 'Start building immediately with sensible defaults and automatic optimization'
  },
  {
    icon: '🔒',
    title: 'Type Safe',
    description: 'Full TypeScript support for better developer experience and fewer bugs'
  }
];
