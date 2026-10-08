# MTR-GWRE2 for Android

**MissingTheRecompilation: Geometry Wars: Retro Evolved 2**

An ARM64 Android recompilation of the Xbox 360 game, built with
[ReXGlue](https://github.com/rexglue/rexglue-sdk) and Vulkan. Includes native
1080p rendering, controller support and an automatic local save profile.
The game's original 60 FPS behaviour is preserved.

## Getting started

1. Download **MTR-GWRE2-android-arm64.apk** from the
   [Android release](https://github.com/MissingThePixel/MTR-GWRE2/releases/tag/android-v0.1.0)
   and install it. Allow installation from your browser or file manager if prompted.
2. Open **MTR-GWRE2**, choose **Allow file access** and enable **All files access**
   in Android's settings. Return to the app.
3. Copy your complete extracted game files into **GWRE2** in the root of internal
   shared storage, with `default.xex` directly inside that folder.
4. Choose **Open game** and play with your controller.

Android 9 or later, an ARM64 device and a compatible Vulkan 1.1 GPU are required.
Android 9/10 use the ordinary storage permission instead. The APK includes the
runtime and renderer; no separate emulator installation is needed. No game files
are included. `GWRE2/Game/default.xex` is also accepted.

## Saves and updates

A local User profile is created automatically if no save exists. Scores and unlock
progress are stored privately by the app, together with its shader cache. Install
updates over the existing app to keep them. Uninstalling removes private saves and
caches but leaves the shared **GWRE2** game folder in place. Existing development
installations using the app's external `files/Game` folder remain supported.
New profiles start with the game's normal mode unlock progression.

Runtime logging, session notes, crash-report export and performance measurements
are disabled in this release. Full-game licensing is enabled in the local runtime.
No frame-rate or game-clock patch is applied.

## Source and credits

The Android source is in the [android folder](https://github.com/MissingThePixel/MTR-GWRE2/tree/main/android)
of the shared MTR-GWRE2 repository. See [BUILDING.md](BUILDING.md) for build
instructions, [../THIRD_PARTY.md](../THIRD_PARTY.md) for credits and dependency
licenses, and [../LICENSE](../LICENSE) for the project license.
Generated game code and original game assets are excluded from the repository.
New project code uses the MIT license; third-party components retain their own
licenses. Geometry Wars and its artwork belong to their respective owners.
