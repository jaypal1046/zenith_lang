import * as path from 'path';
import * as vscode from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;
let lspProcess: any | undefined;

export function activate(context: vscode.ExtensionContext) {
  console.log('Zenith extension is now active!');

  // Register commands
  context.subscriptions.push(
    vscode.commands.registerCommand('zenith.startLSP', startLSP)
  );
  context.subscriptions.push(
    vscode.commands.registerCommand('zenith.restartLSP', restartLSP)
  );
  context.subscriptions.push(
    vscode.commands.registerCommand('zenith.stopLSP', stopLSP)
  );

  // Auto-start LSP if enabled
  const config = vscode.workspace.getConfiguration('zenith');
  if (config.get<boolean>('lsp.autoStart', true)) {
    startLSP();
  }
}

export function deactivate(): Thenable<void> | undefined {
  return stopLSP();
}

async function startLSP(): Promise<void> {
  const config = vscode.workspace.getConfiguration('zenith');
  
  // Get LSP path from config or auto-detect
  let lspPath = config.get<string>('lsp.path', '');
  
  if (!lspPath) {
    // Auto-detect: try common locations
    const possiblePaths = [
      'zenith-lsp',
      'zenith-lsp.exe',
      path.join(process.cwd(), 'build', 'zenith-lsp'),
      path.join(process.cwd(), 'bin', 'zenith-lsp'),
      path.join(__dirname, '..', '..', 'bin', 'zenith-lsp'),
    ];
    
    for (const p of possiblePaths) {
      try {
        const testPath = vscode.Uri.file(p);
        // Simple heuristic - in production, use fs.access
        lspPath = p;
        break;
      } catch {
        continue;
      }
    }
    
    if (!lspPath) {
      lspPath = 'zenith-lsp'; // Default to PATH lookup
    }
  }

  vscode.window.showInformationMessage(`Starting Zenith LSP: ${lspPath}`);

  // Server options
  const serverOptions: ServerOptions = {
    command: lspPath,
    args: ['--lsp'],
    transport: TransportKind.stdio,
  };

  // Client options
  const clientOptions: LanguageClientOptions = {
    documentSelector: [{ scheme: 'file', language: 'zenith' }],
    synchronize: {
      configurationSection: 'zenith',
      fileEvents: vscode.workspace.createFileSystemWatcher('**/*.zen'),
    },
    outputChannelName: 'Zenith Language Server',
  };

  // Create and start the client
  client = new LanguageClient(
    'zenithLanguageServer',
    'Zenith Language Server',
    serverOptions,
    clientOptions
  );

  try {
    await client.start();
    vscode.window.showInformationMessage('Zenith LSP started successfully!');
  } catch (error) {
    vscode.window.showErrorMessage(`Failed to start Zenith LSP: ${error}`);
  }
}

async function stopLSP(): Promise<void> {
  if (client) {
    try {
      await client.stop();
      client = undefined;
      vscode.window.showInformationMessage('Zenith LSP stopped.');
    } catch (error) {
      vscode.window.showErrorMessage(`Error stopping LSP: ${error}`);
    }
  }
  
  if (lspProcess) {
    try {
      lspProcess.kill();
      lspProcess = undefined;
    } catch {
      // Ignore kill errors
    }
  }
}

async function restartLSP(): Promise<void> {
  await stopLSP();
  setTimeout(() => startLSP(), 500);
}
