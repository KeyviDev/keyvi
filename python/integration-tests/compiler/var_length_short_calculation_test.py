# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
import json

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../../tests"))

from test_tools import tmp_dictionary


def test_input_output_keys():
    compiler = JsonDictionaryCompiler({'compression_threshold': '32', 'compression': 'zlib', "memory_limit_mb":"10"})
    input_keys_count = 0
    with open(os.path.join(root, 'var_length_short_calculation_test_data.tsv')) as f_in:
        for line in f_in:
            k, v = line.split('\t')
            key = json.loads(k)
            value = json.loads(v)
            compiler.Add(key, value)
            input_keys_count += 1

    output_keys_count = 0
    with tmp_dictionary(compiler, 'var_length_short_test.kv') as d:
        for _ in d.GetAllItems():
            output_keys_count += 1

    assert input_keys_count == output_keys_count
