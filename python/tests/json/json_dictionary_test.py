# -*- coding: utf-8 -*-
# Usage: py.test tests
from test_tools import tmp_dictionary
import sys
import os
import pytest

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))


def test_simple():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc", '{"a" : 2}')
    c.add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'
    with tmp_dictionary(c, "simple_json.kv") as d:
        assert len(d) == 2
        assert d["abc"].value_as_string() == '{"a":2}'
        assert d["abd"].value_as_string() == '{"a":3}'


@pytest.mark.parametrize("compression", ["zlib", "snappy", "zstd"])
def test_simple_compression(compression: str):
    c = JsonDictionaryCompiler(
        {
            "memory_limit_mb": "10",
            "compression": compression,
            "compression_threshold": "0",
        }
    )
    c.add("abc", '{"a" : 2}')
    c.add("abd", '{"a" : 3}')
    with tmp_dictionary(c, f"simple_json_{compression}.kv") as d:
        assert len(d) == 2
        assert d["abc"].value_as_string() == '{"a":2}'
        assert d["abd"].value_as_string() == '{"a":3}'
        m = d.statistics()["Value Store"]
        assert m["__compression"] == compression


def test_unicode_compile():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("üöä", '{"y" : 2}')
    c.add("üüüüüüabd", '{"a" : 3}')
    c.add("ääääädäd", '{"b" : 33}')

    with tmp_dictionary(c, "simple_json.kv") as d:
        assert len(d) == 3
        assert d["üöä"].value_as_string() == '{"y":2}'
        assert d["üöä"].value_as_string() == '{"y":2}'
        assert d["üüüüüüabd"].value_as_string() == '{"a":3}'
        assert d["ääääädäd"].value_as_string() == '{"b":33}'


def test_float_compaction():
    cs = JsonDictionaryCompiler(
        {"memory_limit_mb": "10", "floating_point_precision": "single"}
    )
    cd = JsonDictionaryCompiler({"memory_limit_mb": "10"})

    # add a couple of floats to both
    cs.add(
        "aa",
        "[1.7008715758978892, 1.8094465532317732, 1.6098250864350536, 1.6369107966501981, 1.7736887965234107, 1.606682751740542, 1.6186427703265525, 1.7939763843449683, 1.5973550162469434, 1.6799721708726192, 1.8199786239525833, 1.7956178070065245, 1.7269879953863045]",
    )
    cd.add(
        "aa",
        "[1.7008715758978892, 1.8094465532317732, 1.6098250864350536, 1.6369107966501981, 1.7736887965234107, 1.606682751740542, 1.6186427703265525, 1.7939763843449683, 1.5973550162469434, 1.6799721708726192, 1.8199786239525833, 1.7956178070065245, 1.7269879953863045]",
    )

    with tmp_dictionary(cs, "json_single_precision_float.kv") as ds:
        with tmp_dictionary(cd, "json_double_precision_float.kv") as dd:
            # first some basic checks
            assert len(ds) == 1
            assert len(dd) == 1
            # simple test the length of the value store which shall be smaller for single floats
            stats_s = ds.statistics()
            stats_d = dd.statistics()
            assert int(stats_s["Value Store"]["size"]) < int(
                stats_d["Value Store"]["size"]
            )
