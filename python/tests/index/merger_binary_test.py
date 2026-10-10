from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def test_merger_binary():
    module_file = __import__(os.environ.get("KEYVI_MODULE_OVERWRITE", "keyvi")).__file__
    merger_script = Path(module_file).parent / "keyvimerger.py"
    rc = subprocess.call([sys.executable, str(merger_script), "-h"])
    assert rc == 0
