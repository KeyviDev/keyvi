from __future__ import annotations

import subprocess
import sys
from pathlib import Path

import keyvi


def test_merger_binary():
    merger_script = Path(keyvi.__file__).parent / "keyvimerger.py"
    rc = subprocess.call([sys.executable, str(merger_script), "-h"])
    assert rc == 0
