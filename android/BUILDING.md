# Building MTR-GWRE2 for Android

Use a Windows x64 Native Tools Command Prompt for Visual Studio with Clang 20+, CMake, Ninja
and Git, plus the desktop requirements in [../BUILDING.md](../BUILDING.md).
Supply your own complete extracted Xbox 360 game (title 584108FF, version 0.0.1.2).
Install Android SDK platform 35, build-tools 35.0.1, NDK 27.2.12479018 and JDK 17.
Set ANDROID_HOME and JAVA_HOME, or pass --android-sdk and --java-home to the script.
From the repository root:

```text
python setup_sdk.py --vulkan-only
python scripts/build_android.py --game-data-root "D:/Games/Geometry Wars 2 extracted"
```

SDK setup prepares the pinned host SDK and code generator. The Android script
applies the maintained ARM64/Vulkan patches, generates game code, builds the
libraries and assembles android/app/build/outputs/apk/release/app-release.apk.
APK libraries are stripped and linked for 16 KB page compatibility. Builds,
dependencies, game translations, signing keys and logs stay outside Git.

The release is not debuggable. It uses the local Android development signing
certificate to update existing test installations without removing saves. Published
APKs keep the maintainer's existing certificate; rebuilding on another machine uses
that machine's certificate. Preserve the private keystore for future updates.
This package is intended for direct APK installation.

Android retains unchanged CPU-uploaded pages under physical write tracking and
pauses for 50 microseconds only while the original guest GPU poll is pending.
Rendering is native 1080p with FIFO presentation. Guest timing, the automatic
profile and texture-brightness fixes are preserved.

For explicit development diagnostics, assembleDebug -PcrashLogs=true enables
startup and crash reports; the diagnostic activity extra also enables timing.
The release disables ordinary logging and diagnostic activity extras. The native
tests in android/tests cover fibers, physical write tracking and retained memory
coherency. Tests are excluded from the APK.

Package an existing release APK with its README and licenses:

```text
python scripts/package_android.py
```
