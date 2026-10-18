# Usage: py.test tests
from __future__ import annotations

import os
import tempfile
import time

from keyvi.index import Index, ReadOnlyIndex


def test_external_merge():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = f"key-{batch * 50 + i:05d}"
                value = f"value-{batch * 50 + i}"
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # verify all keys are still accessible
        for i in range(500):
            key = f"key-{i:05d}"
            assert key in index, f"missing key: {key}"
            match = index[key]
            assert match.value == f"value-{i}"


def test_external_merge_with_deletes():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = f"key-{batch * 50 + i:05d}"
                value = f"value-{batch * 50 + i}"
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # delete every other key
        for i in range(0, 500, 2):
            index.delete(f"key-{i:05d}")
        index.flush()

        # wait for external merge to complete
        time.sleep(5)

        for i in range(500):
            key = f"key-{i:05d}"
            if i % 2 == 0:
                assert key not in index, f"key should be deleted: {key}"
            else:
                assert key in index, f"missing key: {key}"
                match = index[key]
                assert match.value == f"value-{i}"


def test_external_merge_with_overwrite():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        # write initial data across multiple segments
        for batch in range(5):
            key_values = []
            for i in range(50):
                key = f"key-{batch * 50 + i:05d}"
                value = f"value-v1-{batch * 50 + i}"
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # overwrite some keys with new values in new segments
        for batch in range(5):
            key_values = []
            for i in range(50):
                key = f"key-{batch * 50 + i:05d}"
                value = f"value-v2-{batch * 50 + i}"
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # verify that the latest values are returned
        for i in range(250):
            key = f"key-{i:05d}"
            assert key in index, f"missing key: {key}"
            match = index[key]
            assert match.value == f"value-v2-{i}", (
                f"expected v2 value for {key}, got {match.value}"
            )


def test_external_merge_read_only_index():
    with tempfile.TemporaryDirectory() as test_dir:
        index_dir = os.path.join(test_dir, "index")
        index = Index(index_dir, {"segment_external_merge_key_threshold": "100"})

        for batch in range(10):
            key_values = []
            for i in range(50):
                key = f"key-{batch * 50 + i:05d}"
                value = f"value-{batch * 50 + i}"
                key_values.append((key, value))
            index.bulk_set(key_values)
            index.flush()

        # wait for external merge to complete
        time.sleep(5)

        # open a read-only index and verify all data
        reader = ReadOnlyIndex(index_dir)
        for i in range(500):
            key = f"key-{i:05d}"
            assert key in reader, f"missing key in reader: {key}"
