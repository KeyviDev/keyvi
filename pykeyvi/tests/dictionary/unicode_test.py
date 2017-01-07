# -*- coding: utf-8 -*-
# Usage: py.test tests

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary, decode_to_unicode

def test_unicode():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("öäü", '{"a" : 2}')
    c.Add("abd", '{"a" : 3}')
    # use python syntax ala __setitem__
    c["abd"] = '{"a" : 3}'

    # create unicode string
    key = decode_to_unicode("öäü")
    with tmp_dictionary(c, 'unicode_json.kv') as d:
        assert key in d
        assert decode_to_unicode(d[key].GetValue()) == decode_to_unicode({"a" : 2})
        assert decode_to_unicode(d.get(key).GetValue()) == decode_to_unicode({"a" : 2})

def test_unicode_lookup():
    c = pykeyvi.JsonDictionaryCompiler()
    c.Add("Los Angeles", '{"country" : "USA"}')
    c.Add("Frankfurt am Main", '{"country" : "Germany"}')
    c.Add(decode_to_unicode("Kirchheim bei München"), '{"country" : "Germany"}')

    # create unicode string for lookup
    text = decode_to_unicode("From Los Angeles via Frankfurt am Main to Kirchheim bei München it should just work")
    with tmp_dictionary(c, 'unicode_json_lookup.kv') as d:
        assert "Kirchheim bei München" in d
        matched_strings = [x.GetMatchedString() for x in d.LookupText(text)]
        assert len(matched_strings) == 3
        assert decode_to_unicode("Kirchheim bei München") in decode_to_unicode(matched_strings)
        assert decode_to_unicode("Los Angeles") in decode_to_unicode(matched_strings)
        assert decode_to_unicode("Frankfurt am Main") in decode_to_unicode(matched_strings)
