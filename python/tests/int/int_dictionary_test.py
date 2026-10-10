# Usage: py.test tests
from __future__ import annotations

import json

import pytest
from test_tools import tmp_dictionary

from keyvi import compiler


def test_manifest():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("Leela", 20)
    c["Kif"] = 2
    c.set_manifest('{"drink": "slurm"}')
    with tmp_dictionary(c, "slurm.kv") as d:
        m = json.loads(d.manifest())
        assert m["drink"] == "slurm"


def test_limit_max():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("a", 9223372036854775)
    c.add("b", 2**64 - 1)
    c.add("c", 0)
    with tmp_dictionary(c, "int.kv") as d:
        assert d.get("a").value == 9223372036854775
        assert d.get("b").value == (2**64 - 1)
        assert d.get("c").value == 0


def test_limit_overflow():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    with pytest.raises((OverflowError, TypeError)):
        c.add("a", 2**64)


def test_limit_underflow():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    with pytest.raises((AssertionError, TypeError)):
        c.add("a", -1)
