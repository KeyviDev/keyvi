from __future__ import annotations

import shutil
import tempfile
from pathlib import Path

from keyvi.compiler import KeyOnlyDictionaryGenerator
from keyvi.dictionary import Dictionary


def test_generator_basic():
    tmp_dir = tempfile.mkdtemp()
    try:
        filename = str(Path(tmp_dir) / "generator_test.kv")
        generator = KeyOnlyDictionaryGenerator()
        generator.add("a")
        generator.add("b")
        generator.add("c")
        generator.close_feeding()
        generator.write_to_file(filename)

        d = Dictionary(filename)
        assert "a" in d
        assert "b" in d
        assert "c" in d
        assert "d" not in d
        assert len(d) == 3
    finally:
        shutil.rmtree(tmp_dir)
