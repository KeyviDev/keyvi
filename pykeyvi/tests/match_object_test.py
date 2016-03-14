# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi
from test_tools import tmp_dictionary


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
        assert m.GetValueAsString() == '{"a":2}'
        d = m.dumps()
        m2 = pykeyvi.Match.loads(d)
        assert m2.GetValueAsString() == '{"a":2}'


def test_unicode_attributes():
    m = pykeyvi.Match()
    m.SetAttribute("küy".decode("utf-8"), 22)
    assert m.GetAttribute("küy") == 22
    m.SetAttribute("k2", " 吃饭了吗".decode("utf-8"))
    assert m.GetAttribute("k2") == " 吃饭了吗"

def test_get_value():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("abc", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    with tmp_dictionary(c, 'match_object_json.kv') as d:
        m = d["abc"]
        assert m.GetValue() == {"a":2}
        m = d["abd"]
        assert m.GetValue() == {"a":3}

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
        assert m.GetValue() == ''
        m = d["abd"]
        assert m.GetValue() == ''

def test_get_value_string():
    c = pykeyvi.StringDictionaryCompiler()
    c.Add("abc", "aaaaa")
    c.Add("abd", "bbbbb")
    with tmp_dictionary(c, 'match_object_string.kv') as d:
        m = d["abc"]
        assert m.GetValue() == "aaaaa"
        m = d["abd"]
        assert m.GetValue() == "bbbbb"
