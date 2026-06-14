/**
 * Example API Route
 * Demonstrates API endpoint creation (like Next.js API routes)
 */

// GET handler - Fetch documentation pages
export async function GET(context) {
  const { query, params } = context;
  
  // Mock data - in production, fetch from database or file system
  const docs = [
    {
      id: 'getting-started',
      title: 'Getting Started',
      slug: 'getting-started',
      category: 'Introduction',
      updatedAt: '2024-01-15'
    },
    {
      id: 'installation',
      title: 'Installation Guide',
      slug: 'installation',
      category: 'Introduction',
      updatedAt: '2024-01-14'
    },
    {
      id: 'ssr-csr',
      title: 'SSR and CSR',
      slug: 'ssr-csr',
      category: 'Rendering',
      updatedAt: '2024-01-16'
    },
    {
      id: 'routing',
      title: 'File-Based Routing',
      slug: 'routing',
      category: 'Routing',
      updatedAt: '2024-01-13'
    }
  ];

  // Filter by category if provided
  let filteredDocs = docs;
  if (query.category) {
    filteredDocs = docs.filter(doc => 
      doc.category.toLowerCase() === query.category.toLowerCase()
    );
  }

  return {
    statusCode: 200,
    headers: {
      'Content-Type': 'application/json',
      'Cache-Control': 'public, max-age=60'
    },
    body: {
      success: true,
      data: filteredDocs,
      meta: {
        total: filteredDocs.length,
        timestamp: new Date().toISOString()
      }
    }
  };
}

// POST handler - Create new documentation entry
export async function POST(context) {
  const { body, req } = context;
  
  // Validate request
  if (!body || !body.title) {
    return {
      statusCode: 400,
      headers: { 'Content-Type': 'application/json' },
      body: {
        success: false,
        error: 'Title is required'
      }
    };
  }

  // In production, save to database
  const newDoc = {
    id: `doc-${Date.now()}`,
    title: body.title,
    slug: body.title.toLowerCase().replace(/\s+/g, '-'),
    category: body.category || 'General',
    content: body.content || '',
    createdAt: new Date().toISOString(),
    updatedAt: new Date().toISOString()
  };

  return {
    statusCode: 201,
    headers: { 
      'Content-Type': 'application/json',
      'Location': `/api/docs/${newDoc.id}`
    },
    body: {
      success: true,
      data: newDoc,
      message: 'Documentation created successfully'
    }
  };
}

// Default export - handles all methods
export default async function docsHandler(context) {
  const { method } = context;
  
  switch (method) {
    case 'GET':
      return GET(context);
    case 'POST':
      return POST(context);
    default:
      return {
        statusCode: 405,
        headers: { 'Content-Type': 'application/json' },
        body: {
          success: false,
          error: 'Method not allowed'
        }
      };
  }
}
