# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

def test_zerobyte():
    c=JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("\x00abc", '["a" : 2]')
    c.Add("abc\x00def", '["a" : 3]')
    c.Add("cd\x00", '["a" : 4]')
    with tmp_dictionary(c, 'zerobyte.kv') as d:
        assert d["\x00abc"].GetValue() == '["a" : 2]'
        assert d["abc\x00def"].GetValue() == '["a" : 3]'
        assert d["cd\x00"].GetValue() == '["a" : 4]'
        assert len([(k, v) for k, v in d.GetAllItems()]) == 3
