import os
import sys
import shutil
import subprocess
import glob

def run_cmd(cmd, cwd=None):
    print(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    res = subprocess.run(cmd, shell=True, cwd=cwd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error executing command. Code: {res.returncode}")
        print(f"Stdout:\n{res.stdout}")
        print(f"Stderr:\n{res.stderr}")
    return res

def main():
    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    tests_dir = os.path.join(root_dir, "tests")
    ui_tests_dir = os.path.join(tests_dir, "ui_tests")
    lang_tests_dir = os.path.join(tests_dir, "language_tests")
    sandbox_dir = os.path.join(tests_dir, "sandbox")

    print("=== Zenith Automated Test Runner & Reorganizer ===")

    # 1. Ensure directories exist
    os.makedirs(ui_tests_dir, exist_ok=True)
    os.makedirs(lang_tests_dir, exist_ok=True)

    # 2. Dynamic Reorganization
    ui_files = ["gallery.zen", "main.zen", "helper.zen", "test_widgets.zen", "test_npm_chart.zen", "website.zen", "test_ssr.zen"]
    lang_files = ["simple_inference.zen", "advanced_inference.zen", "comprehensive_type_inference.zen", "type_inference_simple.zen", "type_inference_test.zen", "test_agentic_features.zen", "test_async.zen", "test_bridge_app.zen", "test_concurrency.zen", "test_interop.zen", "test_memory.zen"]

    for f in ui_files:
        src = os.path.join(tests_dir, f)
        dst = os.path.join(ui_tests_dir, f)
        if os.path.exists(src):
            print(f"Reorganizing: Moving {f} to ui_tests/")
            shutil.move(src, dst)

    for f in lang_files:
        src = os.path.join(tests_dir, f)
        dst = os.path.join(lang_tests_dir, f)
        if os.path.exists(src):
            print(f"Reorganizing: Moving {f} to language_tests/")
            shutil.move(src, dst)

    # 3. Clean up obsolete files & directories
    dirs_to_remove = ["scaffold_test_app", "test_app", "test_runner_app", "web_test_project", "chrome_dev_profile", "tests/demo_app"]
    for d in dirs_to_remove:
        path = os.path.join(root_dir, d)
        if os.path.exists(path):
            print(f"Cleaning up directory: {d}")
            shutil.rmtree(path, ignore_errors=True)

    files_to_remove = ["test_math_lib.dart", "screen.png", "test_seo_fastload.py", "bridge.py", "scripts/reorganize_tests.py", "scripts/cleanup_workspace.py"]
    for f in files_to_remove:
        path = os.path.join(root_dir, f) if "/" in f or "\\" in f else os.path.join(tests_dir, f)
        if os.path.exists(path):
            print(f"Cleaning up file: {f}")
            try:
                os.remove(path)
            except Exception as e:
                print(f"Could not delete {f}: {e}")

    extensions = ["*.exe", "*.cpp", "*.html", "*.wat", "*.dll"]
    for ext in extensions:
        for path in glob.glob(os.path.join(tests_dir, ext)):
            print(f"Cleaning up generated test file: {os.path.basename(path)}")
            try:
                os.remove(path)
            except Exception as e:
                print(f"Could not delete {path}: {e}")

    # 4. Compile Compiler (zenith.exe)
    print("\n--- [Step 1/4] Building Zenith Compiler ---")
    build_cmd = [
        "g++", "-O3", "-std=c++17",
        "src/main.cpp", "src/frontend/lexer.cpp", "src/frontend/parser.cpp",
        "src/frontend/semantic.cpp", "src/frontend/formatter.cpp", "src/lsp/lsp.cpp",
        "src/backend/codegen.cpp", "src/backend/js_codegen.cpp", "src/backend/wasm_codegen.cpp",
        "-I", "include", "-o", "zenith.exe", "-lws2_32", "-lpthread"
    ]
    res = run_cmd(build_cmd, cwd=root_dir)
    if res.returncode != 0:
        print("CRITICAL: Failed to build zenith.exe compiler binary.")
        sys.exit(1)
    print("[OK] zenith.exe built successfully.")

    # Prepare sandbox
    if os.path.exists(sandbox_dir):
        shutil.rmtree(sandbox_dir, ignore_errors=True)
    os.makedirs(sandbox_dir, exist_ok=True)

    zenith_exe = os.path.join(root_dir, "zenith.exe")

    # 5. UI Apps Testing (Template App)
    print("\n--- [Step 2/4] Scaffolding UI Sandbox App & Running UI Tests ---")
    ui_sandbox = os.path.join(sandbox_dir, "ui_app")
    create_app_cmd = [zenith_exe, "create", ui_sandbox, "--template=app"]
    res = run_cmd(create_app_cmd, cwd=root_dir)
    if res.returncode != 0:
        print("CRITICAL: Failed to scaffold sandbox app project.")
        sys.exit(1)

    ui_test_files = [f for f in os.listdir(ui_tests_dir) if f.endswith(".zen") and f != "helper.zen"]
    passed_ui = 0
    failed_ui = 0

    for test_file in ui_test_files:
        print(f"\n[UI Test] {test_file} ...")
        # Copy to lib/main.zen
        src_path = os.path.join(ui_tests_dir, test_file)
        dest_main = os.path.join(ui_sandbox, "lib", "main.zen")
        shutil.copy2(src_path, dest_main)

        # Copy helper if main.zen
        if test_file == "main.zen":
            shutil.copy2(os.path.join(ui_tests_dir, "helper.zen"), os.path.join(ui_sandbox, "lib", "helper.zen"))

        # Test transpiling targets
        success = True
        
        # Target CPP
        dest_cpp = os.path.join(ui_sandbox, "desktop", "main.cpp")
        transpile_cpp_cmd = [zenith_exe, dest_main, "-target", "cpp", "-o", dest_cpp]
        if run_cmd(transpile_cpp_cmd, cwd=root_dir).returncode != 0:
            success = False
            print(f"  [FAIL] CPP transpile for {test_file}")

        # Build generated CPP
        if success:
            exe_out = os.path.join(ui_sandbox, "desktop", "main_app.exe")
            yoga_sources = (
                glob.glob(os.path.join(root_dir, "lib", "yoga", "*.cpp")) +
                glob.glob(os.path.join(root_dir, "lib", "yoga", "event", "*.cpp")) +
                glob.glob(os.path.join(root_dir, "lib", "yoga", "internal", "*.cpp"))
            )
            gpp_cmd = [
                "g++", "-O3", "-std=c++17", "-DYOGA_AVAILABLE",
                dest_cpp, os.path.join(root_dir, "src", "zenith", "ui", "yoga_layout.cpp")
            ] + yoga_sources + [
                "-I", "include", "-I", "lib", "-o", exe_out, "-lwinhttp", "-lws2_32"
            ]
            if run_cmd(gpp_cmd, cwd=root_dir).returncode != 0:
                success = False
                print(f"  [FAIL] C++ Native Build for {test_file}")

        # Target Web
        dest_html = os.path.join(ui_sandbox, "web", "main.html")
        transpile_web_cmd = [zenith_exe, dest_main, "-target", "web", "-o", dest_html]
        if run_cmd(transpile_web_cmd, cwd=root_dir).returncode != 0:
            success = False
            print(f"  [FAIL] Web transpile for {test_file}")

        # Target WASM
        dest_wasm_html = os.path.join(ui_sandbox, "web", "main_wasm.html")
        transpile_wasm_cmd = [zenith_exe, dest_main, "-target", "wasm", "-o", dest_wasm_html]
        if run_cmd(transpile_wasm_cmd, cwd=root_dir).returncode != 0:
            success = False
            print(f"  [FAIL] WASM transpile for {test_file}")

        if success:
            print(f"  [PASS] UI test targets successfully built: {test_file}")
            passed_ui += 1
        else:
            failed_ui += 1

    # 6. Language Packages Testing (Template Package)
    print("\n--- [Step 3/4] Scaffolding Language Sandbox Package & Running Language Tests ---")
    lang_sandbox = os.path.join(sandbox_dir, "lang_pkg")
    create_pkg_cmd = [zenith_exe, "create", lang_sandbox, "--template=package"]
    res = run_cmd(create_pkg_cmd, cwd=root_dir)
    if res.returncode != 0:
        print("CRITICAL: Failed to scaffold sandbox package project.")
        sys.exit(1)

    lang_test_files = [f for f in os.listdir(lang_tests_dir) if f.endswith(".zen")]
    passed_lang = 0
    failed_lang = 0

    for test_file in lang_test_files:
        print(f"\n[Language Test] {test_file} ...")
        src_path = os.path.join(lang_tests_dir, test_file)
        dest_main = os.path.join(lang_sandbox, "lib", "main.zen")
        shutil.copy2(src_path, dest_main)

        success = True
        dest_cpp = os.path.join(lang_sandbox, "main.cpp")
        transpile_cmd = [zenith_exe, dest_main, "-target", "cpp", "-o", dest_cpp]
        if run_cmd(transpile_cmd, cwd=root_dir).returncode != 0:
            success = False
            print(f"  [FAIL] Package transpile for {test_file}")

        if success:
            exe_out = os.path.join(lang_sandbox, "main_app.exe")
            gpp_cmd = ["g++", "-O3", "-std=c++17", dest_cpp, "-I", "include", "-o", exe_out, "-lws2_32"]
            if run_cmd(gpp_cmd, cwd=root_dir).returncode != 0:
                success = False
                print(f"  [FAIL] Package Native C++ Build for {test_file}")

        if success:
            # Run the executable and verify exit code 0
            run_res = run_cmd([exe_out], cwd=lang_sandbox)
            if run_res.returncode != 0:
                success = False
                print(f"  [FAIL] Package runtime execution failed for {test_file}")
            else:
                print(f"  [OK] Output:\n{run_res.stdout.strip()}")

        if success:
            print(f"  [PASS] Language test executed successfully: {test_file}")
            passed_lang += 1
        else:
            failed_lang += 1

    # 7. Sandbox Clean-up
    print("\n--- [Step 4/4] Cleaning Sandbox Folder ---")
    shutil.rmtree(sandbox_dir, ignore_errors=True)
    print("[OK] Sandbox directory cleaned.")

    # Summary
    print("\n===================================================")
    print("                TEST RESULTS SUMMARY                ")
    print("===================================================")
    print(f"  UI App Tests       :  {passed_ui} Passed  /  {failed_ui} Failed")
    print(f"  Language Pkg Tests :  {passed_lang} Passed  /  {failed_lang} Failed")
    print("===================================================")

    if failed_ui > 0 or failed_lang > 0:
        print(">>> SOME TESTS FAILED! <<<")
        sys.exit(1)
    else:
        print(">>> ALL TESTS PASSED SUCCESSFULLY! <<<")
        sys.exit(0)

if __name__ == "__main__":
    main()
