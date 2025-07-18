# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi
import msgpack
from test_tools import tmp_dictionary
import warnings
import zlib
import snappy
import zstd

from keyvi.compiler import (
    JsonDictionaryCompiler,
    CompletionDictionaryCompiler,
    KeyOnlyDictionaryCompiler,
    StringDictionaryCompiler,
)


def test_serialization():
    m = keyvi.Match()
    m.start = 22
    m.end = 30
    m.score = 42
    d = m.dumps()
    m2 = keyvi.Match.loads(d)
    assert m2.start == 22
    assert m2.end == 30
    assert m2.score == 42


def test_raw_serialization():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc", '{"a" : 2}')
    c.add("abd", '{"a" : 3}')
    with tmp_dictionary(c, "match_object_json.kv") as d:
        m = d["abc"]
        assert m.value_as_string() == '{"a":2}'
        d = m.dumps()
        m2 = keyvi.Match.loads(d)
        assert m2.value_as_string() == '{"a":2}'
        assert msgpack.loads(m.msgpacked_value_as_string()) == {"a": 2}
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            assert m.GetValueAsString() == '{"a":2}'
            assert len(w) == 1
            assert issubclass(w[-1].category, DeprecationWarning)


def test_unicode_attributes():
    m = keyvi.Match()
    m["küy"] = 22
    assert m["küy"] == 22
    m["k2"] = " 吃饭了吗"
    m.score = 99
    assert m["k2"] == " 吃饭了吗"
    assert m.score == 99.0
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        m.SetAttribute("k2", "öäü")
        assert m["k2"] == "öäü"
        assert m.GetAttribute("k2") == "öäü"
        assert len(w) == 2
        assert issubclass(w[0].category, DeprecationWarning)
        assert issubclass(w[1].category, DeprecationWarning)


def test_bytes_attributes():
    m = keyvi.Match()
    bytes_key = bytes("äöü".encode("utf-8"))
    bytes_value = bytes("äöüöäü".encode("utf-8"))
    m[bytes_key] = 22
    assert m[bytes_key] == 22
    m["k2"] = bytes_value
    assert m["k2"] == "äöüöäü"


def test_double_attributes():
    m = keyvi.Match()
    bytes_key = bytes("abc".encode("utf-8"))
    m[bytes_key] = 42.0
    assert m[bytes_key] == 42.0


def test_boolean_attributes():
    m = keyvi.Match()
    bytes_key = bytes("def".encode("utf-8"))
    m[bytes_key] = True
    assert m[bytes_key] == True


def test_start():
    m = keyvi.Match()
    m.start = 42
    assert m.start == 42
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        m.SetStart(44)
        assert m.start == 44
        assert len(w) == 1
        assert issubclass(w[-1].category, DeprecationWarning)


def test_end():
    m = keyvi.Match()
    m.end = 49
    assert m.end == 49
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        m.SetEnd(55)
        assert m.end == 55
        assert len(w) == 1
        assert issubclass(w[-1].category, DeprecationWarning)


def test_score():
    m = keyvi.Match()
    m.score = 149
    assert m.score == 149
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        m.SetScore(155)
        assert m.score == 155
        assert len(w) == 1
        assert issubclass(w[-1].category, DeprecationWarning)


def test_get_value():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc", '{"a" : 2}')
    c.add("abd", '{"a" : 3}')
    with tmp_dictionary(c, "match_object_json.kv") as d:
        m = d["abc"]
        assert m.value == {"a": 2}
        m = d["abd"]
        assert m.value == {"a": 3}
        assert msgpack.loads(m.msgpacked_value_as_string()) == {"a": 3}
        assert msgpack.loads(
            zlib.decompress(
                m.msgpacked_value_as_string(keyvi.CompressionAlgorithm.ZLIB_COMPRESSION)
            )
        ) == {"a": 3}
        assert msgpack.loads(
            snappy.decompress(
                m.msgpacked_value_as_string(
                    keyvi.CompressionAlgorithm.SNAPPY_COMPRESSION
                )
            )
        ) == {"a": 3}
        assert msgpack.loads(
            zstd.decompress(
                m.msgpacked_value_as_string(keyvi.CompressionAlgorithm.ZSTD_COMPRESSION)
            )
        ) == {"a": 3}
        assert msgpack.loads(
            m.msgpacked_value_as_string(keyvi.CompressionAlgorithm.NO_COMPRESSION)
        ) == {"a": 3}


def test_get_value_int():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc", 42)
    c.add("abd", 21)
    with tmp_dictionary(c, "match_object_int.kv") as d:
        m = d["abc"]
        assert m.value == 42
        m = d["abd"]
        assert m.value == 21
        assert msgpack.loads(m.msgpacked_value_as_string()) == 21
        assert (
            msgpack.loads(
                zlib.decompress(
                    m.msgpacked_value_as_string(
                        keyvi.CompressionAlgorithm.ZLIB_COMPRESSION
                    )
                )
            )
            == 21
        )


def test_get_value_key_only():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc")
    c.add("abd")
    with tmp_dictionary(c, "match_object_key_only.kv") as d:
        m = d["abc"]
        assert m.value is None
        m = d["abd"]
        assert m.value is None
        assert msgpack.loads(m.msgpacked_value_as_string()) is None
        assert (
            msgpack.loads(
                zlib.decompress(
                    m.msgpacked_value_as_string(
                        keyvi.CompressionAlgorithm.ZLIB_COMPRESSION
                    )
                )
            )
            is None
        )


def test_get_value_string():
    c = StringDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("abc", "aaaaa")
    c.add("abd", "bbbbb")
    c.add("abe", "{}")
    with tmp_dictionary(c, "match_object_string.kv") as d:
        m = d["abc"]
        assert m.value == "aaaaa"
        m = d["abd"]
        assert m.value == "bbbbb"
        assert msgpack.loads(m.msgpacked_value_as_string()) == "bbbbb"
        assert (
            msgpack.loads(
                zlib.decompress(
                    m.msgpacked_value_as_string(
                        keyvi.CompressionAlgorithm.ZLIB_COMPRESSION
                    )
                )
            )
            == "bbbbb"
        )
        m = d["abe"]
        # gh#333: keyvi < 0.6.4 returned a dictionary instead of a string
        assert m.value == "{}"
        assert isinstance(m.value, str)


def test_matched_string():
    m = keyvi.Match()
    m.matched_string = "match"
    assert m.matched_string == "match"
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        m.SetMatchedString("other_match")
        assert m.matched_string == "other_match"
        assert len(w) == 1
        assert issubclass(w[-1].category, DeprecationWarning)


def test_bool_operator():
    m = keyvi.Match()
    with warnings.catch_warnings(record=True) as w:
        warnings.simplefilter("always")
        assert m.IsEmpty()
        assert issubclass(w[-1].category, DeprecationWarning)
    assert not m
    m.end = 42
    assert not m is False
    assert m
