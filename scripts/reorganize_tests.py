import os
import shutil

def reorganize():
    tests_dir = r"c:\Jay\_Plugin\zenith_lang\tests"
    ui_dir = os.path.join(tests_dir, "ui_tests")
    lang_dir = os.path.join(tests_dir, "language_tests")

    os.makedirs(ui_dir, exist_ok=True)
    os.makedirs(lang_dir, exist_ok=True)

    ui_files = [
        "gallery.zen",
        "main.zen",
        "helper.zen",
        "test_widgets.zen",
        "test_npm_chart.zen",
        "website.zen",
        "test_ssr.zen"
    ]

    lang_files = [
        "simple_inference.zen",
        "advanced_inference.zen",
        "comprehensive_type_inference.zen",
        "type_inference_simple.zen",
        "type_inference_test.zen",
        "test_agentic_features.zen",
        "test_async.zen",
        "test_bridge_app.zen",
        "test_concurrency.zen",
        "test_interop.zen",
        "test_memory.zen"
    ]

    # Move files
    for f in ui_files:
        src = os.path.join(tests_dir, f)
        dst = os.path.join(ui_dir, f)
        if os.path.exists(src):
            print(f"Moving {f} to ui_tests/")
            shutil.move(src, dst)

    for f in lang_files:
        src = os.path.join(tests_dir, f)
        dst = os.path.join(lang_dir, f)
        if os.path.exists(src):
            print(f"Moving {f} to language_tests/")
            shutil.move(src, dst)

    print("Reorganization complete!")

if __name__ == "__main__":
    reorganize()
