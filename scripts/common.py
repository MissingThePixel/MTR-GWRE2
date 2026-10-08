"""Shared build helpers; uses direct process arguments without a command shell."""
from pathlib import Path
import json, shutil, subprocess, zipfile, hashlib
ROOT = Path(__file__).resolve().parents[1]
PIN = "f5337cdc947ff6d4c4196737e2c807a48f2a1fc2"
def run(*args, **kwargs):
    subprocess.run([str(a) for a in args], check=True, **kwargs)
def patch(sdk, name):
    file = ROOT / "patches" / name
    if subprocess.run(["git", "-C", str(sdk), "apply", "--check", "--reverse", str(file)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0:
        return
    run("git", "-C", sdk, "apply", "--check", file)
    run("git", "-C", sdk, "apply", file)
def manifest(game):
    game = Path(game).resolve(strict=True)
    if not (game / "default.xex").is_file(): raise ValueError("Game folder must contain default.xex")
    import re
    text = (ROOT / "source/gw2_recompiled_manifest.toml").read_text()
    for key, path in [("game_root", game), ("file_path", game / "default.xex")]:
        value = key + " = " + json.dumps(path.as_posix())
        text = re.sub(key + r' = "[^"]*"', lambda match: value, text)
    output = ROOT / "source/gw2_local.toml"
    output.write_text(text, encoding="utf-8")
    return output
def notice_files(package):
    shutil.copy2(ROOT / "LICENSE", package / "LICENSE.txt")
    shutil.copy2(ROOT / "THIRD_PARTY.md", package / "THIRD_PARTY.md")
    directory = package / "LICENSES"; directory.mkdir()
    for file in (ROOT / "LICENSES").glob("*.txt"): shutil.copy2(file, directory / file.name)
def archive(package, destination):
    with zipfile.ZipFile(destination, "w", zipfile.ZIP_DEFLATED) as output:
        for file in sorted(package.rglob("*")):
            if file.is_file(): output.write(file, file.relative_to(package.parent).as_posix())
def sums(files, destination):
    destination.write_text("".join(hashlib.sha256(f.read_bytes()).hexdigest() + "  " + f.name + "\n" for f in files), encoding="utf-8")
