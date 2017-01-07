# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi
from test_tools import tmp_dictionary, decode_to_unicode


def test_serialization():
    m = pykeyvi.Match()
    m.SetStart(22)
    m.SetEnd(30)
    d = m.dumps()
    m2 = pykeyvi.Match.loads(d)
    assert m2.GetStart() == 22
    assert m2.GetEnd() == 30


def test_raw_serialization():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert decode_to_unicode(m.GetValueAsString()) == decode_to_unicode('{"a":2}')
        d = m.dumps()
        m2 = pykeyvi.Match.loads(d)
        assert decode_to_unicode(m2.GetValueAsString()) == decode_to_unicode('{"a":2}')


def test_unicode_attributes():
    m = pykeyvi.Match()
    m.SetAttribute(decode_to_unicode("küy"), 22)
    assert m.GetAttribute("küy") == 22
    m.SetAttribute("k2", decode_to_unicode(" 吃饭了吗"))
    assert decode_to_unicode(m.GetAttribute("k2")) == decode_to_unicode(" 吃饭了吗")

def test_bytes_attributes():
    m = pykeyvi.Match()
    bytes_key = bytes(decode_to_unicode("äöü").encode('utf-8'))
    bytes_value = bytes(decode_to_unicode("äöüöäü").encode('utf-8'))
    m.SetAttribute(bytes_key, 22)
    assert m.GetAttribute(bytes_key) == 22
    m.SetAttribute("k2", bytes_value)
    assert decode_to_unicode(m.GetAttribute("k2")) == decode_to_unicode("äöüöäü")

def test_get_value():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode({"a":2})
        m = d["abd"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode({"a":3})

def test_get_value_int():
    c = pykeyvi.CompletionDictionaryCompiler()
    c.Add("abc", 42)
    c.Add("abd", 21)
    with tmp_dictionary(c, 'match_object_int.kv') as d:
        m = d["abc"]
        assert m.GetValue() == 42
        m = d["abd"]
        assert m.GetValue() == 21

def test_get_value_key_only():
    c = pykeyvi.KeyOnlyDictionaryCompiler()
    c.Add("abc")
    c.Add("abd")
    with tmp_dictionary(c, 'match_object_key_only.kv') as d:
        m = d["abc"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode('')
        m = d["abd"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode('')

def test_get_value_string():
    c = pykeyvi.StringDictionaryCompiler()
    c.Add("abc", "aaaaa")
    c.Add("abd", "bbbbb")
    with tmp_dictionary(c, 'match_object_string.kv') as d:
        m = d["abc"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode("aaaaa")
        m = d["abd"]
        assert decode_to_unicode(m.GetValue()) == decode_to_unicode("bbbbb")
