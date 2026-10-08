import argparse, shutil
from pathlib import Path
from common import ROOT, archive, notice_files, sums
parser = argparse.ArgumentParser(description="Package the Android APK and licenses")
parser.add_argument("--apk", type=Path, default=ROOT / "android/app/build/outputs/apk/release/app-release.apk")
parser.add_argument("--output-directory", type=Path, default=ROOT / "dist/android-v0.1.0")
args = parser.parse_args(); output = args.output_directory.resolve()
package = output / "MTR-GWRE2-android-arm64"; zipfile = output / "MTR-GWRE2-android-arm64.zip"; standalone = output / "MTR-GWRE2-android-arm64.apk"
if any(p.exists() for p in [package, zipfile, standalone]): raise ValueError("Use an empty output directory")
if not args.apk.is_file(): raise FileNotFoundError(args.apk)
package.mkdir(parents=True)
shutil.copy2(args.apk, standalone); shutil.copy2(args.apk, package / standalone.name)
text = (ROOT / "android/README.md").read_text().replace("(BUILDING.md)", "(https://github.com/MissingThePixel/MTR-GWRE2/blob/main/android/BUILDING.md)").replace("(../THIRD_PARTY.md)", "(THIRD_PARTY.md)").replace("(../LICENSE)", "(LICENSE.txt)")
(package / "README.md").write_text(text, encoding="utf-8"); notice_files(package)
allowed = {standalone.name, "README.md", "LICENSE.txt", "THIRD_PARTY.md"} | {"LICENSES/" + f.name for f in (ROOT / "LICENSES").glob("*.txt")}
assert {f.relative_to(package).as_posix() for f in package.rglob("*") if f.is_file()} == allowed
archive(package, zipfile); sums([standalone, zipfile], output / "SHA256SUMS.txt")
print("Android package:", zipfile)
