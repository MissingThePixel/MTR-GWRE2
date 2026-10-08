import argparse, os, shutil, subprocess
from pathlib import Path
from common import ROOT, PIN, run, patch, manifest
parser = argparse.ArgumentParser(description="Build the Android ARM64 release APK")
parser.add_argument("--game-data-root", required=True, type=Path)
parser.add_argument("--android-sdk", type=Path, default=Path(os.environ.get("ANDROID_HOME", str(Path(os.environ.get("LOCALAPPDATA", ".")) / "Android/Sdk"))))
parser.add_argument("--java-home", type=Path, default=Path(os.environ.get("JAVA_HOME", ".")))
parser.add_argument("--host-sdk-prefix", type=Path, default=ROOT / "sdk-install-vulkan")
parser.add_argument("--ndk-version", default="27.2.12479018")
parser.add_argument("--jobs", type=int, default=4)
args = parser.parse_args(); android = ROOT / "android"; sdk = ROOT / "thirdparty/rexglue-sdk"
ndk = args.android_sdk.resolve(strict=True) / "ndk" / args.ndk_version
java = args.java_home.resolve(strict=True) / "bin/java.exe"
codegen = args.host_sdk_prefix.resolve(strict=True) / "bin/rexglue.exe"
if subprocess.check_output(["git", "-C", str(sdk), "rev-parse", "HEAD"], text=True).strip() != PIN: raise ValueError("Prepare the pinned SDK using setup_sdk.py --vulkan-only")
for name in ["rexglue-vulkan-texture-exponent.patch", "rexglue-vulkan-present-pipeline.patch", "rexglue-android-arm64.patch"]: patch(sdk, name)
run(codegen, "codegen", manifest(args.game_data_root))
common = ["-G", "Ninja", "-DCMAKE_TOOLCHAIN_FILE=" + str(ndk / "build/cmake/android.toolchain.cmake"), "-DANDROID_ABI=arm64-v8a", "-DANDROID_PLATFORM=android-28", "-DANDROID_STL=c++_shared", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384"]
sdk_build = android / "out/sdk-arm64"; game_build = android / "out/game-arm64"
run("cmake", "-S", sdk, "-B", sdk_build, *common, "-DREXGLUE_USE_VULKAN=ON", "-DREXGLUE_USE_D3D12=OFF", "-DREXGLUE_ENABLE_TRACY=OFF", "-DREXGLUE_ENABLE_PERF_COUNTERS=OFF")
run("cmake", "--build", sdk_build, "--target", "rexruntime", "rexgpu-xenos", "-j", args.jobs)
run("cmake", "-S", android, "-B", game_build, *common)
run("cmake", "--build", game_build, "-j", args.jobs)
stage = android / "app/src/main/jniLibs/arm64-v8a"; stage.mkdir(parents=True, exist_ok=True)
for name in ["librexruntime.so", "librexgpu-xenos.so", "libSDL3.so"]: shutil.copy2(sdk / "out/android-arm64" / name, stage / name)
shutil.copy2(game_build / "libmain.so", stage / "libmain.so")
host = ndk / "toolchains/llvm/prebuilt/windows-x86_64"
shutil.copy2(host / "sysroot/usr/lib/aarch64-linux-android/libc++_shared.so", stage / "libc++_shared.so")
for file in stage.glob("*.so"): run(host / "bin/llvm-strip.exe", "--strip-unneeded", file)
(android / "local.properties").write_text("sdk.dir=" + args.android_sdk.resolve().as_posix() + "\n")
env = dict(os.environ, JAVA_HOME=str(args.java_home.resolve()), GRADLE_USER_HOME=str(android / "tools/gradle-cache"))
run(java, "-classpath", android / "gradle/wrapper/gradle-wrapper.jar", "org.gradle.wrapper.GradleWrapperMain", "-p", android, "assembleRelease", "--no-daemon", "--console=plain", env=env)
print("APK:", android / "app/build/outputs/apk/release/app-release.apk")
