const cp = require('child_process');
const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

let serverProcess = null;
let buffer = '';
const pendingRequests = new Map();
let requestId = 0;
let diagnosticCollection = null;
let outputChannel = null;

function activate(context) {
    outputChannel = vscode.window.createOutputChannel("Zenith LSP Client");
    outputChannel.appendLine("Zenith extension activating...");

    diagnosticCollection = vscode.languages.createDiagnosticCollection('zenith');
    context.subscriptions.push(diagnosticCollection);

    startServer(context);

    // Register Hover Provider
    const hoverProvider = vscode.languages.registerHoverProvider('zenith', {
        async provideHover(document, position, token) {
            if (!serverProcess) return null;
            try {
                const res = await sendRequest('textDocument/hover', {
                    textDocument: { uri: document.uri.toString() },
                    position: { line: position.line, character: position.character }
                });
                if (res && res.contents) {
                    return new vscode.Hover(new vscode.MarkdownString(res.contents.value));
                }
            } catch (e) {
                outputChannel.appendLine(`Hover error: ${e.message || e}`);
            }
            return null;
        }
    });
    context.subscriptions.push(hoverProvider);

    // Register Completion Provider
    const completionProvider = vscode.languages.registerCompletionItemProvider('zenith', {
        async provideCompletionItems(document, position, token, context) {
            if (!serverProcess) return [];
            try {
                const res = await sendRequest('textDocument/completion', {
                    textDocument: { uri: document.uri.toString() },
                    position: { line: position.line, character: position.character }
                });
                if (res && res.items) {
                    return res.items.map(item => {
                        const mapKind = (lspKind) => {
                            if (typeof lspKind === 'number') {
                                return lspKind - 1; // Map 1-based LSP to 0-based VS Code kinds
                            }
                            return vscode.CompletionItemKind.Text;
                        };
                        const vscodeItem = new vscode.CompletionItem(item.label, mapKind(item.kind));
                        vscodeItem.detail = item.detail;
                        vscodeItem.insertText = item.insertText;
                        return vscodeItem;
                    });
                }
            } catch (e) {
                outputChannel.appendLine(`Completion error: ${e.message || e}`);
            }
            return [];
        }
    }, '.', ' ', '(', ':');
    context.subscriptions.push(completionProvider);

    // Monitor Document changes to trigger Diagnostics
    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(doc => {
            if (doc.languageId === 'zenith') {
                sendNotification('textDocument/didOpen', {
                    textDocument: {
                        uri: doc.uri.toString(),
                        languageId: 'zenith',
                        version: doc.version,
                        text: doc.getText()
                    }
                });
            }
        })
    );

    context.subscriptions.push(
        vscode.workspace.onDidChangeTextDocument(event => {
            const doc = event.document;
            if (doc.languageId === 'zenith') {
                sendNotification('textDocument/didChange', {
                    textDocument: {
                        uri: doc.uri.toString(),
                        version: doc.version
                    },
                    contentChanges: [{
                        text: doc.getText()
                    }]
                });
            }
        })
    );

    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument(doc => {
            if (doc.languageId === 'zenith') {
                sendNotification('textDocument/didSave', {
                    textDocument: {
                        uri: doc.uri.toString()
                    },
                    text: doc.getText()
                });
            }
        })
    );

    context.subscriptions.push(
        vscode.workspace.onDidCloseTextDocument(doc => {
            if (doc.languageId === 'zenith') {
                diagnosticCollection.delete(doc.uri);
            }
        })
    );

    // Initialize already open files
    vscode.workspace.textDocuments.forEach(doc => {
        if (doc.languageId === 'zenith') {
            sendNotification('textDocument/didOpen', {
                textDocument: {
                    uri: doc.uri.toString(),
                    languageId: 'zenith',
                    version: doc.version,
                    text: doc.getText()
                }
            });
        }
    });

    outputChannel.appendLine("Zenith extension activated.");
}

