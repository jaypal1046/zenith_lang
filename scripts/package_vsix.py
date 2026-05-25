import os
import zipfile

def main():
    workspace_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    vscode_dir = os.path.join(workspace_root, 'editors', 'vscode')
    vsix_path = os.path.join(workspace_root, 'zenith-1.0.0.vsix')
    
    print(f"Packaging VS Code extension from: {vscode_dir}")
    print(f"Output package: {vsix_path}")
    
    if not os.path.exists(vscode_dir):
        print(f"Error: {vscode_dir} does not exist!")
        return

    with zipfile.ZipFile(vsix_path, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        for root, dirs, files in os.walk(vscode_dir):
            for file in files:
                abs_path = os.path.join(root, file)
                # Compute path relative to editors/vscode
                rel_path = os.path.relpath(abs_path, vscode_dir)
                
                # Make sure the relative path uses forward slashes inside the ZIP file
                zip_path = rel_path.replace('\\', '/')
                
                print(f" Adding: {zip_path}")
                zip_file.write(abs_path, zip_path)
                
    print("Extension packaged successfully as zenith-1.0.0.vsix")

if __name__ == '__main__':
    main()
