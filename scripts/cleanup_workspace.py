import os
import shutil
import glob

def cleanup():
    # Root level directories to remove
    dirs_to_remove = [
        r"c:\Jay\_Plugin\zenith_lang\scaffold_test_app",
        r"c:\Jay\_Plugin\zenith_lang\test_app",
        r"c:\Jay\_Plugin\zenith_lang\test_runner_app",
        r"c:\Jay\_Plugin\zenith_lang\web_test_project",
        r"c:\Jay\_Plugin\zenith_lang\chrome_dev_profile",
        r"c:\Jay\_Plugin\zenith_lang\tests\demo_app"
    ]

    for d in dirs_to_remove:
        if os.path.exists(d):
            print(f"Removing directory: {d}")
            try:
                shutil.rmtree(d)
            except Exception as e:
                print(f"Error removing {d}: {e}")

    # Files to remove in tests/
    tests_dir = r"c:\Jay\_Plugin\zenith_lang\tests"
    
    # Specific files
    files_to_remove = [
        "test_math_lib.dart",
        "screen.png",
        "test_seo_fastload.py",
        "bridge.py"
    ]
    
    for f in files_to_remove:
        path = os.path.join(tests_dir, f)
        if os.path.exists(path):
            print(f"Removing file: {path}")
            try:
                os.remove(path)
            except Exception as e:
                print(f"Error removing {path}: {e}")

    # Wildcard matches in tests/
    extensions = ["*.exe", "*.cpp", "*.html", "*.wat", "*.dll"]
    for ext in extensions:
        pattern = os.path.join(tests_dir, ext)
        for path in glob.glob(pattern):
            print(f"Removing file matching pattern: {path}")
            try:
                os.remove(path)
            except Exception as e:
                print(f"Error removing {path}: {e}")

    print("Cleanup complete!")

if __name__ == "__main__":
    cleanup()
