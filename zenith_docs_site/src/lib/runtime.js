/**
 * Zenith Framework Runtime
 * Handles SSR hydration, component rendering, and routing
 */

class ZenithRuntime {
  constructor(config = {}) {
    this.config = config;
    this.components = new Map();
    this.pages = new Map();
    this.state = new Map();
    this.hydrated = false;
  }

  // Register a component
  registerComponent(name, component) {
    this.components.set(name, component);
  }

  // Register a page
  registerPage(path, pageModule) {
    this.pages.set(path, pageModule);
  }

  // Get current state
  getState(key) {
    return this.state.get(key);
  }

  // Set state
  setState(key, value) {
    this.state.set(key, value);
    if (this.hydrated) {
      this.render();
    }
  }

  // Render component to string (SSR)
  renderToString(componentName, props = {}) {
    const component = this.components.get(componentName);
    if (!component) {
      console.warn(`Component "${componentName}" not found`);
      return '';
    }
    
    if (typeof component === 'function') {
      return component(props);
    }
    
    if (component.render) {
      return component.render(props);
    }
    
    return String(component);
  }

  // Render page to string (SSR)
  async renderPage(path, context = {}) {
    const pageModule = this.pages.get(path);
    if (!pageModule) {
      return { status: 404, html: '<h1>404 - Page Not Found</h1>' };
    }

    try {
      let html;
      if (pageModule.default) {
        // ES module default export
        const PageComponent = pageModule.default;
        html = typeof PageComponent === 'function' 
          ? PageComponent(context) 
          : String(PageComponent);
      } else if (pageModule.render) {
        // CommonJS style
        html = pageModule.render(context);
      } else {
        html = String(pageModule);
      }

      return {
        status: 200,
        html: this.wrapWithLayout(html, pageModule.layout || 'default', context)
      };
    } catch (error) {
      console.error(`Error rendering page ${path}:`, error);
      return {
        status: 500,
        html: `<h1>Server Error</h1><pre>${error.message}</pre>`
      };
    }
  }

  // Wrap content with layout
  wrapWithLayout(content, layoutName, context = {}) {
    const layout = this.components.get(layoutName) || this.components.get('default');
    if (!layout) {
      return content;
    }

    if (typeof layout === 'function') {
      return layout({ ...context, children: content });
    }

    if (layout.render) {
      return layout.render({ ...context, children: content });
    }

    return content;
  }

  // Client-side: Hydrate the application
  hydrate() {
    if (this.hydrated) return;
    
    this.hydrated = true;
    console.log('[Zenith] Application hydrated');

    // Initialize router
    this.initRouter();
    
    // Attach event listeners
    this.attachEventListeners();
    
    // Render initial view
    this.render();
  }

  // Initialize client-side routing
  initRouter() {
    const router = window.ZenithRouter;
    if (!router) {
      console.warn('[Zenith] Router not found');
      return;
    }

    router.on('navigate', (path) => {
      this.loadPage(path);
    });

    // Handle browser back/forward
    window.addEventListener('popstate', () => {
      const path = window.location.pathname;
      this.loadPage(path, false);
    });
  }

  // Load page on client
  async loadPage(path, pushState = true) {
    try {
      // Check if we have the page cached
      let pageModule = this.pages.get(path);
      
      if (!pageModule) {
        // Dynamic import for code splitting
        const modulePath = `/js/pages${path}.js`;
        pageModule = await import(modulePath);
        this.registerPage(path, pageModule);
      }

      const context = { path, isClient: true };
      const result = await this.renderPage(path, context);
      
      if (result.status === 200) {
        document.getElementById('app').innerHTML = result.html;
        
        if (pushState) {
          history.pushState({ path }, '', path);
        }
        
        // Trigger page load events
        this.onPageLoad(path);
      } else {
        document.getElementById('app').innerHTML = result.html;
      }
    } catch (error) {
      console.error('[Zenith] Error loading page:', error);
      document.getElementById('app').innerHTML = 
        `<h1>Error</h1><p>Failed to load page: ${path}</p>`;
    }
  }

