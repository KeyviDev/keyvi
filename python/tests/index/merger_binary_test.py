from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import keyvi


def test_merger_binary():
    package_dir = Path(keyvi.__file__).parent
    # Cython extension places keyvimerger.py in _pycore/
    merger_script = package_dir / "keyvimerger.py"
    if not merger_script.exists():
        merger_script = package_dir / "_pycore" / "keyvimerger.py"
    rc = subprocess.call([sys.executable, str(merger_script), "-h"])
    assert rc == 0
