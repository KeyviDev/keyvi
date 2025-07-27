# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi2 as keyvi
from test_tools import tmp_dictionary
import warnings


from keyvi2.compiler import (
    JsonDictionaryCompiler,
    CompletionDictionaryCompiler,
    KeyOnlyDictionaryCompiler,
    StringDictionaryCompiler,
)


""" def test_serialization():
    m = keyvi.Match()
    m.start = 22
    m.end = 30
    m.score = 42
    d = m.dumps()
    m2 = keyvi.Match.loads(d)
    assert m2.start == 22
    assert m2.end == 30
    assert m2.score == 42 """


""" def test_raw_serialization():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert m.value_as_string() == '{"a":2}'
        d = m.dumps()
        m2 = keyvi.Match.loads(d)
        assert m2.value_as_string() == '{"a":2}'
        with warnings.catch_warnings(record=True) as w:
            warnings.simplefilter("always")
            assert m.GetValueAsString() == '{"a":2}'
            assert len(w) == 1
            assert issubclass(w[-1].category, DeprecationWarning)
 """

""" def test_unicode_attributes():
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
        assert issubclass(w[1].category, DeprecationWarning) """


""" def test_bytes_attributes():
    m = keyvi.Match()
    bytes_key = bytes(u"äöü".encode('utf-8'))
    bytes_value = bytes(u"äöüöäü".encode('utf-8'))
    m[bytes_key] = 22
    assert m[bytes_key] == 22
    m["k2"] = bytes_value
    assert m["k2"] == "äöüöäü"


def test_double_attributes():
    m = keyvi.Match()
    bytes_key = bytes("abc".encode('utf-8'))
    m[bytes_key] = 42.0
    assert m[bytes_key] == 42.0


def test_boolean_attributes():
    m = keyvi.Match()
    bytes_key = bytes("def".encode('utf-8'))
    m[bytes_key] = True
    assert m[bytes_key] == True """


def test_start():
    m = keyvi.Match()
    m.start = 42
    assert m.start == 42


def test_end():
    m = keyvi.Match()
    m.end = 49
    assert m.end == 49


def test_score():
    m = keyvi.Match()
    m.score = 149
    assert m.score == 149


""" def test_get_value():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert m.value == {"a": 2}
        m = d["abd"]
        assert m.value == {"a": 3} """


def test_get_value_int():
    c = CompletionDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("abc", 42)
    c.Add("abd", 21)
    with tmp_dictionary(c, 'match_object_int.kv') as d:
        m = d["abc"]
        assert m.value == 42
        m = d["abd"]
        assert m.value == 21


""" def test_get_value_key_only():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("abc")
    c.Add("abd")
    with tmp_dictionary(c, 'match_object_key_only.kv') as d:
        m = d["abc"]
        assert m.value == ''
        m = d["abd"]
        assert m.value == ''
 """

def test_get_value_string():
    c = StringDictionaryCompiler({"memory_limit_mb": "10"})
    c.Add("abc", "aaaaa")
    c.Add("abd", "bbbbb")
    with tmp_dictionary(c, 'match_object_string.kv') as d:
        m = d["abc"]
        assert m.value == "aaaaa"
        m = d["abd"]
        assert m.value == "bbbbb"


def test_matched_string():
    m = keyvi.Match()
    m.matched_string = "match"
    assert m.matched_string == "match"


def test_bool_operator():
    m = keyvi.Match()
    assert not m
    m.end = 42
    assert not m is False
    assert m
