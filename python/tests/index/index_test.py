# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi.index import Index
import os
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
