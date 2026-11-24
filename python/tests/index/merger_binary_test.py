from __future__ import annotations

import os
import subprocess
import sys
import tempfile
from pathlib import Path

from keyvi.compiler import JsonDictionaryCompiler
from keyvi.dictionary import Dictionary

import keyvi


def merger_script():
    package_dir = Path(keyvi.__file__).parent
    # Cython extension places keyvimerger.py in _pycore/
    script = package_dir / "keyvimerger.py"
    if not script.exists():
        script = package_dir / "_pycore" / "keyvimerger.py"
    return script


def test_merger_binary():
    rc = subprocess.call([sys.executable, str(merger_script()), "-h"])
    assert rc == 0


def test_merger_binary_memory_limit():
    with tempfile.TemporaryDirectory() as test_dir:
        segment = os.path.join(test_dir, "segment.kv")
        c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
        c.add("a", '{"x": 1}')
        c.compile()
        c.write_to_file(segment)
        merged = os.path.join(test_dir, "merged.kv")

        # the memory limit is passed to the merger
        rc = subprocess.call(
            [
                sys.executable,
                str(merger_script()),
                "-m",
                "5242880",
                "-i",
                segment,
                "-o",
                merged,
            ]
        )
        assert rc == 0
        assert Dictionary(merged)["a"].value == {"x": 1}

        # an invalid value is not silently ignored
        rc = subprocess.call(
            [
                sys.executable,
                str(merger_script()),
                "-m",
                "invalid",
                "-i",
                segment,
                "-o",
                merged,
            ],
            stderr=subprocess.DEVNULL,
        )
        assert rc != 0
