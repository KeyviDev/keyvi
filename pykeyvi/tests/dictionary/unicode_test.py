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