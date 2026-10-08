import argparse, shutil
from pathlib import Path
from common import ROOT, archive, notice_files, sums
parser = argparse.ArgumentParser(description="Package the Windows launcher, runtime and licenses")
parser.add_argument("--executable", type=Path, default=ROOT / "source/out/build/gw2_recompiled.exe")
parser.add_argument("--launcher-executable", type=Path, default=ROOT / "source/out/launcher/MTR-GWRE2.exe")
parser.add_argument("--runtime-directory", type=Path, default=ROOT / "sdk-install/bin")
parser.add_argument("--output-directory", type=Path, default=ROOT / "dist")
args = parser.parse_args(); output = args.output_directory.resolve()
package = output / "MTR-GWRE2-windows-x64"; zipfile = output / "MTR-GWRE2-windows-x64.zip"
if package.exists() or zipfile.exists(): raise ValueError("Use an empty output directory")
binaries = {"MTR-GWRE2.exe": args.launcher_executable, "gw2_recompiled.exe": args.executable, **{n: args.runtime_directory / n for n in ["rexruntime.dll", "rexgpu-xenos.dll"]}}
for file in binaries.values():
    if not file.is_file(): raise FileNotFoundError(file)
package.mkdir(parents=True)
for name, file in binaries.items(): shutil.copy2(file, package / name)
shutil.copy2(ROOT / "README.md", package / "README.md"); notice_files(package)
(package / "Game").mkdir()
(package / "Game/PLACE-GAME-FILES-HERE.txt").write_text("Copy your complete extracted Geometry Wars: Retro Evolved 2 game here, including default.xex and accompanying assets.")
allowed = set(binaries) | {"README.md", "THIRD_PARTY.md", "LICENSE.txt", "Game/PLACE-GAME-FILES-HERE.txt"}
allowed |= {"LICENSES/" + f.name for f in (ROOT / "LICENSES").glob("*.txt")}
assert {f.relative_to(package).as_posix() for f in package.rglob("*") if f.is_file()} == allowed
archive(package, zipfile)
standalone = output / "MTR-GWRE2.exe"; shutil.copy2(args.launcher_executable, standalone)
sums([zipfile, standalone], output / "SHA256SUMS.txt")
print("Windows release:", zipfile)
