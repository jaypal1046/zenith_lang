#!/usr/bin/env node

/**
 * Zenith Framework Test Suite
 * Tests SSR, CSR, API routes, and dynamic routing
 */

const http = require('http');
const assert = require('assert');

const BASE_URL = 'http://localhost:3000';

// Color codes for output
const colors = {
  reset: '\x1b[0m',
  green: '\x1b[32m',
  red: '\x1b[31m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m'
};

function log(message, color = 'reset') {
  console.log(`${colors[color]}${message}${colors.reset}`);
}

function pass(test) {
  log(`✓ ${test}`, 'green');
}

function fail(test, error) {
  log(`✗ ${test}`, 'red');
  if (error) log(`  Error: ${error.message}`, 'red');
}

async function makeRequest(path, options = {}) {
  return new Promise((resolve, reject) => {
    const url = new URL(path, BASE_URL);
    const reqOptions = {
      hostname: url.hostname,
      port: url.port,
      path: url.pathname + url.search,
      method: options.method || 'GET',
      headers: options.headers || {}
    };

    const req = http.request(reqOptions, (res) => {
      let data = '';
      res.on('data', chunk => data += chunk);
      res.on('end', () => {
        resolve({
          status: res.statusCode,
          headers: res.headers,
          body: data
        });
      });
    });

    req.on('error', reject);

    if (options.body) {
      req.write(JSON.stringify(options.body));
    }

    req.end();
  });
}

