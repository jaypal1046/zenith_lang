# Zenith Docs Site - Modern Architecture Proposal

## Problem Statement

Currently, the Zenith documentation site generates monolithic HTML files (`main.html`, `main_wasm.html`) that:
- Contain all code in a single file (44KB+ of inline HTML/CSS/JS)
- Have completely different UIs despite using the same source code
- Are difficult to maintain and extend
- Don't follow modern web development practices (like React/Next.js)
- Lack component reusability across pages
- Have no proper routing or state management

## Solution: Component-Based Architecture

We propose restructuring the project to follow a modern, modular architecture similar to React/Next.js.

### Proposed Directory Structure

```
zenith_docs_site/
├── src/                          # Source code (Zenith components)
│   ├── components/               # Reusable UI components
│   │   ├── common/              # Shared components
│   │   │   ├── Button.zenith
│   │   │   ├── Card.zenith
│   │   │   ├── Navbar.zenith
│   │   │   └── Footer.zenith
│   │   ├── layout/              # Layout components
│   │   │   ├── PageLayout.zenith
│   │   │   └── Section.zenith
│   │   └── content/             # Content-specific components
│   │       ├── CodeBlock.zenith
│   │       └── PlatformCard.zenith
│   ├── pages/                   # Page components (routes)
│   │   ├── index.zenith         # Home page
│   │   ├── docs/
│   │   │   ├── index.zenith     # Docs listing
│   │   │   └── [slug].zenith    # Dynamic doc pages
│   │   ├── cookbook/
│   │   │   ├── index.zenith
│   │   │   └── [recipe].zenith
│   │   └── platforms.zenith
│   ├── layouts/                 # Page layouts
│   │   ├── DefaultLayout.zenith
│   │   └── DocsLayout.zenith
│   ├── lib/                     # Utilities & helpers
│   │   ├── routing.zenith
│   │   ├── state.zenith
│   │   └── theme.zenith
│   └── main.zenith              # Application entry point
├── content/                     # Markdown/content files
│   ├── docs/                    # Documentation markdown
│   ├── cookbook/                # Recipe markdown
│   └── legal/                   # Legal documents
├── public/                      # Static assets
│   ├── images/
│   ├── fonts/
│   └── icons/
├── styles/                      # Global styles
│   ├── variables.css
│   ├── global.css
│   └── themes/
│       ├── light.css
│       └── dark.css
├── config/                      # Configuration files
│   ├── site.config.zenith
│   └── build.config.zenith
├── web/                         # Build output
│   ├── index.html              # Entry point (minimal)
│   ├── js/                     # Compiled JavaScript
│   │   ├── main.js
│   │   ├── chunks/            # Code-split bundles
│   │   └── vendors.js         # Third-party libs
│   ├── css/                    # Compiled stylesheets
│   │   ├── main.css
│   │   └── themes/
│   ├── wasm/                   # WASM binaries
│   │   └── app.wasm
│   └── assets/                 # Copied static assets
├── tools/                       # Build tools & scripts
│   ├── builder.zenith
│   ├── dev-server.zenith
│   └── optimizer.zenith
└── package.yaml                 # Dependencies & metadata
```

### Key Improvements

#### 1. **Component-Based Architecture**
```zenith
// src/components/common/Button.zenith
import 'package:zenith_ui/zenith_ui.dart';

class Button extends StatelessWidget {
  final String label;
  final VoidCallback onPressed;
  final ButtonStyle style;
  
  const Button({
    required this.label,
    required this.onPressed,
    this.style = ButtonStyle.primary,
  });
  
  @override
  Widget build(BuildContext context) {
    return ElevatedButton(
      onPressed: onPressed,
      style: ElevatedButton.styleFrom(
        backgroundColor: style == ButtonStyle.primary 
            ? Theme.of(context).primaryColor 
            : Colors.transparent,
      ),
      child: Text(label),
    );
  }
}
```

#### 2. **Page-Based Routing** (Next.js style)
```zenith
// src/pages/index.zenith
import '../components/layout/PageLayout.dart';
import '../components/content/HeroSection.dart';

class HomePage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return PageLayout(
      children: [
        HeroSection(),
        PlatformShowcase(),
        FeatureGrid(),
      ],
    );
  }
}

// src/pages/docs/[slug].zenith - Dynamic routing
class DocPage extends StatelessWidget {
  final String slug;
  
  const DocPage({required this.slug});
  
  @override
  Widget build(BuildContext context) {
    return DocsLayout(
      doc: loadDoc(slug),
      child: MarkdownContent(slug: slug),
    );
  }
}
```

#### 3. **Code Splitting & Lazy Loading**
```zenith
// Automatic code splitting by page
// Each page is compiled to its own chunk
// Only loaded when user navigates to that route

// Manual lazy loading for heavy components
class HeavyComponent extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return LazyBuilder(
      loader: () async => await import('components/HeavyWidget.zenith'),
      loading: LoadingSpinner(),
      builder: (widget) => widget,
    );
  }
}
```

#### 4. **Global State Management**
```zenith
// src/lib/state.zenith
class AppState extends ChangeNotifier {
  ThemeMode _themeMode = ThemeMode.system;
  String _currentRoute = '/';
  
  ThemeMode get themeMode => _themeMode;
  String get currentRoute => _currentRoute;
  
  void setThemeMode(ThemeMode mode) {
    _themeMode = mode;
    notifyListeners();
  }
  
  void navigate(String route) {
    _currentRoute = route;
    notifyListeners();
  }
}
```

