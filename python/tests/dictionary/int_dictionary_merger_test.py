# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
import tempfile
import shutil
import collections
import pytest

from os import path

from keyvi.compiler import IntDictionaryCompiler, IntDictionaryMerger
from keyvi.dictionary import Dictionary

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

key_values_1 = {
    'a': 12,
    'bzzzz': 34,
    'bbb': 34,
    'b': 34
}

key_values_2 = {
    'a': 78,
    'c': 5,
    'i': 5,
    'ia': 5,
    'd': 6
}

key_values_3 = {
    'e': 76,
    'd': 356,
    'e1': 76,
    'e2': 76,
    'e3': 76,
    'e4': 76,
    'f': 9
}


def generate_keyvi(key_values, filename):
    dictionary_compiler = IntDictionaryCompiler({"memory_limit_mb": "10"})
    for key, value in key_values.items():
        dictionary_compiler.Add(key, value)

    dictionary_compiler.Compile()
    dictionary_compiler.WriteToFile(filename)


@pytest.mark.parametrize('merger', [IntDictionaryMerger({"memory_limit_mb": "10"}),
                                    IntDictionaryMerger({"memory_limit_mb": "10", 'merge_mode': 'append'})
                                    ])
def test_merge(merger):
    tmp_dir = tempfile.mkdtemp()
    try:
        file_1 = path.join(tmp_dir, 'test_merger_1.kv')
        file_2 = path.join(tmp_dir, 'test_merger_2.kv')
        file_3 = path.join(tmp_dir, 'test_merger_3.kv')
        merge_file = path.join(tmp_dir, 'merge.kv')

        generate_keyvi(key_values_1, file_1)
        generate_keyvi(key_values_2, file_2)
        generate_keyvi(key_values_3, file_3)

        merger.Add(file_1)
        merger.Add(file_2)
        merger.Add(file_3)
        merger.Merge(merge_file)

        merged_dictionary = Dictionary(merge_file)

        key_values = {}
        key_values.update(key_values_1)
        key_values.update(key_values_2)
        key_values.update(key_values_3)

        key_values_ordered = collections.OrderedDict(sorted(key_values.items()))

        for (base_key, base_value), (keyvi_key, keyvi_value) in zip(key_values_ordered.items(),
                                                                    merged_dictionary.GetAllItems()):
            assert base_key == keyvi_key
            assert base_value == keyvi_value

    finally:
        shutil.rmtree(tmp_dir)
