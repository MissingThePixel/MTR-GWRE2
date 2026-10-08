# Building MTR-GWRE2 on Windows

Use the x64 Native Tools Command Prompt for Visual Studio. Install the Desktop
development with C++ workload, Windows SDK, Clang 20+ (clang-cl), CMake 3.25+,
Ninja, Git and Python 3.10+ on PATH. The launcher requires .NET Framework 4.8
and its included C# compiler. Players do not need Python or developer tools.
Developers need their own complete extracted Xbox 360 game (title 584108FF,
version 0.0.1.2).

## Build the SDK

```text
python setup_sdk.py
```

This obtains ReXGlue v0.10.0 at commit
`f5337cdc947ff6d4c4196737e2c807a48f2a1fc2`, initializes its pinned submodules,
applies the included shared MTR runtime/recompiler patches and source overrides,
and installs a Release SDK in `sdk-install`. Use a dedicated SDK clone.
The same patched runtime is used by MTR-PGR4; do not substitute an unpatched SDK.

## Generate and build the game

```text
python scripts/build.py --game-data-root "D:/Games/Geometry Wars 2 extracted"
```

Use `--sdk-prefix` for an existing installation of this patched SDK and `--jobs`
to set compiler parallelism (default 4). The script creates an ignored local
manifest, generates game C++ from your own `default.xex`, and compiles
`source/out/build/gw2_recompiled.exe` and `source/out/launcher/MTR-GWRE2.exe`.
Generated game source is disposable and excluded from Git.
`source/generated/rexglue.cmake` is SDK integration boilerplate, not translated
game instructions.

## Package a player release

```text
python scripts/package_release.py
```

The explicit file list includes the game EXE, runtime DLLs, standalone launcher,
README and license notices. No extracted game files, saves, caches, logs,
diagnostic dumps or generated game C++ are copied. The ZIP, separate launcher
EXE and checksums are created under `dist`. The separate launcher EXE is for
updating an existing installation; new installations need the complete ZIP.
Use `--output-directory` to select a fresh release folder.
`--executable`, `--launcher-executable` and `--runtime-directory` can select
already-built binaries.

To build only the launcher:

```text
python launcher/build_launcher.py
```

The supplied PNG is embedded in the EXE and converted to its Windows icon during
compilation. The launcher starts the game directly with paths relative to its
installation folder. It needs no command interpreter or script runner.

## Maintained fixes

- `source/gw2_recompiled_manifest.toml`: discovered function targets and CRT
  setjmp/longjmp mappings used by code generation.
- `source/src/gw2_recompiled_app.h`: first-launch profile defaults, preserving
  existing saves; disables the permanent achievement notification drawer that
  caused guest frame generation to slow toward 30 FPS.
- `source/src/compat_math.cpp`: Windows CRT roundevenf compatibility.
- `patches` and `sdk-overrides`: shared runtime and recompiler modifications.

Do not modify original game assets or commit generated game instructions.

## Android

See [android/BUILDING.md](android/BUILDING.md) for the ARM64/Vulkan APK build.
The Android sources live in `android/`, share the maintained game manifest and
profile code, and use separate build directories. Prepare the Android host code
generator with `python setup_sdk.py --vulkan-only`, then use
`python scripts/build_android.py --game-data-root "D:/Games/Geometry Wars 2 extracted"`.
