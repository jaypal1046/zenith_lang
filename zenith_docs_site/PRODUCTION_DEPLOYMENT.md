# Zenith Framework - Production Deployment Guide

## 🚀 Complete Production Setup

This guide covers deploying your Zenith SSR/CSR application to production environments.

---

## Table of Contents

1. [Production Build](#production-build)
2. [Environment Configuration](#environment-configuration)
3. [Deployment Options](#deployment-options)
4. [Performance Optimization](#performance-optimization)
5. [Monitoring & Logging](#monitoring--logging)
6. [Security Best Practices](#security-best-practices)
7. [Scaling Strategies](#scaling-strategies)

---

## Production Build

### 1. Optimize Assets

```bash
# Minify JavaScript files
npm install -g terser

# Minify CSS files
npm install -g clean-css-cli

# Run optimization script
./optimize-production.sh
```

### 2. Create Production Bundle

```bash
# Create dist directory structure
mkdir -p dist/{server,src/{pages,layouts,lib,components},public/{css,js,images}}

# Copy optimized files
cp server/ssr-server.js dist/server/
cp -r src/pages dist/src/
cp -r src/layouts dist/src/
cp src/lib/runtime.js dist/src/lib/
cp src/styles/globals.css dist/public/css/
cp src/components/*.js dist/src/components/
```

### 3. Environment Variables

Create `.env.production`:

```bash
NODE_ENV=production
PORT=3000
HOST=0.0.0.0
ENABLE_CACHE=true
CACHE_TTL=3600
LOG_LEVEL=warn
ENABLE_COMPRESSION=true
MAX_CONNECTIONS=1000
```

---

## Deployment Options

### Option 1: Docker Deployment

**Dockerfile:**
```dockerfile
FROM node:18-alpine

WORKDIR /app

# Install dependencies
COPY package*.json ./
RUN npm ci --only=production

# Copy application files
COPY server/ ./server/
COPY src/ ./src/
COPY public/ ./public/

# Set environment
ENV NODE_ENV=production
ENV PORT=3000

# Expose port
EXPOSE 3000

# Health check
HEALTHCHECK --interval=30s --timeout=3s \
  CMD wget -qO- http://localhost:3000/api/health || exit 1

# Start application
CMD ["node", "server/ssr-server.js"]
```

**docker-compose.yml:**
```yaml
version: '3.8'

services:
  zenith-app:
    build: .
    ports:
      - "3000:3000"
    environment:
      - NODE_ENV=production
      - PORT=3000
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "wget", "-qO-", "http://localhost:3000/api/health"]
      interval: 30s
      timeout: 3s
      retries: 3
    
  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
      - "443:443"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf
      - ./ssl:/etc/nginx/ssl
    depends_on:
      - zenith-app
    restart: unless-stopped
```

### Option 2: PM2 Process Manager

**ecosystem.config.js:**
```javascript
module.exports = {
  apps: [{
    name: 'zenith-docs',
    script: './server/ssr-server.js',
    instances: 'max',
    exec_mode: 'cluster',
    env: {
      NODE_ENV: 'production',
      PORT: 3000
    },
    env_production: {
      NODE_ENV: 'production',
      PORT: 3000
    },
    error_file: './logs/error.log',
    out_file: './logs/out.log',
    log_file: './logs/combined.log',
    time: true,
    autorestart: true,
    max_memory_restart: '1G',
    watch: false
  }]
};
```

**Deploy with PM2:**
```bash
# Install PM2 globally
npm install -g pm2

# Start application
pm2 start ecosystem.config.js --env production

# Save PM2 configuration
pm2 save

# Setup PM2 to start on boot
pm2 startup
```

### Option 3: Kubernetes Deployment

**k8s/deployment.yaml:**
```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: zenith-docs
spec:
  replicas: 3
  selector:
    matchLabels:
      app: zenith-docs
  template:
    metadata:
      labels:
        app: zenith-docs
    spec:
      containers:
      - name: zenith-app
        image: your-registry/zenith-docs:latest
        ports:
        - containerPort: 3000
        env:
        - name: NODE_ENV
          value: "production"
        - name: PORT
          value: "3000"
        resources:
          requests:
            memory: "256Mi"
            cpu: "250m"
          limits:
            memory: "512Mi"
            cpu: "500m"
        livenessProbe:
          httpGet:
            path: /api/health
            port: 3000
          initialDelaySeconds: 30
          periodSeconds: 10
        readinessProbe:
          httpGet:
            path: /api/health
            port: 3000
          initialDelaySeconds: 5
          periodSeconds: 5
```

**k8s/service.yaml:**
```yaml
apiVersion: v1
kind: Service
metadata:
  name: zenith-docs-service
spec:
  selector:
    app: zenith-docs
  ports:
    - protocol: TCP
      port: 80
      targetPort: 3000
  type: LoadBalancer
```

---

## Performance Optimization

### 1. Enable Compression

Add to `ssr-server.js`:
```javascript
const compression = require('compression');
app.use(compression());
```

### 2. Implement Caching Strategies

**In-Memory Cache:**
```javascript
const NodeCache = require('node-cache');
const cache = new NodeCache({ stdTTL: 300, checkperiod: 60 });

// Cache SSR responses
app.get('*', (req, res) => {
  const cached = cache.get(req.url);
  if (cached) {
    return res.send(cached);
  }
  
  // Render and cache
  const html = renderPage(req.url);
  cache.set(req.url, html);
  res.send(html);
});
```

**Redis Cache (for distributed systems):**
```javascript
const redis = require('redis');
const client = redis.createClient();

async function getCachedPage(url) {
  return new Promise((resolve, reject) => {
    client.get(`page:${url}`, (err, data) => {
      if (err) reject(err);
      resolve(data);
    });
  });
}
```

### 3. Static Asset Optimization

- **Minify CSS/JS**: Use Terser and CleanCSS
- **Enable Gzip/Brotli**: Compress text assets
- **CDN Integration**: Serve static assets from CDN
- **Lazy Loading**: Defer non-critical resources

### 4. Database Query Optimization

If using database for docs:
```javascript
// Add indexes
db.docs.createIndex({ slug: 1 });
db.docs.createIndex({ category: 1, createdAt: -1 });

// Use connection pooling
const pool = new Pool({
  max: 20,
  idleTimeoutMillis: 30000,
  connectionTimeoutMillis: 2000
});
```

---

## Monitoring & Logging

### 1. Application Logging

**Winston Logger Setup:**
```javascript
const winston = require('winston');

const logger = winston.createLogger({
  level: process.env.LOG_LEVEL || 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.json()
  ),
  transports: [
    new winston.transports.File({ filename: 'logs/error.log', level: 'error' }),
    new winston.transports.File({ filename: 'logs/combined.log' })
  ]
});

if (process.env.NODE_ENV !== 'production') {
  logger.add(new winston.transports.Console({
    format: winston.format.simple()
  }));
}
```

### 2. Health Check Endpoint

Add to `ssr-server.js`:
```javascript
app.get('/api/health', (req, res) => {
  res.json({
    status: 'healthy',
    timestamp: new Date().toISOString(),
    uptime: process.uptime(),
    memory: process.memoryUsage(),
    version: '1.0.0'
  });
});
```

### 3. Metrics Collection

**Prometheus Metrics:**
```javascript
const promClient = require('prom-client');

const register = new promClient.Registry();
const httpRequestDuration = new promClient.Histogram({
  name: 'http_request_duration_seconds',
  help: 'Duration of HTTP requests in seconds',
  labelNames: ['method', 'route', 'status_code']
});

register.registerMetric(httpRequestDuration);

app.get('/metrics', async (req, res) => {
  res.set('Content-Type', register.contentType);
  res.end(await register.metrics());
});
```

### 4. Error Tracking

**Sentry Integration:**
```javascript
const Sentry = require('@sentry/node');

Sentry.init({
  dsn: process.env.SENTRY_DSN,
  environment: process.env.NODE_ENV,
  tracesSampleRate: 0.1
});

app.use(Sentry.Handlers.requestHandler());
app.use(Sentry.Handlers.errorHandler());
```

---

## Security Best Practices

### 1. HTTPS Configuration

**Nginx SSL Config:**
```nginx
server {
    listen 443 ssl http2;
    server_name your-domain.com;

    ssl_certificate /etc/nginx/ssl/fullchain.pem;
    ssl_certificate_key /etc/nginx/ssl/privkey.pem;
    
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers HIGH:!aNULL:!MD5;
    ssl_prefer_server_ciphers on;
    
    # HSTS
    add_header Strict-Transport-Security "max-age=31536000" always;
    
    location / {
        proxy_pass http://localhost:3000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}
```

### 2. Security Headers

Add middleware to `ssr-server.js`:
```javascript
const helmet = require('helmet');

app.use(helmet({
  contentSecurityPolicy: {
    directives: {
      defaultSrc: ["'self'"],
      styleSrc: ["'self'", "'unsafe-inline'"],
      scriptSrc: ["'self'", "'unsafe-inline'"],
      imgSrc: ["'self'", 'data:', 'https:']
    }
  },
  hsts: {
    maxAge: 31536000,
    includeSubDomains: true,
    preload: true
  }
}));
```

### 3. Rate Limiting

```javascript
const rateLimit = require('express-rate-limit');

const limiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 100, // limit each IP to 100 requests per windowMs
  message: 'Too many requests from this IP'
});

app.use('/api/', limiter);
```

### 4. Input Validation

```javascript
const { body, param, query, validationResult } = require('express-validator');

app.post('/api/docs', [
  body('title').isString().isLength({ min: 1, max: 200 }),
  body('content').isString(),
  body('slug').isAlphanumeric().isLength({ min: 1, max: 100 })
], (req, res) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    return res.status(400).json({ errors: errors.array() });
  }
  // Process valid input
});
```

---

## Scaling Strategies

### 1. Horizontal Scaling

- Deploy multiple instances behind load balancer
- Use sticky sessions for stateful connections
- Implement distributed caching (Redis)
- Database read replicas

### 2. Vertical Scaling

- Increase server resources (CPU, RAM)
- Optimize Node.js memory limits: `NODE_OPTIONS="--max-old-space-size=4096"`
- Use clustering mode in PM2

### 3. CDN Integration

**Cloudflare Setup:**
- Point DNS to Cloudflare
- Enable caching for static assets
- Configure page rules for SSR paths
- Enable Brotli compression

### 4. Auto-Scaling (Kubernetes)

```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: zenith-docs-hpa
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: zenith-docs
  minReplicas: 3
  maxReplicas: 10
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
```

---

## Continuous Deployment

### GitHub Actions Workflow

**.github/workflows/deploy.yml:**
```yaml
name: Deploy to Production

on:
  push:
    branches: [main]

jobs:
  deploy:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Node.js
      uses: actions/setup-node@v3
      with:
        node-version: '18'
    
    - name: Install dependencies
      run: npm ci
    
    - name: Run tests
      run: npm test
    
    - name: Build
      run: npm run build
    
    - name: Deploy to server
      uses: appleboy/ssh-action@master
      with:
        host: ${{ secrets.SERVER_HOST }}
        username: ${{ secrets.SERVER_USER }}
        key: ${{ secrets.SSH_PRIVATE_KEY }}
        script: |
          cd /var/www/zenith-docs
          git pull
          npm ci --production
          pm2 restart zenith-docs
```

---

## Troubleshooting

### Common Issues

**High Memory Usage:**
```bash
# Check memory
pm2 monit

# Restart with memory limit
pm2 restart zenith-docs --max-memory-restart 1G
```

**Slow Response Times:**
- Check database query performance
- Enable query logging
- Review cache hit rates
- Profile with Node.js inspector

**Connection Errors:**
- Verify firewall rules
- Check load balancer configuration
- Review connection pool settings

---

## Maintenance

### Regular Tasks

1. **Weekly:**
   - Review error logs
   - Check disk space
   - Update dependencies

2. **Monthly:**
   - Security audit
   - Performance review
   - Backup verification

3. **Quarterly:**
   - Dependency updates
   - SSL certificate renewal
   - Disaster recovery test

---

## Support & Resources

- Documentation: `/docs`
- API Reference: `/api/docs`
- Status Page: `/api/health`
- Metrics: `/metrics`

---

**🎉 Your Zenith application is now production-ready!**
