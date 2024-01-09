# -*- coding: utf-8 -*-
# Usage: py.test tests
import sys
import os

from keyvi.compiler import FloatVectorDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary


def test_simple():
    c = FloatVectorDictionaryCompiler({"memory_limit_mb":"10", "vector_size": "8"})
    c.Add("abc", [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8])
    c.Add("abd", [1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8])

    with tmp_dictionary(c, 'simple_float_vector.kv') as d:
        assert len(d) == 2
        assert d["abc"].GetValueAsString() == '0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8'
        assert d["abd"].GetValueAsString() == '1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8'

