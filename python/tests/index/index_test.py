# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi.index import Index, ReadOnlyIndex
import os
import random
import shutil
import tempfile
import gc


def test_open_index():
    test_dir = os.path.join(tempfile.gettempdir(), "index_open_index")
    try:
        if not os.path.exists(test_dir):
            os.mkdir(test_dir)
        index = Index(os.path.join(test_dir, "index"))
        index.Set("a", "{}")
        del index
        # required for pypy to ensure deletion/destruction of the index object
        gc.collect()
        index = Index(os.path.join(test_dir, "index"))
        assert "a" in index
        del index
    finally:
        shutil.rmtree(test_dir, ignore_errors=True)


def test_some_indexing():
    test_dir = os.path.join(tempfile.gettempdir(), "index_some_indexing")
    iterations = 10000
    split = 2000
    try:
        if not os.path.exists(test_dir):
            os.mkdir(test_dir)
        index = Index(os.path.join(test_dir, "index"))
        for i in range (0, iterations):
            index.Set("key-{}".format(i), "value-{}".format(i))
        index.Flush()
        for i in range (split, iterations):
            assert "key-{}".format(i) in index
            index.Delete("key-{}".format(i))
        index.Flush()

        for i in range (0, split):
            assert "key-{}".format(i) in index

        for i in range (split, iterations):
            assert not "key-{}".format(i) in index
        del index
    finally:
        shutil.rmtree(test_dir, ignore_errors=True)


def test_bulk_add():
    test_dir = os.path.join(tempfile.gettempdir(), "index_bulk_add")
    iterations = 10
    chunk_size = 1000
    try:
        if not os.path.exists(test_dir):
            os.mkdir(test_dir)
        index = Index(os.path.join(test_dir, "index"))
        key_values = []

        for i in range (0, chunk_size * iterations):
            key_values.append(("key-{}".format(i), "value-{}".format(i)))
            if i % chunk_size == 0:
                index.MSet(key_values)
                key_values = []
        index.MSet(key_values)
        index.Flush()

        for i in range(0, 50):
            assert "key-{}".format(random.randrange(0, chunk_size * iterations)) in index
        del index
    finally:
        shutil.rmtree(test_dir, ignore_errors=True)


def test_get_fuzzy():
    test_dir = os.path.join(tempfile.gettempdir(), "index_test_fuzzy")
    try:
        if not os.path.exists(test_dir):
            os.mkdir(test_dir)
        write_index = Index(os.path.join(test_dir, "index"))
        write_index.Set("apple", "{}")
        write_index.Set("apples", "{}")
        write_index.Set("banana", "{}")
        write_index.Set("orange", "{}")
        write_index.Set("avocado", "{}")
        write_index.Set("peach", "{}")
        write_index.Flush()

        read_only_index = ReadOnlyIndex(os.path.join(test_dir, "index"))

        for index in [write_index, read_only_index]:
            matches = list(index.GetFuzzy("appe", 1, 2))
            assert len(matches) == 1
            assert  u'apple' == matches[0].GetMatchedString()

            matches = list(index.GetFuzzy("appes", 2, 2))
            assert len(matches) == 2
            assert  u'apple' == matches[0].GetMatchedString()
            assert  u'apples' == matches[1].GetMatchedString()
            matches = list(index.GetFuzzy("apples", 1, 2))
            assert len(matches) == 2
            assert  u'apple' == matches[0].GetMatchedString()
            assert  u'apples' == matches[1].GetMatchedString()
            matches = list(index.GetFuzzy("atocao", 2, 1))
            assert len(matches) == 1
            assert  u'avocado' == matches[0].GetMatchedString()

        write_index.Delete("avocado")
        write_index.Flush()
        matches = list(write_index.GetFuzzy("atocao", 2, 1))
        assert len(matches) == 0

        del write_index
        del read_only_index
    finally:
        shutil.rmtree(test_dir, ignore_errors=True)

def test_get_near():
    test_dir = os.path.join(tempfile.gettempdir(), "index_test_near")
    try:
        if not os.path.exists(test_dir):
            os.mkdir(test_dir)
        write_index = Index(os.path.join(test_dir, "index"))
        # the following geohashes are created from openstreetmap coordinates and translated using a geohash encoder
        write_index.Set("u21xj502gs79", "{'city' : 'Kobarid', 'country': 'si'}")
        write_index.Set("u21xk2uxkhh2", "{'city' : 'Trnovo ob soci', 'country': 'si'}")
        write_index.Set("u21x75n34qrp", "{'city' : 'Srpnecia', 'country': 'si'}")
        write_index.Set("u21x6v1nx0c3", "{'city' : 'Zaga', 'country': 'si'}")
        write_index.Set("u21xs20w9ssu", "{'city' : 'Cezsoca', 'country': 'si'}")
        write_index.Set("u21x6yx5cqy6", "{'city' : 'Log Cezsoski', 'country': 'si'}")
        write_index.Set("u21xs7ses4s3", "{'city' : 'Bovec', 'country': 'si'}")
        write_index.Flush()

        read_only_index = ReadOnlyIndex(os.path.join(test_dir, "index"))

        for index in [write_index, read_only_index]:
            # some coordinate nearby, greedy false, so it prefers as close as possible
            matches = list(index.GetNear("u21xjjhhymt7", 4))
            assert len(matches) == 1
            assert  u'u21xj502gs79' == matches[0].GetMatchedString()
            assert  u"{'city' : 'Kobarid', 'country': 'si'}" == matches[0].GetValue()

            # greedy match, still closest should be the 1st match
            matches = list(index.GetNear("u21xjjhhymt7", 4, True))
            assert len(matches) == 7
            assert  u'u21xj502gs79' == matches[0].GetMatchedString()
            assert  u"{'city' : 'Kobarid', 'country': 'si'}" == matches[0].GetValue()

            # closer match near Bovec and Cezsoca but closer to Cezsoca
            matches = list(index.GetNear("u21xs20w9ssu", 5))
            assert len(matches) == 1
            assert  u'u21xs20w9ssu' == matches[0].GetMatchedString()
            assert  u"{'city' : 'Cezsoca', 'country': 'si'}" == matches[0].GetValue()

            # greedy should return Bovec, but not the other locations due to the prefix
            matches = list(index.GetNear("u21xs20w9ssu", 5, True))
            assert len(matches) == 2
            assert  u'u21xs20w9ssu' == matches[0].GetMatchedString()
            assert  u"{'city' : 'Cezsoca', 'country': 'si'}" == matches[0].GetValue()
            assert  u'u21xs7ses4s3' == matches[1].GetMatchedString()
            assert  u"{'city' : 'Bovec', 'country': 'si'}" == matches[1].GetValue()

        del write_index
        del read_only_index
    finally:
        shutil.rmtree(test_dir, ignore_errors=True)
