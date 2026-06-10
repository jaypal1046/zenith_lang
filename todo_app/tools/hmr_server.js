#!/usr/bin/env node
/**
 * Zenith Hot Module Replacement (HMR) Server
 * Provides instant feedback by pushing code updates via WebSocket
 * without losing application state
 */

const WebSocket = require('ws');
const fs = require('fs');
const path = require('path');
const chokidar = require('chokidar');
const http = require('http');

class ZenithHMRSERVER {
    constructor(port = 8080) {
        this.port = port;
        this.clients = new Set();
        this.fileCache = new Map();
        this.watcher = null;
        
        // Create HTTP server for static files
        this.server = http.createServer((req, res) => {
            if (req.url === '/hmr.js') {
                res.writeHead(200, {'Content-Type': 'application/javascript'});
                res.end(this.getClientScript());
            } else {
                res.writeHead(404);
                res.end('Not found');
            }
        });
        
        // Create WebSocket server
        this.wss = new WebSocket.Server({server: this.server});
        
        this.setupWebSocketHandlers();
    }

    setupWebSocketHandlers() {
        this.wss.on('connection', (ws) => {
            console.log('🔌 Client connected');
            this.clients.add(ws);
            
            ws.on('close', () => {
                console.log('🔌 Client disconnected');
                this.clients.delete(ws);
            });
            
            ws.on('error', (err) => {
                console.error('WebSocket error:', err);
                this.clients.delete(ws);
            });
        });
    }

    getClientScript() {
        return `
(function() {
    console.log('🔥 Zenith HMR Client loaded');
    
    const socket = new WebSocket('ws://' + window.location.hostname + ':8080');
    
    socket.onmessage = (event) => {
        const data = JSON.parse(event.data);
        console.log('📩 HMR Update:', data);
        
        if (data.type === 'file_changed') {
            // Hot reload logic
            if (data.fileType === 'zenith') {
                window.ZenithRuntime?.reloadModule(data.fileName, data.content);
                console.log('✅ Hot reloaded:', data.fileName);
            }
        }
    };
    
    socket.onopen = () => console.log('✅ Connected to Zenith HMR Server');
    socket.onerror = (err) => console.error('HMR Connection error:', err);
    socket.onclose = () => console.log('⚠️ HMR Server disconnected');
})();
`;
    }

    watchFiles(patterns) {
        console.log('👀 Watching files:', patterns);
        
        this.watcher = chokidar.watch(patterns, {
            ignored: /node_modules|build|\.git/,
            persistent: true,
            ignoreInitial: true
        });

        this.watcher
            .on('change', (filePath) => this.handleFileChange(filePath))
            .on('add', (filePath) => this.handleFileChange(filePath))
            .on('error', (err) => console.error('Watcher error:', err));
    }

    handleFileChange(filePath) {
        console.log('📝 File changed:', filePath);
        
        fs.readFile(filePath, 'utf8', (err, content) => {
            if (err) {
                console.error('Error reading file:', err);
                return;
            }
            
            this.fileCache.set(filePath, content);
            
            const update = {
                type: 'file_changed',
                fileName: path.basename(filePath),
                filePath: filePath,
                fileType: filePath.endsWith('.zen') ? 'zenith' : 'unknown',
                content: content,
                timestamp: Date.now()
            };
            
            this.broadcast(update);
        });
    }

    broadcast(message) {
        const data = JSON.stringify(message);
        let sentCount = 0;
        
        this.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(data);
                sentCount++;
            }
        });
        
        console.log(`📡 Broadcasted to ${sentCount} clients`);
    }

    start(callback) {
        this.server.listen(this.port, () => {
            console.log(`🚀 Zenith HMR Server running on http://localhost:${this.port}`);
            console.log(`🔌 WebSocket endpoint: ws://localhost:${this.port}`);
            if (callback) callback();
        });
    }

    stop() {
        if (this.watcher) {
            this.watcher.close();
        }
        this.wss.close();
        this.server.close();
        console.log('🛑 HMR Server stopped');
    }
}

// CLI Usage
if (require.main === module) {
    const args = process.argv.slice(2);
    const port = parseInt(args.find(a => a.startsWith('--port=')))?.split('=')[1] || 8080;
    const watchPath = args.find(a => !a.startsWith('--')) || './examples';
    
    const hmrServer = new ZenithHMRSERVER(port);
    hmrServer.start(() => {
        hmrServer.watchFiles([`${watchPath}/**/*.zen`, `${watchPath}/**/*.zenith`]);
    });
    
    process.on('SIGINT', () => {
        hmrServer.stop();
        process.exit(0);
    });
}

module.exports = ZenithHMRSERVER;
