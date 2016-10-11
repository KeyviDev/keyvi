# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

def test_unicode():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("öäü", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'

    # create unicode string
    key = "öäü".decode('utf-8')
    with tmp_dictionary(c, 'unicode_json.kv') as d:
        assert key in d
        assert d[key].GetValue() == {"a" : 2}
        assert d.get(key).GetValue() == {"a" : 2}

def test_unicode_lookup():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("Los Angeles", '{"country" : "USA"}')
    c.Add("Frankfurt am Main", '{"country" : "Germany"}')
    c.Add("Kirchheim bei München".decode('utf-8'), '{"country" : "Germany"}')

    # create unicode string for lookup
    text = "From Los Angeles via Frankfurt am Main to Kirchheim bei München it should just work".decode('utf-8')
    with tmp_dictionary(c, 'unicode_json_lookup.kv') as d:
        assert "Kirchheim bei München" in d
        matched_strings = [x.GetMatchedString() for x in d.LookupText(text)]
        assert len(matched_strings) == 3
        assert "Kirchheim bei München" in matched_strings
        assert "Los Angeles" in matched_strings
        assert "Frankfurt am Main" in matched_strings
