import argparse, sys
from pathlib import Path
from common import ROOT, run, manifest
parser = argparse.ArgumentParser(description="Generate and build the Windows game")
parser.add_argument("--game-data-root", required=True, type=Path)
parser.add_argument("--sdk-prefix", type=Path, default=ROOT / "sdk-install")
parser.add_argument("--jobs", type=int, default=4)
args = parser.parse_args(); sdk = args.sdk_prefix.resolve(strict=True)
local = manifest(args.game_data_root)
run(sdk / "bin/rexglue.exe", "codegen", local)
build = ROOT / "source/out/build"
run("cmake", "-S", ROOT / "source", "-B", build, "-G", "Ninja", "-DCMAKE_CXX_COMPILER=clang-cl", "-DCMAKE_BUILD_TYPE=Release", "-DCMAKE_PREFIX_PATH=" + str(sdk), "-DMTR_GAME_MANIFEST=" + str(local))
run("cmake", "--build", build, "-j", args.jobs)
run(sys.executable, ROOT / "launcher/build_launcher.py")
