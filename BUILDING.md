# Building MTR-GWRE2 on Windows

Use a Developer PowerShell for Visual Studio with the x64 C++ environment. The
launcher also requires .NET Framework 4.8 and its included C# compiler.
Install the Desktop development with C++ workload and Windows SDK, Clang 20+
(clang-cl), CMake 3.25+, Ninja and Git, available on PATH. Developers need their
own complete extracted Xbox 360 game (title 584108FF, version 0.0.1.2).

## Build the SDK

```powershell
powershell -ExecutionPolicy Bypass -File setup-sdk.ps1
```

This obtains ReXGlue v0.10.0 at commit
`f5337cdc947ff6d4c4196737e2c807a48f2a1fc2`, initializes its pinned submodules,
applies the included shared MTR runtime/recompiler patches and source overrides,
and installs a Release SDK in `sdk-install`. Use a dedicated SDK clone.
The same patched runtime is used by MTR-PGR4; do not substitute an unpatched SDK.

## Generate and build the game

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -GameDataRoot "D:\Games\Geometry Wars 2 extracted"
```

Use `-SdkPrefix` for an existing installation of this patched SDK and `-Jobs`
to set compiler parallelism (default 4). The script creates an ignored local
manifest, generates game C++ from your own `default.xex`, and compiles
`source/out/build/gw2_recompiled.exe` and `source/out/launcher/MTR-GWRE2.exe`. Generated game source is disposable and
excluded from Git. `source/generated/rexglue.cmake` is SDK integration boilerplate,
not translated game instructions.

## Package a player release

```powershell
powershell -ExecutionPolicy Bypass -File scripts\package-release.ps1
```

The explicit file list includes the game EXE, runtime DLLs, standalone launcher, README and
license notices. No extracted game files, saves, caches, logs, diagnostic dumps
or generated game C++ are copied. The ZIP is created under `dist`.
`-Executable`, `-LauncherExecutable` and `-RuntimeDirectory` can select already-built binaries.

To build only the launcher, run `launcher/build-launcher.ps1`. The supplied PNG
is embedded in the EXE and converted to its Windows icon during compilation.
The launcher starts the game directly and does not run BAT or PowerShell files.

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
The Android sources live in android/, share the maintained game manifest and
profile code, and use separate build directories. Use setup-sdk.ps1 -VulkanOnly
for the Android host code generator, then scripts/build-android.ps1.
