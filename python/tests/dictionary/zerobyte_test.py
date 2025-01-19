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
    c.add("\x00abc", '["a" : 2]')
    c.add("abc\x00def", '["a" : 3]')
    c.add("cd\x00", '["a" : 4]')
    with tmp_dictionary(c, 'zerobyte.kv') as d:
        assert d["\x00abc"].value == '["a" : 2]'
        assert d["abc\x00def"].value == '["a" : 3]'
        assert d["cd\x00"].value == '["a" : 4]'
        assert len([(k, v) for k, v in d.items()]) == 3
