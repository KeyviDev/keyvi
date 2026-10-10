# -*- coding: utf-8 -*-
# Usage: py.test tests

import json
from keyvi import compiler
from pytest import raises
from test_tools import tmp_dictionary


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
        assert 9223372036854775 == d.get("a").value
        assert (2**64 - 1) == d.get("b").value
        assert 0 == d.get("c").value


def test_limit_overflow():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    with raises(OverflowError):
        c.add("a", 2**64)


def test_limit_underflow():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb": "10"})
    with raises(AssertionError):
        c.add("a", -1)
