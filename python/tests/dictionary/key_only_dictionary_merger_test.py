# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
import tempfile
import shutil
import pytest

from os import path

from keyvi.compiler import KeyOnlyDictionaryMerger, KeyOnlyDictionaryCompiler
from keyvi.dictionary import Dictionary

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

keys_1 = {
    'a',
    'bzzzz',
    'bbb',
    'b'
}

keys_2 = {
    'a',
    'c',
    'i',
    'ia',
    'd'
}

keys_3 = {
    'e',
    'd',
    'e1',
    'e2',
    'e3',
    'e4',
    'f'
}


def generate_keyvi(key_values, filename):
    dictionary_compiler = KeyOnlyDictionaryCompiler({"memory_limit_mb": "10"})
    for key in key_values:
        dictionary_compiler.add(key)

    dictionary_compiler.compile()
    dictionary_compiler.write_to_file(filename)


@pytest.mark.parametrize('merger', [KeyOnlyDictionaryMerger({"memory_limit_mb": "10"}),
                                    KeyOnlyDictionaryMerger({"memory_limit_mb": "10", 'merge_mode': 'append'})])
def test_merge(merger):
    tmp_dir = tempfile.mkdtemp()
    try:
        file_1 = path.join(tmp_dir, 'test_merger_1.kv')
        file_2 = path.join(tmp_dir, 'test_merger_2.kv')
        file_3 = path.join(tmp_dir, 'test_merger_3.kv')
        merge_file = path.join(tmp_dir, 'merge.kv')

        generate_keyvi(keys_1, file_1)
        generate_keyvi(keys_2, file_2)
        generate_keyvi(keys_3, file_3)

        merger.add(file_1)
        merger.add(file_2)
        merger.add(file_3)
        merger.merge(merge_file)

        merged_dictionary = Dictionary(merge_file)

        keys = set()
        keys.update(keys_1)
        keys.update(keys_2)
        keys.update(keys_3)

        keys_ordered = sorted(keys)

        for base_key, keyvi_key in zip(keys_ordered, merged_dictionary.keys()):
            assert base_key == keyvi_key

    finally:
        shutil.rmtree(tmp_dir)
