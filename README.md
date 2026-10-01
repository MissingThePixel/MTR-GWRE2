# MTR-GWRE2

**MissingTheRecompilation: Geometry Wars: Retro Evolved 2**

A Windows PC recompilation of the Xbox 360 game, built with
[ReXGlue](https://github.com/rexglue/rexglue-sdk). Includes 4K rendering,
native 1080p rendering, controller support and an automatic local save profile.
The game's original 60 FPS behaviour is preserved.

## Getting started

1. Download the Windows x64 ZIP from [Releases](https://github.com/MissingThePixel/MTR-GWRE2/releases)
   and extract the whole ZIP to a writable folder.
2. Copy your own complete extracted Xbox 360 game into the release's `Game`
   folder. `Game/default.xex` and all accompanying game files must be present.
   No game files are included in this project or its releases.
3. Open **run.bat** for 4K fullscreen, **run-native.bat** for 1080p fullscreen,
   or **run-options.bat** to choose between them. Use your controller normally.

Windows x64 and a Direct3D 12-capable graphics card are required. The runtime
and renderer are included; you do not need to install ReXGlue to play.
Alt+F4 closes the game. The launchers work when started from another folder,
including through QuiverLauncher. Paths containing spaces are supported.

## Saves and logging

A local User profile is created automatically if no save exists. Your scores
and unlock progress are stored in `userdata`; shader caches are in `cache`.
Keep both folders when updating. Releases contain no pre-existing saves.
New profiles start with the game's normal mode unlock progression.

Logging defaults to warnings and errors in `logs`. Verbose logging and frame,
GPU and fence measurements are disabled. For troubleshooting, extra runtime
arguments can be passed to a launcher, for example:

```bat
run.bat --log_level=debug --guest_frame_stats=true
```

4K uses the game's native 1920x1080 video mode with 2x rendering in each
dimension (3840x2160). Full-game licensing is enabled in the local runtime.
No frame-rate patch is applied.

## Tested and known limitations

- A complete controller-played Deadline round held 60 FPS at 4K after the
  presentation fix, and the game wrote progress to the local save profile.
- Single Player opens without the missing-profile/storage warning.
- Initial shader compilation may cause brief hitches.
- Other modes, long sessions, multiplayer and achievements need further testing.
- Xbox Live leaderboards are unavailable offline.

## Source and credits

See [BUILDING.md](BUILDING.md) for the pinned SDK and build instructions, and
[THIRD_PARTY.md](THIRD_PARTY.md) for credits and dependency licenses.
Generated game code and original game assets are excluded from the repository.
New project code uses the MIT license; third-party components retain their own
licenses. Geometry Wars and its artwork belong to their respective owners.
