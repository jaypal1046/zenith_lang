/**
 * Code Block Component
 * Syntax-highlighted code display with copy button
 */

export default function CodeBlock({ code, language = 'javascript', title = '', showLineNumbers = true }) {
  const highlightedCode = highlightSyntax(code, language);
  
  return `
    <div class="code-block-component" data-language="${language}">
      ${title ? `
        <div class="code-header">
          <span class="code-title">${title}</span>
          <button class="copy-button" onclick="copyCode(this)" aria-label="Copy code">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor">
              <rect x="9" y="9" width="13" height="13" rx="2" ry="2"/>
              <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
            </svg>
          </button>
        </div>
      ` : ''}
      <pre class="code-pre ${showLineNumbers ? 'line-numbers' : ''}"><code class="code-lang-${language}">${highlightedCode}</code></pre>
    </div>
  `;
}

// Simple syntax highlighting (in production, use Prism.js or Highlight.js)
function highlightSyntax(code, language) {
  // Escape HTML
  let escaped = code
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');

  // Basic highlighting patterns
  const patterns = {
    javascript: [
      { regex: /(\/\/.*$)/gm, class: 'code-comment' },
      { regex: /\b(const|let|var|function|return|if|else|for|while|class|import|export|from|async|await)\b/g, class: 'code-keyword' },
      { regex: /(['"`].*?['"`])/g, class: 'code-string' },
      { regex: /\b(\d+)\b/g, class: 'code-number' },
      { regex: /\b([A-Z][a-zA-Z0-9]*)\b/g, class: 'code-class' }
    ],
    bash: [
      { regex: /(#.*$)/gm, class: 'code-comment' },
      { regex: /\b(cd|ls|npm|node|yarn|git|docker|curl|wget)\b/g, class: 'code-keyword' },
      { regex: /(['"].*?['"])/g, class: 'code-string' }
    ],
    rust: [
      { regex: /(\/\/.*$)/gm, class: 'code-comment' },
      { regex: /\b(fn|let|mut|pub|struct|impl|use|mod|return|if|else|match|trait|where)\b/g, class: 'code-keyword' },
      { regex: /(['"].*?['"])/g, class: 'code-string' }
    ]
  };

  const langPatterns = patterns[language] || patterns.javascript;
  
  langPatterns.forEach(({ regex, class: className }) => {
    escaped = escaped.replace(regex, `<span class="${className}">$1</span>`);
  });

  return escaped;
}

// Inline code component
export function InlineCode({ code }) {
  return `<code class="inline-code">${code}</code>`;
}

// Copy to clipboard function (will be added to global scope)
globalThis.copyCode = function(button) {
  const codeBlock = button.closest('.code-block-component').querySelector('code');
  const code = codeBlock.textContent;
  
  navigator.clipboard.writeText(code).then(() => {
    button.classList.add('copied');
    setTimeout(() => button.classList.remove('copied'), 2000);
  });
};
