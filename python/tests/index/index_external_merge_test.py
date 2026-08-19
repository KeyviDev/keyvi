# -*- coding: utf-8 -*-
# Usage: py.test tests

from keyvi.index import Index, ReadOnlyIndex
import os
import tempfile
import time


def test_external_merge():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = "key-{:05d}".format(batch * 50 + i)
                value = "value-{}".format(batch * 50 + i)
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # verify all keys are still accessible
        for i in range(500):
            key = "key-{:05d}".format(i)
            assert key in index, "missing key: {}".format(key)
            match = index[key]
            assert match.value == "value-{}".format(i)



def test_external_merge_with_deletes():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = "key-{:05d}".format(batch * 50 + i)
                value = "value-{}".format(batch * 50 + i)
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # delete every other key
        for i in range(0, 500, 2):
            index.delete("key-{:05d}".format(i))
        index.flush()

        # wait for external merge to complete
        time.sleep(5)

        for i in range(500):
            key = "key-{:05d}".format(i)
            if i % 2 == 0:
                assert key not in index, "key should be deleted: {}".format(key)
            else:
                assert key in index, "missing key: {}".format(key)
                match = index[key]
                assert match.value == "value-{}".format(i)



def test_external_merge_with_overwrite():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        # write initial data across multiple segments
        for batch in range(5):
            key_values = []
            for i in range(50):
                key = "key-{:05d}".format(batch * 50 + i)
                value = "value-v1-{}".format(batch * 50 + i)
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # overwrite some keys with new values in new segments
        for batch in range(5):
            key_values = []
            for i in range(50):
                key = "key-{:05d}".format(batch * 50 + i)
                value = "value-v2-{}".format(batch * 50 + i)
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # verify that the latest values are returned
        for i in range(250):
            key = "key-{:05d}".format(i)
            assert key in index, "missing key: {}".format(key)
            match = index[key]
            assert match.value == "value-v2-{}".format(i), \
                "expected v2 value for {}, got {}".format(key, match.value)



def test_external_merge_read_only_index():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = "key-{:05d}".format(batch * 50 + i)
                value = "value-{}".format(batch * 50 + i)
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # open a read-only index and verify all data
        reader = ReadOnlyIndex(index_dir)
        for i in range(500):
            key = "key-{:05d}".format(i)
            assert key in reader, "missing key in reader: {}".format(key)

