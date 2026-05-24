import subprocess
import os
import sys

def test_seo_fastload():
    print("=== Zenith SEO & Fastload E2E Compiler Tests ===")
    
    # 1. Compile website.zen to standard HTML (JS target)
    print("[1/4] Compiling website.zen to JS target (website.html)...")
    res = subprocess.run([r".\zenith.exe", r"tests/website.zen", "-target", "web", "-o", r"tests/website.html"], capture_output=True, text=True)
    if res.returncode != 0:
        print("Error compiling to JS target:", res.stderr)
        sys.exit(1)
    
    # 2. Compile website.zen to WebAssembly target (website_wasm.html + website.wat)
    print("[2/4] Compiling website.zen to WASM target (website_wasm.html)...")
    res = subprocess.run([r".\zenith.exe", r"tests/website.zen", "-target", "wasm", "-o", r"tests/website_wasm.html"], capture_output=True, text=True)
    if res.returncode != 0:
        print("Error compiling to WASM target:", res.stderr)
        sys.exit(1)

    print("[3/4] Running SEO checks (pre-rendered static content)...")
    
    # Verify website.html
    html_path = "tests/website.html"
    with open(html_path, "r", encoding="utf-8") as f:
        html_content = f.read()
    
    # Ensure pre-rendered HTML contains the title and description inside zenith-ui-root
    seo_text_1 = "ZENITH NATIVE WASM ENGINE"
    seo_text_2 = "No Javascript VMs. No Garbage Collector Pauses. Pure Systems Performance."
    
    if f'<div id="zenith-ui-root">' not in html_content:
        print("FAILURE: website.html is missing zenith-ui-root div tag!")
        sys.exit(1)
        
    if seo_text_1 not in html_content or seo_text_2 not in html_content:
        print("FAILURE: website.html does not contain pre-rendered static text for SEO!")
        sys.exit(1)
        
    print("  [OK] Standard Web Target SEO check passed!")

    # Verify website_wasm.html
    wasm_html_path = "tests/website_wasm.html"
    with open(wasm_html_path, "r", encoding="utf-8") as f:
        wasm_html_content = f.read()
        
    if f'<div id="app-root">' not in wasm_html_content:
        print("FAILURE: website_wasm.html is missing app-root div tag!")
        sys.exit(1)
        
    if seo_text_1 not in wasm_html_content or seo_text_2 not in wasm_html_content:
        print("FAILURE: website_wasm.html does not contain pre-rendered static text for SEO!")
        sys.exit(1)
        
    print("  [OK] WASM Target SEO check passed!")

    print("[4/4] Running Fastload checks (bundle size analysis)...")
    
    # Size checks: should be very small (under 50KB)
    size_js = os.path.getsize(html_path) / 1024.0
    size_wasm_html = os.path.getsize(wasm_html_path) / 1024.0
    
    print(f"  Standard HTML file size: {size_js:.2f} KB")
    print(f"  WASM HTML loader file size: {size_wasm_html:.2f} KB")
    
    if size_js > 50.0:
        print(f"FAILURE: website.html size ({size_js:.2f} KB) exceeds Fastload threshold (50KB)!")
        sys.exit(1)
        
    if size_wasm_html > 50.0:
        print(f"FAILURE: website_wasm.html size ({size_wasm_html:.2f} KB) exceeds Fastload threshold (50KB)!")
        sys.exit(1)
        
    print("  [OK] Fastload bundle size checks passed!")
    print("\n[ALL PASSED] SEO and Fastload compiler testing completed successfully!")

if __name__ == "__main__":
    test_seo_fastload()
