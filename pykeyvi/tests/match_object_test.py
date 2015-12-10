# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi
from test_tools import tmp_dictionary

'''
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

'''