#### 5. **CSS Variables & Theming**
```css
/* styles/variables.css */
:root {
  --primary: #6366F1;
  --primary-dark: #4F46E5;
  --secondary: #8B5CF6;
  --accent: #EC4899;
  --bg-dark: #0f172a;
  --text-primary: #ffffff;
  --text-secondary: #94a3b8;
}

/* styles/themes/dark.css */
[data-theme="dark"] {
  --bg-color: var(--bg-dark);
  --text-color: var(--text-primary);
}
```

#### 6. **Build System Improvements**
```zenith
// config/build.config.zenith
class BuildConfig {
  static const bool minify = true;
  static const bool treeShake = true;
  static const bool codeSplit = true;
  static const String outputDir = 'web';
  static const List<String> targets = ['web', 'wasm'];
  static const int optimizationLevel = 3;
}
```

### Build Process

#### Current (Problematic)
```bash
zenith compile lib/main.zen -target web -o web/main.html
# Creates: Single 44KB HTML file with everything inline
```

#### Proposed (Better)
```bash
zenith build
# Steps:
# 1. Parse all .zenith files
# 2. Build dependency graph
# 3. Code split by routes
# 4. Compile to optimized JS/WASM
# 5. Extract CSS to separate files
# 6. Generate minimal HTML shell
# 7. Copy static assets
# 8. Generate service worker (PWA)
```

Output:
```
web/
├── index.html (2KB - minimal shell)
├── js/
│   ├── runtime.js (5KB - shared runtime)
│   ├── main.chunk.js (15KB - home page)
│   ├── docs.chunk.js (12KB - docs pages)
│   └── vendors.chunk.js (30KB - third-party)
├── css/
│   ├── main.css (20KB - extracted styles)
│   └── critical.css (5KB - above-the-fold)
└── wasm/
    └── app.wasm (50KB - optional WASM module)
```

### Benefits

1. **Maintainability**: Components are reusable and easier to update
2. **Performance**: Code splitting reduces initial load time
3. **Scalability**: Easy to add new pages/features
4. **Developer Experience**: Hot reload, better debugging
5. **SEO**: Server-side rendering support (future)
6. **Accessibility**: Centralized ARIA/compliance management
7. **Theming**: Consistent design system across all pages
8. **Testing**: Components can be tested in isolation

### Migration Path

#### Phase 1: Foundation (Week 1-2)
- [ ] Set up new directory structure
- [ ] Create build configuration
- [ ] Implement component system
- [ ] Migrate existing components

#### Phase 2: Routing & Pages (Week 3-4)
- [ ] Implement page-based routing
- [ ] Create page layouts
- [ ] Migrate existing pages
- [ ] Add navigation system

#### Phase 3: Optimization (Week 5-6)
- [ ] Implement code splitting
- [ ] Add lazy loading
- [ ] Optimize asset loading
- [ ] Set up PWA features

#### Phase 4: Advanced Features (Week 7-8)
- [ ] Add state management
- [ ] Implement theming system
- [ ] Add analytics integration
- [ ] Set up CI/CD pipeline

### Example: Modern Component Usage

```zenith
// src/pages/index.zenith
import 'package:zenith_ui/zenith_ui.dart';
import '../layouts/DefaultLayout.zenith';
import '../components/content/HeroSection.zenith';
import '../components/content/PlatformGrid.zenith';
import '../components/content/FeatureList.zenith';

class HomePage extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return DefaultLayout(
      title: 'Zenith Language',
      description: 'Build Everything with 100% Zenith Code',
      children: [
        HeroSection(
          title: 'Zenith Language',
          subtitle: 'Build Everything with 100% Zenith Code',
          tagline: 'Games • Mobile • Desktop • Web • WebAssembly',
          ctaPrimary: CTAButton(
            label: 'Get Started',
            href: '/docs',
            style: ButtonStyle.primary,
          ),
          ctaSecondary: CTAButton(
            label: 'View Cookbook',
            href: '/cookbook',
            style: ButtonStyle.outline,
          ),
        ),
        PlatformGrid(platforms: [
          PlatformData(icon: '🎮', title: 'Games', desc: '2D/3D game development'),
          PlatformData(icon: '📱', title: 'Mobile', desc: 'iOS & Android'),
          PlatformData(icon: '🖥️', title: 'Desktop', desc: 'Windows, Mac, Linux'),
          PlatformData(icon: '🌐', title: 'Web', desc: 'SPA & PWA'),
          PlatformData(icon: '⚡', title: 'WASM', desc: 'High-performance modules'),
        ]),
        FeatureList(features: FEATURE_DATA),
      ],
    );
  }
}

const FEATURE_DATA = [
  FeatureData(
    icon: '🚀',
    title: 'Cross-Platform',
    description: 'Single codebase for all platforms',
  ),
  FeatureData(
    icon: '🦀',
    title: 'Rust Integration',
    description: 'Access 10,000+ Rust crates',
  ),
  // ... more features
];
```

### Conclusion

This modern architecture brings Zenith documentation site in line with industry best practices, making it:
- **Easier to maintain** with clear separation of concerns
- **Faster to load** through code splitting and optimization
- **Better to develop** with hot reload and component isolation
- **More scalable** for future growth and features
- **Professional quality** matching modern web standards

The investment in restructuring will pay dividends in developer productivity, user experience, and long-term maintainability.
