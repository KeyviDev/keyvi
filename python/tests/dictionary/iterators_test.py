# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
import json

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))

from test_tools import tmp_dictionary

key_values = [
    ('a', {"a": 2}),
    ('b', {"b": 2}),
    ('c', {"c": 2}),
    ('d', {"d": 2})
]

def generate_dictionary_compiler():

    dictionary_compiler = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    for key, value in key_values:
        dictionary_compiler.Add(key, json.dumps(value))

    return dictionary_compiler

def test_get_all_keys():

    with tmp_dictionary(generate_dictionary_compiler(), 'test_get_all_keys.kv') as keyvi_dictionary:
        for (base_key, _), keyvi_key in zip(key_values, keyvi_dictionary.GetAllKeys()):
            assert base_key == keyvi_key


def test_get_all_values():

    with tmp_dictionary(generate_dictionary_compiler(), 'test_get_all_values.kv') as keyvi_dictionary:
        for (_, base_value), keyvi_value in zip(key_values, keyvi_dictionary.GetAllValues()):
            assert base_value == keyvi_value


def test_get_all_items():

    with tmp_dictionary(generate_dictionary_compiler(), 'test_get_all_items.kv') as keyvi_dictionary:
        for (base_key, base_value), (keyvi_key, keyvi_value) in zip(key_values, keyvi_dictionary.GetAllItems()):
            assert base_key == keyvi_key
            assert base_value == keyvi_value
