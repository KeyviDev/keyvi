# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
from pytest import raises

import keyvi.vector


def test_basic_json_test():
    generator = keyvi.vector.JsonVectorGenerator()

    size = 10000

    for i in range(size):
        generator.PushBack([i, i + 1])

    generator.WriteToFile('vector_json_basic_test.kv')

    vector = keyvi.vector.JsonVector('vector_json_basic_test.kv')

    assert size == vector.Size()

    for i in range(size):
        assert [i, i + 1] == vector.Get(i)

    os.remove('vector_json_basic_test.kv')


def test_basic_string_test():
    generator = keyvi.vector.StringVectorGenerator()

    size = 10000

    for i in range(size):
        generator.PushBack(str(i))

    generator.WriteToFile('vector_string_basic_test.kv')

    vector = keyvi.vector.StringVector('vector_string_basic_test.kv')

    assert size == vector.Size()

    for i in range(size):
        assert str(i) == vector.Get(i)

    os.remove('vector_string_basic_test.kv')


def test_basic_manifest():
    generator = keyvi.vector.StringVectorGenerator()
    generator.SetManifest('manifest')
    generator.WriteToFile('vector_manifest.kv')

    vector = keyvi.vector.StringVector('vector_manifest.kv')
    assert 'manifest' == vector.Manifest()

    os.remove('vector_manifest.kv')


def test_basic_write_to_invalid_file():
    generator = keyvi.vector.StringVectorGenerator()
    generator.SetManifest('manifest')
    with raises(ValueError):
        generator.WriteToFile(os.path.join("invalid", "sub", "directory", "file.kv"))
