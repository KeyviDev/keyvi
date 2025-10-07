# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
from pytest import raises

import keyvi.vector


def test_basic_json_test():
    generator = keyvi.vector.JsonVectorGenerator()

    size = 10000

    for i in range(size):
        generator.append([i, i + 1])

    generator.write_to_file('vector_json_basic_test.kv')

    vector = keyvi.vector.JsonVector('vector_json_basic_test.kv')

    assert size == len(vector)

    for i in range(size):
        assert [i, i + 1] == vector[i]

    os.remove('vector_json_basic_test.kv')


def test_basic_string_test():
    generator = keyvi.vector.StringVectorGenerator()

    size = 10000

    for i in range(size):
        generator.append(str(i))

    generator.write_to_file('vector_string_basic_test.kv')

    vector = keyvi.vector.StringVector('vector_string_basic_test.kv')

    assert size == len(vector)

    for i in range(size):
        assert str(i) == vector[i]

    os.remove('vector_string_basic_test.kv')


def test_basic_manifest():
    generator = keyvi.vector.StringVectorGenerator()
    generator.set_manifest('manifest')
    generator.write_to_file('vector_manifest.kv')

    vector = keyvi.vector.StringVector('vector_manifest.kv')
    assert 'manifest' == vector.manifest()

    os.remove('vector_manifest.kv')


def test_basic_write_to_invalid_file():
    generator = keyvi.vector.StringVectorGenerator()
    generator.set_manifest('manifest')
    with raises(ValueError):
        generator.write_to_file(os.path.join("invalid", "sub", "directory", "file.kv"))