async function runTests() {
  log('\n🧪 Zenith Framework Test Suite\n', 'blue');
  log('=' .repeat(50), 'blue');

  let passed = 0;
  let failed = 0;

  // Test 1: Home Page SSR
  try {
    log('\n📄 Testing Home Page (SSR)...', 'yellow');
    const homeRes = await makeRequest('/');
    
    assert.strictEqual(homeRes.status, 200, 'Home page should return 200');
    assert.ok(homeRes.body.includes('<!DOCTYPE html>'), 'Should return HTML');
    assert.ok(homeRes.body.includes('Zenith'), 'Should contain Zenith branding');
    assert.ok(homeRes.body.includes('id="app"'), 'Should have app container');
    assert.ok(homeRes.body.includes('__INITIAL_STATE__'), 'Should have initial state for hydration');
    
    pass('Home page renders with SSR');
    passed++;
  } catch (error) {
    fail('Home page SSR', error);
    failed++;
  }

  // Test 2: Docs Index Page
  try {
    log('\n📚 Testing Docs Index Page...', 'yellow');
    const docsRes = await makeRequest('/docs');
    
    assert.strictEqual(docsRes.status, 200, 'Docs page should return 200');
    assert.ok(docsRes.body.includes('Documentation'), 'Should contain Documentation title');
    assert.ok(docsRes.body.includes('docs-index-page'), 'Should have docs page class');
    
    pass('Docs index page renders');
    passed++;
  } catch (error) {
    fail('Docs index page', error);
    failed++;
  }

  // Test 3: Dynamic Route - Getting Started
  try {
    log('\n📖 Testing Dynamic Route (/docs/getting-started)...', 'yellow');
    const docRes = await makeRequest('/docs/getting-started');
    
    assert.strictEqual(docRes.status, 200, 'Dynamic doc page should return 200');
    assert.ok(docRes.body.includes('Getting Started'), 'Should contain Getting Started title');
    assert.ok(docRes.body.includes('doc-page'), 'Should have doc page class');
    
    pass('Dynamic route works correctly');
    passed++;
  } catch (error) {
    fail('Dynamic route', error);
    failed++;
  }

  // Test 4: Another Dynamic Route
  try {
    log('\n🔧 Testing Another Dynamic Route (/docs/installation)...', 'yellow');
    const installRes = await makeRequest('/docs/installation');
    
    assert.strictEqual(installRes.status, 200, 'Installation page should return 200');
    assert.ok(installRes.body.includes('Installation'), 'Should contain Installation title');
    
    pass('Another dynamic route works');
    passed++;
  } catch (error) {
    fail('Another dynamic route', error);
    failed++;
  }

  // Test 5: API Route - GET
  try {
    log('\n🔌 Testing API Route (GET /api/docs)...', 'yellow');
    const apiRes = await makeRequest('/api/docs');
    
    assert.strictEqual(apiRes.status, 200, 'API should return 200');
    assert.ok(apiRes.headers['content-type'].includes('application/json'), 'Should return JSON');
    
    const data = JSON.parse(apiRes.body);
    assert.ok(data.success, 'Should have success flag');
    assert.ok(Array.isArray(data.data), 'Should return array of docs');
    assert.ok(data.meta, 'Should have metadata');
    
    pass('API GET endpoint works');
    passed++;
  } catch (error) {
    fail('API GET endpoint', error);
    failed++;
  }

  // Test 6: API Route - GET with Query Params
  try {
    log('\n🔍 Testing API Route with Query Params...', 'yellow');
    const apiRes = await makeRequest('/api/docs?category=Introduction');
    
    assert.strictEqual(apiRes.status, 200, 'API with query should return 200');
    
    const data = JSON.parse(apiRes.body);
    assert.ok(data.success, 'Should have success flag');
    data.data.forEach(doc => {
      assert.strictEqual(doc.category, 'Introduction', 'All docs should be from Introduction category');
    });
    
    pass('API query params work');
    passed++;
  } catch (error) {
    fail('API query params', error);
    failed++;
  }

  // Test 7: API Route - POST
  try {
    log('\n✏️ Testing API Route (POST /api/docs)...', 'yellow');
    const apiRes = await makeRequest('/api/docs', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: {
        title: 'Test Documentation',
        category: 'Testing',
        content: 'This is test content'
      }
    });
    
    assert.strictEqual(apiRes.status, 201, 'POST should return 201');
    
    const data = JSON.parse(apiRes.body);
    assert.ok(data.success, 'Should have success flag');
    assert.ok(data.data.id, 'Should return created doc with ID');
    assert.strictEqual(data.data.title, 'Test Documentation', 'Title should match');
    
    pass('API POST endpoint works');
    passed++;
  } catch (error) {
    fail('API POST endpoint', error);
    failed++;
  }

  // Test 8: 404 Page
  try {
    log('\n❌ Testing 404 Page...', 'yellow');
    const notFoundRes = await makeRequest('/nonexistent-page');
    
    assert.strictEqual(notFoundRes.status, 404, 'Should return 404');
    assert.ok(notFoundRes.body.includes('404'), 'Should contain 404 message');
    
    pass('404 page works correctly');
    passed++;
  } catch (error) {
    fail('404 page', error);
    failed++;
  }

  // Test 9: Static Files
  try {
    log('\n🎨 Testing Static File Serving...', 'yellow');
    const cssRes = await makeRequest('/css/globals.css');
    
    assert.strictEqual(cssRes.status, 200, 'CSS file should return 200');
    assert.ok(cssRes.headers['content-type'].includes('text/css'), 'Should return CSS content type');
    assert.ok(cssRes.body.includes('--primary') || cssRes.body.includes(':root'), 'Should contain CSS variables');
    
    pass('Static CSS files served correctly');
    passed++;
  } catch (error) {
    fail('Static file serving', error);
    failed++;
  }

  // Test 10: SSR Data Fetching
  try {
    log('\n💾 Testing Server-Side Data Fetching...', 'yellow');
    const homeRes = await makeRequest('/');
    
    assert.ok(homeRes.body.includes('features'), 'Should include features data');
    assert.ok(homeRes.body.includes('Server-Side Rendering'), 'Should render fetched data');
    assert.ok(homeRes.body.includes('Rendered on server'), 'Should show server render time');
    
    pass('Server-side data fetching works');
    passed++;
  } catch (error) {
    fail('Server-side data fetching', error);
    failed++;
  }

  // Summary
  log('\n' + '='.repeat(50), 'blue');
  log(`\n📊 Test Results:`, 'blue');
  log(`   Passed: ${passed}`, 'green');
  log(`   Failed: ${failed}`, failed > 0 ? 'red' : 'green');
  log(`   Total:  ${passed + failed}`, 'blue');
  log('', 'reset');

  if (failed === 0) {
    log('🎉 All tests passed!', 'green');
    process.exit(0);
  } else {
    log('⚠️  Some tests failed', 'yellow');
    process.exit(1);
  }
}

// Check if server is running
async function checkServer() {
  try {
    await makeRequest('/');
    return true;
  } catch {
    return false;
  }
}

// Main execution
(async () => {
  const serverRunning = await checkServer();
  
  if (!serverRunning) {
    log('\n❌ Zenith server is not running!', 'red');
    log('\nStart the server first:', 'yellow');
    log('  node server/ssr-server.js', 'blue');
    log('\nOr use the dev script:', 'yellow');
    log('  ./dev-hot-reload.sh', 'blue');
    log('', 'reset');
    process.exit(1);
  }

  await runTests();
})();
