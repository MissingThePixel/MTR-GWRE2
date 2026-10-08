from pathlib import Path
import subprocess, sys
subprocess.run([sys.executable, str(Path(__file__).resolve().parents[1] / "scripts/build.py"), *sys.argv[1:]], check=True)
