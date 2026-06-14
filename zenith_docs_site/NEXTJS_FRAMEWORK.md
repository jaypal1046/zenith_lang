# Zenith Docs Site - Next.js-Style Framework

A modern server-side rendering (SSR) and client-side rendering (CSR) framework for Zenith Language documentation, inspired by Next.js and React.

## Architecture Overview

```
zenith_docs_site/
├── src/
│   ├── components/          # Reusable UI components
│   │   ├── Navigation.zenith
│   │   ├── Footer.zenith
│   │   ├── CodeDemo.zenith
│   │   ├── FeatureCard.zenith
│   │   └── PlatformCard.zenith
│   ├── pages/               # Page components (file-based routing)
│   │   ├── index.zenith     # Home page (/)
│   │   ├── cookbook.zenith  # Cookbook page (/cookbook)
│   │   ├── docs.zenith      # Docs page (/docs)
│   │   └── _document.zenith # Custom document template
│   ├── layouts/             # Layout components
│   │   └── default.zenith   # Default layout wrapper
│   ├── lib/                 # Utility functions
│   │   ├── router.zenith    # Client-side routing
│   │   ├── ssr.zenith       # Server-side rendering engine
│   │   └── markdown.zenith  # Markdown parser
│   └── styles/              # Global styles
│       └── globals.css
├── public/                  # Static assets
│   ├── images/
│   └── favicon.ico
├── content/                 # Markdown content
│   ├── cookbook/
│   └── docs/
├── server/                  # SSR server
│   └── main.zenith
├── build.zenith             # Build script
├── config.zenith            # Configuration
└── pubspec.yaml
```

## Key Features

### 1. File-Based Routing (Like Next.js)
- Every file in `src/pages/` becomes a route
- `index.zenith` → `/`
- `cookbook.zenith` → `/cookbook`
- `docs/[slug].zenith` → `/docs/:slug` (dynamic routes)

### 2. Server-Side Rendering (SSR)
- Pages are rendered on the server at request time
- Full SEO support with pre-rendered HTML
- Fast initial page load

### 3. Client-Side Rendering (CSR)
- After initial load, navigation happens client-side
- Smooth transitions between pages
- State preservation across navigations

### 4. Hybrid Rendering
- Choose per-page: SSR, CSR, or Static Generation
- `export const renderMode = 'ssr' | 'csr' | 'static'`

### 5. Component-Based Architecture
- Reusable components with props
- Composition over inheritance
- Hot reload during development

### 6. Built-in Optimizations
- Automatic code splitting
- Image optimization
- CSS minification
- JavaScript bundling

## Quick Start

### Development Mode
```bash
# Start development server with hot reload
zenith run server/main.zenith --dev

# Or use the build script
./build.zenith dev
```

### Production Build
```bash
# Build for production
./build.zenith build

# Serve production build
./build.zenith start
```

## Usage Examples

### Creating a Page (SSR)

```zenith
// src/pages/index.zenith
import '../layouts/default.zenith';
import '../components/Hero.zenith';
import '../components/FeatureGrid.zenith';

export const renderMode = 'ssr';
export const metadata = {
  title: 'Zenith Language - Official Documentation',
  description: 'Build Everything with 100% Zenith Code',
};

export async function getServerSideProps(context) {
  // Fetch data at request time
  final features = await fetchFeatures();
  return { props: { features } };
}

class HomePage extends Component {
  final List<Feature> features;
  
  HomePage({required this.features});
  
  @override
  Widget build() {
    return DefaultLayout(
      child: Column([
        Hero(
          title: 'Zenith Language',
          subtitle: 'Build Everything with 100% Zenith Code',
        ),
        FeatureGrid(features: features),
      ]),
    );
  }
}
```

### Creating a Page (CSR)

```zenith
// src/pages/cookbook.zenith
import '../layouts/default.zenith';

export const renderMode = 'csr';

class CookbookPage extends Component {
  @override
  Widget build() {
    return DefaultLayout(
      child: Column([
        Text('Cookbook', style: TextStyle(fontSize: 32)),
        // Content loaded client-side
      ]),
    );
  }
}
```

### Creating a Component

```zenith
// src/components/Hero.zenith
import 'package:zenith_ui/zenith_ui.dart';

class HeroProps {
  final String title;
  final String subtitle;
  final List<String> ctas;
  
  HeroProps({
    required this.title,
    required this.subtitle,
    this.ctas = const [],
  });
}

class Hero extends Component {
  final HeroProps props;
  
  Hero(this.props);
  
  @override
  Widget build() {
    return Container(
      padding: EdgeInsets.all(80),
      decoration: BoxDecoration(
        gradient: LinearGradient(colors: [
          Color(0xFF6366F1),
          Color(0xFF8B5CF6),
        ]),
      ),
      child: Column([
        Text(props.title, style: TextStyle(fontSize: 64, color: Colors.white)),
        SizedBox(height: 20),
        Text(props.subtitle, style: TextStyle(fontSize: 28, color: Colors.white70)),
        SizedBox(height: 40),
        Row(props.ctas.map((cta) => Button(cta))),
      ]),
    );
  }
}
```

### Custom Document Template