  // Page load lifecycle
  onPageLoad(path) {
    // Update document title
    const titleElement = document.querySelector('title');
    if (titleElement) {
      const pageTitle = document.querySelector('[data-page-title]');
      if (pageTitle) {
        titleElement.textContent = `${pageTitle.textContent} | Zenith Docs`;
      }
    }

    // Re-attach event listeners for new content
    this.attachEventListeners();
    
    // Scroll to top
    window.scrollTo(0, 0);
  }

  // Attach global event listeners
  attachEventListeners() {
    // Handle internal links
    document.querySelectorAll('a[data-zenith-link]').forEach(link => {
      link.addEventListener('click', (e) => {
        e.preventDefault();
        const href = link.getAttribute('href');
        if (href && href.startsWith('/')) {
          window.ZenithRouter?.navigate(href);
        }
      });
    });

    // Handle forms with data-zenith-submit
    document.querySelectorAll('form[data-zenith-submit]').forEach(form => {
      form.addEventListener('submit', async (e) => {
        e.preventDefault();
        const action = form.getAttribute('action') || '/api/submit';
        const method = form.getAttribute('method') || 'POST';
        const formData = new FormData(form);
        
        try {
          const response = await fetch(action, {
            method,
            body: formData,
            headers: {
              'Accept': 'application/json'
            }
          });
          
          const result = await response.json();
          this.handleFormResult(result);
        } catch (error) {
          console.error('[Zenith] Form submission error:', error);
        }
      });
    });
  }

  // Handle form submission result
  handleFormResult(result) {
    if (result.redirect) {
      window.ZenithRouter?.navigate(result.redirect);
    } else if (result.message) {
      this.showNotification(result.message, result.type || 'info');
    }
  }

  // Show notification
  showNotification(message, type = 'info') {
    const container = document.getElementById('notifications') || this.createNotificationContainer();
    
    const notification = document.createElement('div');
    notification.className = `notification notification-${type}`;
    notification.textContent = message;
    
    container.appendChild(notification);
    
    setTimeout(() => {
      notification.classList.add('notification-hide');
      setTimeout(() => notification.remove(), 300);
    }, 3000);
  }

  // Create notification container if it doesn't exist
  createNotificationContainer() {
    const container = document.createElement('div');
    container.id = 'notifications';
    container.className = 'notifications-container';
    document.body.appendChild(container);
    return container;
  }

  // Main render method (client-side)
  render() {
    // Re-render components that depend on state
    this.components.forEach((component, name) => {
      const elements = document.querySelectorAll(`[data-component="${name}"]`);
      elements.forEach(element => {
        const props = JSON.parse(element.getAttribute('data-props') || '{}');
        const mergedProps = { ...props, state: Object.fromEntries(this.state) };
        element.innerHTML = this.renderToString(name, mergedProps);
      });
    });
  }
}

// Global router implementation
class ZenithRouter {
  constructor() {
    this.listeners = new Map();
    this.currentPath = window.location.pathname;
  }

  on(event, callback) {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, []);
    }
    this.listeners.get(event).push(callback);
  }

  emit(event, data) {
    const callbacks = this.listeners.get(event) || [];
    callbacks.forEach(cb => cb(data));
  }

  navigate(path, options = {}) {
    if (path === this.currentPath && !options.force) {
      return;
    }

    this.currentPath = path;
    this.emit('navigate', path);

    if (options.replace) {
      history.replaceState({ path }, '', path);
    } else {
      history.pushState({ path }, '', path);
    }
  }
}

// Initialize global instances
window.ZenithRouter = new ZenithRouter();
window.ZenithApp = new ZenithRuntime();

// Auto-hydrate when DOM is ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => {
    window.ZenithApp.hydrate();
  });
} else {
  window.ZenithApp.hydrate();
}

// Export for module usage
if (typeof module !== 'undefined' && module.exports) {
  module.exports = { ZenithRuntime, ZenithRouter };
}
