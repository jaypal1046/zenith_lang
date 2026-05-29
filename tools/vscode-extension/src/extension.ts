import * as vscode from 'vscode';
import { LanguageClient, LanguageClientOptions, ServerOptions, TransportKind } from 'vscode-languageclient/node';

let client: LanguageClient;

export function activate(context: vscode.ExtensionContext) {
    // Register language server start command
    context.subscriptions.push(
        vscode.commands.registerCommand('zenith.startLanguageServer', () => {
            startLanguageServer(context);
        })
    );

    // Auto-start language server when a Zenith file is opened
    if (vscode.window.activeTextEditor?.document.languageId === 'zenith') {
        startLanguageServer(context);
    }
}

function startLanguageServer(context: vscode.ExtensionContext) {
    // The server is implemented in the Zenith compiler toolchain
    const serverCommand = process.env.ZENITH_LSP_PATH || 'zenith-lsp';

    const serverOptions: ServerOptions = {
        command: serverCommand,
        transport: TransportKind.stdio
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [{ scheme: 'file', language: 'zenith' }],
        synchronize: {
            fileEvents: vscode.workspace.createFileSystemWatcher('**/*.zen')
        }
    };

    client = new LanguageClient(
        'zenithLanguageServer',
        'Zenith Language Server',
        serverOptions,
        clientOptions
    );

    client.start();
    context.subscriptions.push(client);
    
    vscode.window.showInformationMessage('Zenith Language Server started');
}

export function deactivate(): Thenable<void> | undefined {
    if (!client) {
        return undefined;
    }
    return client.stop();
}
