# Credits and third-party code

## ReXGlue and Xenia

This project was initialized with [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk)
v0.10.0, commit `f5337cdc947ff6d4c4196737e2c807a48f2a1fc2`.
Application templates, CMake integration, recompiler and runtime are BSD 3-Clause;
copyright Tom Clay, with portions derived from Ben Vanik and Xenia contributors.
The game manifest's additional function targets and CRT mappings were identified
from local testing of Geometry Wars: Retro Evolved 2.

The shared patched SDK and Windows math compatibility code come from
[MTR-PGR4](https://github.com/MissingThePixel/MTR-PGR4).
Runtime/recompiler changes are supplied as `patches/rexglue-v0.10.0.patch` and
new source files in `sdk-overrides`, preserving original copyright headers.
The patched runtime also draws on [Xenia](https://github.com/xenia-project/xenia)
and [Xenia Canary](https://github.com/xenia-canary/xenia-canary) behaviour.
No PGR4 frame-rate or physics patches are applied to this game.

## Runtime dependencies

- D3D12 Memory Allocator: MIT, commit
  `1d86c1130f61453634b1df85782e1fecfd59a525`; source in
  `sdk-overrides/thirdparty/d3d12ma`.
- FFmpeg: SDK-pinned fork, LGPL. GPL and nonfree features are disabled in this
  runtime build. Retained upstream license files describe optional components.
- libmspack: LGPL; Windows materialization patch in
  `patches/libmspack-windows.patch`.
- SDL, fmt, spdlog, Dear ImGui, SIMDe, xxHash and the SDK's other libraries:
  original notices are included in `LICENSES`, together with dependency commits.

The setup script obtains the pinned dependency sources needed to rebuild the
runtime. Its DLLs may be replaced with rebuilt versions; this project imposes no
restriction on modifying or debugging those dependencies.

The MIT license covers new MTR-GWRE2 code only. Original game instructions,
generated game C++, game assets and trademarks are not relicensed by it.

## Launcher artwork

The launcher uses the Geometry Wars artwork supplied for this project. Game
artwork and trademarks belong to their respective owners; the project MIT
license does not grant rights to that artwork.

## Android

The Android port uses the same pinned ReXGlue SDK and shared game source, with
maintained ARM64 and Vulkan patches in patches/. SDL3 supplies the Android Java
bridge under its zlib license. The Gradle wrapper is Apache 2.0; its notice is in
LICENSES/Gradle-LICENSE.txt. The Vulkan patches correct texture-brightness decoding
and presentation pipeline caching. Game instructions and assets remain excluded.
