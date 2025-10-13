# run.py
"""
One-click runner for the CoB Kiosk RAG app.

What it does on first run:
  1) Creates a .venv (local to the project)
  2) Installs requirements.txt into that venv
  3) Starts the FastAPI server (uvicorn)
  4) Opens http://127.0.0.1:8000 in your browser

Use: open this folder in VS Code or PyCharm, open run.py, click Run ▶️
"""

from __future__ import annotations
import os, sys, subprocess, pathlib, time, webbrowser, platform, venv, urllib.request

ROOT = pathlib.Path(__file__).parent.resolve()
VENV_DIR = ROOT / ".venv"
REQS = ROOT / "requirements.txt"
HOST = os.environ.get("HOST", "127.0.0.1")
PORT = os.environ.get("PORT", "8000")
APP_IMPORT = "app.main:app"   # don't change unless you move files

def venv_python() -> str:
    if platform.system() == "Windows":
        return str(VENV_DIR / "Scripts" / "python.exe")
    return str(VENV_DIR / "bin" / "python")

def ensure_python_installed():
    try:
        subprocess.check_call([sys.executable, "--version"])
    except Exception:
        print("✖ Python is not available to this process. Install Python 3.11+.")
        sys.exit(1)

def ensure_venv():
    if not VENV_DIR.exists():
        print("• Creating virtual environment (.venv)…")
        venv.EnvBuilder(with_pip=True, clear=False, upgrade=False).create(str(VENV_DIR))

    py = venv_python()
    if not pathlib.Path(py).exists():
        print("✖ Could not find venv python at", py)
        sys.exit(1)

    # Always have the standard build helpers
    print("• Upgrading pip / setuptools / wheel…")
    subprocess.check_call([py, "-m", "pip", "install", "--upgrade", "pip", "setuptools", "wheel"])

    # PREINSTALL binary wheels that sometimes trigger source builds
    # (forces pip to download Windows wheels instead of compiling)
    print("• Preinstalling binary wheels (numpy, chroma-hnswlib)…")
    subprocess.check_call([py, "-m", "pip", "install", "--only-binary=:all:", "numpy==1.26.4", "chroma-hnswlib==0.7.6"])

    # Now install the rest (numpy is already satisfied)
    print(f"• Installing dependencies from {REQS.name}…")
    subprocess.check_call([py, "-m", "pip", "install", "-r", str(REQS), "--no-build-isolation"])


def check_ollama():
    try:
        with urllib.request.urlopen("http://127.0.0.1:11434/api/tags", timeout=2) as r:
            if r.status == 200:
                return True
    except Exception:
        pass
    print("⚠ Ollama is not responding on http://127.0.0.1:11434.")
    print("  Install/start Ollama and pull models (once):")
    print("    - Install: https://ollama.com")
    print("    - Pull models:   ollama pull nomic-embed-text   and   ollama pull gemma3:1b")
    return False

def start_server():
    env = os.environ.copy()
    # Reasonable defaults; change here if you want a different local model
    env.setdefault("LLM_MODEL", "gemma3:1b")
    env.setdefault("CHROMA_TELEMETRY_ENABLED", "false")

    py = venv_python()
    cmd = [py, "-m", "uvicorn", APP_IMPORT, "--host", HOST, "--port", PORT, "--reload"]
    print("\n• Starting server:", " ".join(cmd))
    print(f"• Opening http://{HOST}:{PORT} …\n")

    # Open browser shortly after boot
    def open_browser_soon():
        time.sleep(1.2)
        try:
            webbrowser.open(f"http://{HOST}:{PORT}")
        except Exception:
            pass

    # Launch browser in the background
    import threading
    threading.Thread(target=open_browser_soon, daemon=True).start()

    # Run uvicorn in the foreground (shows logs in the IDE console)
    subprocess.check_call(cmd, env=env, cwd=str(ROOT))

def main():
    ensure_python_installed()
    ensure_venv()
    check_ollama()  # warns if not running, but continues so you can still access UI
    start_server()

if __name__ == "__main__":
    main()
