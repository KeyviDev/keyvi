# -*- coding: utf-8 -*-
# Usage: py.test tests

from test_tools import tmp_dictionary
import sys
import os

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))


def test_unicode():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("öäü", '{"a" : 2}')
    c.add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'

    # create unicode string
    key = "öäü"
    with tmp_dictionary(c, 'unicode_json.kv') as d:
        assert key in d
        assert d[key].value == {"a": 2}
        assert d.get(key).value == {"a": 2}


def test_unicode_lookup():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    c.add("Los Angeles", '{"country" : "USA"}')
    c.add("Frankfurt am Main", '{"country" : "Germany"}')
    c.add("Kirchheim bei München", '{"country" : "Germany"}')

    # create unicode string for lookup
    text = "From Los Angeles via Frankfurt am Main to Kirchheim bei München it should just work"
    with tmp_dictionary(c, 'unicode_json_lookup.kv') as d:
        assert "Kirchheim bei München" in d
        matched_strings = [x.matched_string for x in d.search_tokenized(text)]
        assert len(matched_strings) == 3
        assert u"Kirchheim bei München" in matched_strings
        assert u"Los Angeles" in matched_strings
        assert u"Frankfurt am Main" in matched_strings
