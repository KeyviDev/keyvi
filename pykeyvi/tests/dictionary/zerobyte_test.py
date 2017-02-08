# -*- coding: utf-8 -*-
# Usage: py.test tests

import contextlib
import os

import pykeyvi

import sys
import os

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary, decode_to_unicode

def test_zerobyte():
    c=pykeyvi.JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("\x00abc", '["a" : 2]')
    c.Add("abc\x00def", '["a" : 3]')
    c.Add("cd\x00", '["a" : 4]')
    with tmp_dictionary(c, 'zerobyte.kv') as d:
        assert decode_to_unicode(d["\x00abc"].GetValue()) == decode_to_unicode('["a" : 2]')
        assert decode_to_unicode(d["abc\x00def"].GetValue()) == decode_to_unicode('["a" : 3]')
        assert decode_to_unicode(d["cd\x00"].GetValue()) == decode_to_unicode('["a" : 4]')
        assert len([(k, v) for k, v in d.GetAllItems()]) == 3
