from test_tools import tmp_dictionary
import sys
import os

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../"))


def test_empty_dict_items():
    c = JsonDictionaryCompiler({"memory_limit_mb": "10"})
    with tmp_dictionary(c, "empty.kv") as d:
        assert len([(k, v) for k, v in d.items()]) == 0