function startServer(context) {
    const config = vscode.workspace.getConfiguration('zenith');
    let compilerPath = config.get('compilerPath') || 'zenith';

    if (compilerPath === 'zenith' && vscode.workspace.workspaceFolders) {
        const rootPath = vscode.workspace.workspaceFolders[0].uri.fsPath;
        const localExe = path.join(rootPath, 'zenith.exe');
        const localBin = path.join(rootPath, 'zenith');
        if (fs.existsSync(localExe)) {
            compilerPath = localExe;
        } else if (fs.existsSync(localBin)) {
            compilerPath = localBin;
        }
    }

    outputChannel.appendLine(`Starting Zenith LSP server using executable: ${compilerPath}`);

    try {
        serverProcess = cp.spawn(compilerPath, ['lsp'], {
            cwd: vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.fsPath : undefined
        });

        serverProcess.stdout.on('data', (data) => {
            buffer += data.toString();
            processBuffer();
        });

        serverProcess.stderr.on('data', (data) => {
            outputChannel.appendLine(`Server error log: ${data.toString()}`);
        });

        serverProcess.on('close', (code) => {
            outputChannel.appendLine(`Zenith LSP server process exited with code ${code}`);
            serverProcess = null;
            // Clear pending requests
            for (const [id, req] of pendingRequests.entries()) {
                req.reject(new Error("LSP server stopped."));
            }
            pendingRequests.clear();
        });

        serverProcess.on('error', (err) => {
            outputChannel.appendLine(`Failed to start Zenith LSP server: ${err.message}`);
            vscode.window.showErrorMessage(`Failed to start Zenith LSP server: ${err.message}. Please configure the correct 'zenith.compilerPath' setting.`);
            serverProcess = null;
        });

        // Send initialize request
        sendRequest('initialize', {
            processId: process.pid,
            rootUri: vscode.workspace.workspaceFolders ? vscode.workspace.workspaceFolders[0].uri.toString() : null,
            capabilities: {}
        }).then((res) => {
            outputChannel.appendLine("LSP initialized successfully.");
            sendNotification('initialized', {});
        }).catch((err) => {
            outputChannel.appendLine(`LSP initialization failed: ${err.message}`);
        });

    } catch (e) {
        outputChannel.appendLine(`Error spawning server process: ${e.message}`);
        serverProcess = null;
    }
}

function processBuffer() {
    while (true) {
        const headerEnd = buffer.indexOf('\r\n\r\n');
        if (headerEnd === -1) break;

        const headers = buffer.substring(0, headerEnd);
        const match = headers.match(/Content-Length:\s*(\d+)/i);
        if (!match) {
            // Invalid header block, skip it
            buffer = buffer.substring(headerEnd + 4);
            continue;
        }

        const contentLength = parseInt(match[1], 10);
        if (buffer.length < headerEnd + 4 + contentLength) {
            // Need more data
            break;
        }

        const bodyStr = buffer.substring(headerEnd + 4, headerEnd + 4 + contentLength);
        buffer = buffer.substring(headerEnd + 4 + contentLength);

        try {
            const message = JSON.parse(bodyStr);
            handleMessage(message);
        } catch (e) {
            outputChannel.appendLine(`JSON parse error: ${e.message}`);
        }
    }
}

function handleMessage(message) {
    if (message.id !== undefined && message.id !== null) {
        const id = message.id;
        const handler = pendingRequests.get(id);
        if (handler) {
            pendingRequests.delete(id);
            if (message.error) {
                handler.reject(message.error);
            } else {
                handler.resolve(message.result);
            }
        }
    } else if (message.method === 'textDocument/publishDiagnostics') {
        const params = message.params;
        const uri = vscode.Uri.parse(params.uri);
        const diagnostics = params.diagnostics.map(d => {
            const range = new vscode.Range(
                d.range.start.line,
                d.range.start.character,
                d.range.end.line,
                d.range.end.character
            );
            return new vscode.Diagnostic(range, d.message, d.severity);
        });
        diagnosticCollection.set(uri, diagnostics);
    }
}

function sendRequest(method, params) {
    return new Promise((resolve, reject) => {
        if (!serverProcess) {
            reject(new Error("LSP server not running."));
            return;
        }
        const id = requestId++;
        pendingRequests.set(id, { resolve, reject });

        const msg = JSON.stringify({
            jsonrpc: '2.0',
            id,
            method,
            params
        });

        const header = `Content-Length: ${Buffer.byteLength(msg, 'utf8')}\r\n\r\n`;
        serverProcess.stdin.write(header + msg);
    });
}

function sendNotification(method, params) {
    if (!serverProcess) return;

    const msg = JSON.stringify({
        jsonrpc: '2.0',
        method,
        params
    });

    const header = `Content-Length: ${Buffer.byteLength(msg, 'utf8')}\r\n\r\n`;
    serverProcess.stdin.write(header + msg);
}

function deactivate() {
    if (serverProcess) {
        serverProcess.kill();
        serverProcess = null;
    }
}

module.exports = {
    activate,
    deactivate
};
