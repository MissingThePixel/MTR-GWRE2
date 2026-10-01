# Working on MTR-GWRE2

- Read README.md, BUILDING.md and THIRD_PARTY.md before build/release changes.
- Preserve the game's native 60 FPS behaviour and the profile/presentation fixes.
- Keep game files, translated game C++, saves, caches, logs, dumps and personal
  paths outside version control and release ZIPs.
- Persist fixes in the manifest and maintained source; generated code is disposable.
- Use the pinned patched SDK. Release from an explicit file list with licenses.
- Verify startup for launch changes and use a focused controller test when needed.
