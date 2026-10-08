import argparse, shutil
from pathlib import Path
from scripts.common import ROOT, PIN, run, patch
parser = argparse.ArgumentParser(description="Prepare the pinned patched SDK")
parser.add_argument("--sdk-directory", type=Path, default=ROOT / "thirdparty/rexglue-sdk")
parser.add_argument("--jobs", type=int, default=4)
parser.add_argument("--vulkan-only", action="store_true")
args = parser.parse_args(); sdk = args.sdk_directory.resolve()
if not (sdk / ".git").exists(): run("git", "-c", "core.longpaths=true", "clone", "--branch", "v0.10.0", "--depth", "1", "https://github.com/rexglue/rexglue-sdk.git", sdk)
import subprocess
if subprocess.check_output(["git", "-C", str(sdk), "rev-parse", "HEAD"], text=True).strip() != PIN: raise ValueError("Use a dedicated SDK clone at the pinned commit")
run("git", "-c", "core.longpaths=true", "-C", sdk, "submodule", "update", "--init", "--recursive")
patch(sdk, "rexglue-v0.10.0.patch")
if args.vulkan_only:
    patch(sdk, "rexglue-vulkan-texture-exponent.patch")
    patch(sdk, "rexglue-vulkan-present-pipeline.patch")
patch(sdk / "thirdparty/libmspack", "libmspack-windows.patch")
for file in (ROOT / "sdk-overrides").rglob("*"):
    if file.is_file():
        target = sdk / file.relative_to(ROOT / "sdk-overrides")
        target.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(file, target)
build = sdk / ("out/build/mtr-vulkan" if args.vulkan_only else "out/build/mtr-release")
prefix = ROOT / ("sdk-install-vulkan" if args.vulkan_only else "sdk-install")
backend = ["-DREXGLUE_USE_VULKAN=ON", "-DREXGLUE_USE_D3D12=OFF", "-DCMAKE_C_FLAGS=/clang:-msse4.1", "-DCMAKE_CXX_FLAGS=/DWIN32 /D_WINDOWS /GR /EHsc /clang:-msse4.1 /clang:-Wno-everything"] if args.vulkan_only else []
run("cmake", "-S", sdk, "-B", build, "-G", "Ninja", "-DCMAKE_C_COMPILER=clang-cl", "-DCMAKE_CXX_COMPILER=clang-cl", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_INSTALL_PREFIX=" + str(prefix), *backend)
run("cmake", "--build", build, "--target", "install", "-j", args.jobs)
