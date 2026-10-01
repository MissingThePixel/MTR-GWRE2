# MTR-GWRE2

**MissingTheRecompilation: Geometry Wars: Retro Evolved 2**

A Windows PC recompilation of the Xbox 360 game, built with
[ReXGlue](https://github.com/rexglue/rexglue-sdk). Includes 4K rendering,
native 1080p rendering, controller support and an automatic local save profile.
The game's original 60 FPS behaviour is preserved.

## Getting started

1. Download the Windows x64 ZIP from [Releases](https://github.com/MissingThePixel/MTR-GWRE2/releases)
   and extract the whole ZIP to a writable folder.
2. Open **MTR-GWRE2.exe**, use **Browse** to select your complete extracted game
   folder containing `default.xex`, choose 4K or 1080p and click **Play**.
   Your choices are remembered. No game files are included.

Windows x64, .NET Framework 4.8 and a Direct3D 12-capable graphics card are required. The runtime
and renderer are included; you do not need to install ReXGlue to play.
Alt+F4 closes the game. The launcher works when started from another folder,
including through QuiverLauncher. Paths containing spaces are supported.

## Saves and logging

A local User profile is created automatically if no save exists. Your scores
and unlock progress are stored in `userdata`; shader caches are in `cache`.
Keep both folders when updating. Releases contain no pre-existing saves.
New profiles start with the game's normal mode unlock progression.

Logging defaults to warnings and errors in `logs`. Verbose logging and frame,
GPU and fence measurements are disabled.

4K uses the game's native 1920x1080 video mode with 2x rendering in each
dimension (3840x2160). Full-game licensing is enabled in the local runtime.
No frame-rate patch is applied.

## Source and credits

See [BUILDING.md](BUILDING.md) for the pinned SDK and build instructions, and
[THIRD_PARTY.md](THIRD_PARTY.md) for credits and dependency licenses.
Generated game code and original game assets are excluded from the repository.
New project code uses the MIT license; third-party components retain their own
licenses. Geometry Wars and its artwork belong to their respective owners.
