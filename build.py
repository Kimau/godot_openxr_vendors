import os
import sys
import subprocess
import argparse
import winsound
import shutil
from pathlib import Path

# Configuration
PROJECT_PATH = Path(r"C:\Dev\godot_openxr_vendors")
EXTENSION_API_PATH = PROJECT_PATH / "thirdparty/godot_cpp_gdextension_api/extension_api.json"
ADDON_SOURCE = PROJECT_PATH / "demo/addons/godotopenxrvendors"
ADDON_DEST = Path(r"D:\Dev\puzzvr\addons\godotopenxrvendors")
ERROR_SOUND = "C:\\Windows\\Media\\Windows Critical Stop.wav"
SUCCESS_SOUND = "C:\\Windows\\Media\\alarm05.wav"

def error_exit(message: str):
    """
    Print error message, play error sound, and exit with error code
    
    Args:
        message: Error message to display
    """
    print(f"ERROR: {message}")
    winsound.PlaySound(ERROR_SOUND, winsound.SND_FILENAME)
    sys.exit(1)

def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description='Build script for Godot OpenXR Vendors',
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    
    build_group = parser.add_mutually_exclusive_group()
    build_group.add_argument('--windows', action='store_true', help='Build Windows version only')
    build_group.add_argument('--android', action='store_true', help='Build Android version only')
    build_group.add_argument('--full', action='store_true', help='Full build including Windows and Android')
    
    # Clean can be combined with any build mode
    parser.add_argument('--clean', action='store_true', help='Clean build files before building')
    
    return parser.parse_args()

def run_command(cmd: str, cwd: Path | None = None):
    """
    Run a command and handle errors
    
    Args:
        cmd: Command to run
        cwd: Working directory for the command
    """
    try:
        subprocess.run(cmd, cwd=cwd, check=True, shell=True)
    except subprocess.CalledProcessError as e:
        error_exit(f"Command failed with exit code {e.returncode}: {cmd}")

def build_windows():
    """Build Windows version"""
    print("------------------- BUILDING WINDOWS VERSION ----------------------------")
    
    # Build default version
    run_command("scons", cwd=PROJECT_PATH)
    
    # Build debug template
    run_command(
        f"scons target=template_debug custom_api_file={EXTENSION_API_PATH}",
        cwd=PROJECT_PATH
    )
    
    # Build release template
    run_command(
        f"scons target=template_release custom_api_file={EXTENSION_API_PATH}",
        cwd=PROJECT_PATH
    )
    
    print("Windows build completed successfully!")

def build_android():
    """Build Android version using Gradle"""
    print("------------------- BUILDING ANDROID VERSION ----------------------------")
    if os.name == 'nt':
        run_command("gradlew.bat build", cwd=PROJECT_PATH)
    else:
        run_command("./gradlew build", cwd=PROJECT_PATH)
    print("Android build completed successfully!")

def clean_build():
    """Clean build files"""
    print("------------------- CLEANING BUILD FILES ----------------------------")
    
    # Clean scons build for Windows
    run_command("scons -c", cwd=PROJECT_PATH)
    
    # Clean gradle build for Android
    if os.name == 'nt':
        run_command("gradlew.bat clean", cwd=PROJECT_PATH)
    else:
        run_command("./gradlew clean", cwd=PROJECT_PATH)
    
    # Clean specific build directories
    build_dirs = [
        PROJECT_PATH / ".gradle",
        PROJECT_PATH / "plugin/build"
    ]
    
    for build_dir in build_dirs:
        if build_dir.exists():
            try:
                shutil.rmtree(build_dir)
                print(f"Cleaned {build_dir}")
            except Exception as e:
                error_exit(f"Failed to clean {build_dir}: {e}")
    
    print("Build files cleaned successfully!")

def copy_addon():
    """Copy addon files to destination"""
    print("------------------- COPYING ADDON FILES ----------------------------")
    try:
        if ADDON_DEST.exists():
            shutil.rmtree(ADDON_DEST)
        shutil.copytree(ADDON_SOURCE, ADDON_DEST)
        print(f"Successfully copied addon files to {ADDON_DEST}")
    except Exception as e:
        error_exit(f"Failed to copy addon files: {e}")

def main():
    """Main build process"""
    args = parse_args()
    
    os.system('cls' if os.name == 'nt' else 'clear')
    print("Starting build process...")
    
    # Clean if requested
    if args.clean:
        clean_build()
    
    # Determine build mode
    if args.full:
        build_windows()
        build_android()
    elif args.windows:
        build_windows()
    elif args.android:
        build_android()
    else:
        # Default behavior: Windows build
        build_windows()
        
    # Copy addon files
    copy_addon()
    
    # Play success sound
    winsound.PlaySound(SUCCESS_SOUND, winsound.SND_FILENAME)

if __name__ == "__main__":
    main()