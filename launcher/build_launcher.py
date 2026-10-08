import argparse, os, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from common import ROOT, run
parser = argparse.ArgumentParser(description="Build the standalone Windows launcher")
parser.add_argument("--output-directory", type=Path, default=ROOT / "source/out/launcher")
args = parser.parse_args()
output = args.output_directory.resolve(); output.mkdir(parents=True, exist_ok=True)
compiler = Path(os.environ.get("WINDIR", "C:/Windows")) / "Microsoft.NET/Framework64/v4.0.30319/csc.exe"
icon_tool = output / "IconBuilder.exe"
run(compiler, "/nologo", "/target:exe", "/reference:System.Drawing.dll", "/out:" + str(icon_tool), ROOT / "launcher/IconBuilder.cs")
icon = output / "launcher.ico"
run(icon_tool, ROOT / "launcher/launchericon.png", icon)
exe = output / "MTR-GWRE2.exe"
run(compiler, "/nologo", "/target:winexe", "/platform:x64", "/optimize+", "/out:" + str(exe), "/win32icon:" + str(icon), "/resource:" + str(ROOT / "launcher/launchericon.png") + ",launchericon.png", "/reference:System.Windows.Forms.dll", "/reference:System.Drawing.dll", "/reference:System.Web.Extensions.dll", ROOT / "launcher/Launcher.cs")
print("Launcher:", exe)
