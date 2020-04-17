# -*- coding: utf-8 -*-
# Usage: py.test tests

import keyvi
from test_tools import tmp_dictionary

from keyvi.compiler import (
    JsonDictionaryCompiler,
    CompletionDictionaryCompiler,
    KeyOnlyDictionaryCompiler,
    StringDictionaryCompiler,
)

def test_serialization():
    m = keyvi.Match()
    m.SetStart(22)
    m.SetEnd(30)
    d = m.dumps()
    m2 = keyvi.Match.loads(d)
    assert m2.GetStart() == 22
    assert m2.GetEnd() == 30

def test_raw_serialization():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert m.GetValueAsString() == '{"a":2}'
        d = m.dumps()
        m2 = keyvi.Match.loads(d)
        assert m2.GetValueAsString() == '{"a":2}'

def test_unicode_attributes():
    m = keyvi.Match()
    m.SetAttribute("küy", 22)
    assert m.GetAttribute("küy") == 22
    m.SetAttribute("k2", " 吃饭了吗")
    m.SetScore(99)
    assert m.GetAttribute("k2") == " 吃饭了吗"
    assert m.GetScore() == 99.0

def test_bytes_attributes():
    m = keyvi.Match()
    bytes_key = bytes(u"äöü".encode('utf-8'))
    bytes_value = bytes(u"äöüöäü".encode('utf-8'))
    m.SetAttribute(bytes_key, 22)
    assert m.GetAttribute(bytes_key) == 22
    m.SetAttribute("k2", bytes_value)
    assert m.GetAttribute("k2") == "äöüöäü"

def test_double_attributes():
    m = keyvi.Match()
    bytes_key = bytes("abc".encode('utf-8'))
    m.SetAttribute(bytes_key, 42.0)
    assert m.GetAttribute(bytes_key) == 42.0

def test_boolean_attributes():
    m = keyvi.Match()
    bytes_key = bytes("def".encode('utf-8'))
    m.SetAttribute(bytes_key, True)
    assert m.GetAttribute(bytes_key) == True

def test_get_value():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert m.GetValue() == {"a":2}
        m = d["abd"]
        assert m.GetValue() == {"a":3}

def test_get_value_int():
    c = CompletionDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc", 42)
    c.Add("abd", 21)
    with tmp_dictionary(c, 'match_object_int.kv') as d:
        m = d["abc"]
        assert m.GetValue() == 42
        m = d["abd"]
        assert m.GetValue() == 21

def test_get_value_key_only():
    c = KeyOnlyDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc")
    c.Add("abd")
    with tmp_dictionary(c, 'match_object_key_only.kv') as d:
        m = d["abc"]
        assert m.GetValue() == ''
        m = d["abd"]
        assert m.GetValue() == ''

def test_get_value_string():
    c = StringDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("abc", "aaaaa")
    c.Add("abd", "bbbbb")
    with tmp_dictionary(c, 'match_object_string.kv') as d:
        m = d["abc"]
        assert m.GetValue() == "aaaaa"
        m = d["abd"]
        assert m.GetValue() == "bbbbb"