```zenith
// src/pages/_document.zenith
import 'package:zenith_ui/zenith_ui.dart';

class DocumentTemplate extends Component {
  final String htmlLang;
  final Widget head;
  final Widget body;
  
  DocumentTemplate({
    this.htmlLang = 'en',
    required this.head,
    required this.body,
  });
  
  @override
  Widget build() {
    return HtmlDocument(
      lang: htmlLang,
      head: Head([
        Meta(charset: 'UTF-8'),
        Meta(name: 'viewport', content: 'width=device-width, initial-scale=1.0'),
        Link(rel: 'stylesheet', href: '/styles/globals.css'),
        head,
      ]),
      body: Body([
        Script(src: '/runtime/client.js', defer: true),
        body,
      ]),
    );
  }
}
```

### Dynamic Routes

```zenith
// src/pages/docs/[slug].zenith
import '../layouts/default.zenith';

export const renderMode = 'ssr';

export async function getStaticPaths() {
  // Generate paths at build time
  final slugs = ['getting-started', 'tutorial', 'api-reference'];
  return slugs.map((slug) => {'params': {'slug': slug}});
}

export async function getServerSideProps(context) {
  final slug = context.params['slug'];
  final content = await loadDocContent(slug);
  return { props: { slug, content } };
}

class DocPage extends Component {
  final String slug;
  final String content;
  
  DocPage({required this.slug, required this.content});
  
  @override
  Widget build() {
    return DefaultLayout(
      child: Markdown(content: content),
    );
  }
}
```

## Configuration

```zenith
// config.zenith
class SiteConfig {
  static const String title = 'Zenith Language';
  static const String baseUrl = 'https://docs.zenith.dev';
  static const String outputDir = 'dist';
  
  static const BuildConfig build = BuildConfig(
    minify: true,
    sourceMaps: false,
    splitChunks: true,
  );
  
  static const ServerConfig server = ServerConfig(
    port: 3000,
    host: 'localhost',
    ssr: true,
  );
}
```

## Rendering Modes Explained

### SSR (Server-Side Rendering)
- ✅ Best for SEO
- ✅ Fast initial load
- ✅ Dynamic content per request
- ❌ Higher server load

### CSR (Client-Side Rendering)
- ✅ Lower server load
- ✅ Rich interactivity
- ✅ Offline capable
- ❌ Slower initial load
- ❌ Poor SEO

### Static Generation
- ✅ Fastest performance
- ✅ CDN deployable
- ✅ Zero server load
- ❌ Requires rebuild for updates

## Build Process

1. **Parse**: Parse all `.zenith` files in `src/pages/`
2. **Analyze**: Detect dependencies and create dependency graph
3. **Transform**: Convert Zenith components to optimized JavaScript
4. **Bundle**: Split code by routes for optimal loading
5. **Render**: 
   - SSR: Render to HTML at request time
   - Static: Pre-render to HTML at build time
6. **Optimize**: Minify CSS, JS, and HTML
7. **Output**: Write to `dist/` directory

## Development vs Production

### Development
- Hot module replacement (HMR)
- Source maps enabled
- Unminified code
- Detailed error messages
- Fast rebuilds

### Production
- Code splitting
- Tree shaking
- Minification
- Compression (gzip/brotli)
- Cache headers
- CDN-ready

## Migration from Static HTML

To migrate from your current `main.html`:

1. **Extract Components**: Break down monolithic HTML into components
2. **Create Pages**: Move page-specific content to `src/pages/`
3. **Add Layout**: Create shared layout in `src/layouts/`
4. **Configure Routing**: Set up routes in config
5. **Test**: Run dev server and verify all pages
6. **Deploy**: Build and deploy production version

## API Reference

### Component Base Class
```zenith
class Component {
  Widget build();
  void componentDidMount();
  void componentDidUpdate(prevProps);
  void componentWillUnmount();
}
```

### Router API
```zenith
// Navigate programmatically
Router.push('/cookbook');
Router.replace('/docs');
Router.back();

// Access route params
final slug = Router.params['slug'];
final query = Router.query['search'];

// Listen to route changes
Router.onChange((route) {
  print('Navigated to: ${route.path}');
});
```

### Data Fetching
```zenith
// SSR: getServerSideProps
export async function getServerSideProps(context) {
  final data = await fetchData();
  return { props: { data } };
}

// Static: getStaticProps
export async function getStaticProps(context) {
  final data = await fetchData();
  return { props: { data }, revalidate: 60 }; // ISR
}

// Client: useEffect equivalent
class MyComponent extends Component {
  var data;
  
  @override
  void componentDidMount() {
    fetchData().then((result) {
      setState(() => data = result);
    });
  }
}
```

## Performance Best Practices

1. **Lazy Loading**: Load components only when needed
2. **Image Optimization**: Use next-gen formats (WebP, AVIF)
3. **Code Splitting**: Split by routes and components
4. **Caching**: Implement proper cache strategies
5. **CDN**: Deploy static assets to CDN
6. **Compression**: Enable gzip/brotli compression
7. **Prefetching**: Prefetch likely next pages

## Troubleshooting

### Common Issues

**Issue**: Components not rendering
- Check import paths are correct
- Ensure component extends `Component` class
- Verify `build()` method returns a `Widget`

**Issue**: SSR hydration mismatch
- Ensure server and client render same content
- Avoid using `DateTime.now()` in render
- Use `useEffect` for client-only logic

**Issue**: Slow builds
- Enable incremental builds
- Use persistent caching
- Optimize dependency graph

## License

MIT License - See LICENSE file for details
