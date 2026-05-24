import os

def inject_ffi_implementations():
    html_files = ["web/main.html", "web/main_wasm.html"]
    
    ffi_script = """
        // FFI Implementations for Zenith JS/WASM Bridge
        window.triggerBrowserAlert = function(text) {
            alert(text);
        };
        window.getBrowserTime = function() {
            return Math.floor(Date.now() / 1000);
        };
    """
    
    for filename in html_files:
        if not os.path.exists(filename):
            print(f"Skipping {filename} (not found)")
            continue
            
        with open(filename, "r", encoding="utf-8") as f:
            content = f.read()
            
        # If already injected, skip
        if "triggerBrowserAlert" in content and "getBrowserTime" in content and "Math.floor" in content:
            print(f"FFI implementations already exist in {filename}")
            continue
            
        # Find the first `<script>` tag and insert FFI script right after it
        script_idx = content.find("<script>")
        if script_idx == -1:
            print(f"Could not find <script> tag in {filename}")
            continue
            
        insertion_point = script_idx + len("<script>")
        new_content = content[:insertion_point] + ffi_script + content[insertion_point:]
        
        with open(filename, "w", encoding="utf-8") as f:
            f.write(new_content)
            
        print(f"Successfully injected FFI implementations into {filename}")

if __name__ == "__main__":
    inject_ffi_implementations()
