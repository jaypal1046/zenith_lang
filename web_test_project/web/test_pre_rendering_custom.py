import os
import sys

def verify_custom_app_web():
    print("=== Verifying Custom Zenith App Web Targets ===")
    
    html_path = "web/main.html"
    wasm_html_path = "web/main_wasm.html"
    wat_path = "web/main.wat"
    
    # 1. Verify existence of assets
    for p in [html_path, wasm_html_path, wat_path]:
        if not os.path.exists(p):
            print(f"FAILURE: Expected generated target file not found: {p}")
            sys.exit(1)
            
    # 2. Check SEO pre-rendered content
    print("Checking static SEO pre-rendered content in main.html and main_wasm.html...")
    with open(html_path, "r", encoding="utf-8") as f:
        html_src = f.read()
    with open(wasm_html_path, "r", encoding="utf-8") as f:
        wasm_html_src = f.read()
        
    keywords = [
        "Zenith Web FFI Demo",
        "Hello, Developer!",
        "Adjust greeting size:",
        "JavaScript FFI Verification",
        "WASM/JS Bridge Status:"
    ]
    
    for kw in keywords:
        if kw not in html_src:
            print(f"FAILURE: JS target main.html is missing SEO pre-rendered keyword: '{kw}'")
            sys.exit(1)
        if kw not in wasm_html_src:
            print(f"FAILURE: WASM target main_wasm.html is missing SEO pre-rendered keyword: '{kw}'")
            sys.exit(1)
            
    print("  [OK] Both web targets contain static pre-rendered SEO content!")
    
    # 3. Check Fastload file size bounds
    size_js = os.path.getsize(html_path) / 1024.0
    size_wasm = os.path.getsize(wasm_html_path) / 1024.0
    
    print(f"  JS build file size: {size_js:.2f} KB")
    print(f"  WASM build file size: {size_wasm:.2f} KB")
    
    if size_js > 50.0 or size_wasm > 50.0:
        print("FAILURE: Compilation output size exceeds Fastload limit of 50KB!")
        sys.exit(1)
        
    print("  [OK] Both targets pass bundle size constraints (< 50KB)!")
    print("\n[ALL PASSED] Custom Zenith project web build validated successfully!")

if __name__ == "__main__":
    verify_custom_app_web()
