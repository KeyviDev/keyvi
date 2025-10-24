# -*- coding: utf-8 -*-
# Usage: py.test tests

import contextlib
import os
import tempfile
from keyvi.dictionary import SecondaryKeyDictionary
from keyvi.compiler import (
    SecondaryKeyCompletionDictionaryCompiler,
    SecondaryKeyJsonDictionaryCompiler,
)


@contextlib.contextmanager
def tmp_secondary_key_dictionary(compiler, file_name):
    tmp_dir = tempfile.gettempdir()
    fq_file_name = os.path.join(tmp_dir, file_name)
    compiler.compile()
    compiler.write_to_file(fq_file_name)
    del compiler
    d = SecondaryKeyDictionary(fq_file_name)
    yield d
    del d
    os.remove(fq_file_name)


def test_completion():
    c = SecondaryKeyCompletionDictionaryCompiler(["user_id"], {"memory_limit_mb": "10"})

    c.add("my_completion", {"user_id": "1"}, 10)
    c.add("my_completion", {"user_id": "2"}, 20)
    c.add("my_completion", {"user_id": "3"}, 30)
    c.add("my_completion", {"user_id": "4"}, 40)

    c.add("my_other_completion", {"user_id": "1"}, 100)
    c.add("my_other_completion", {"user_id": "3"}, 200)
    with tmp_secondary_key_dictionary(c, "secondary_key_completion.kv") as d:
        assert len(d) == 6
        assert [m.value for m in d.complete_prefix("my", {"user_id": "3"})] == [200, 30]
        assert [m.value for m in d.complete_prefix("my", {"user_id": "2"})] == [20]

        # ensure no accidental prefix hit
        for i in range(0, 255):
            assert [m.value for m in d.complete_prefix(chr(i), {"user_id": ""})] == []


def test_json():
    c = SecondaryKeyJsonDictionaryCompiler(["user_id"], {"memory_limit_mb": "10"})

    c.add("my_key", {"user_id": "1"}, '{"a" : 2}')
    c.add("my_key", {"user_id": "2"}, '{"a" : 3}')
    c.add("my_key", {"user_id": "3"}, '{"a" : 4}')
    c.add("my_key", {"user_id": "4"}, '{"a" : 5}')
    c.add("my_key", {"user_id": ""}, '{"a" : 6}')
    c.add("my_other_key", {"user_id": "1"}, '{"a" : 15}')
    c.add("my_other_key", {"user_id": "3"}, '{"a" : 16}')

    with tmp_secondary_key_dictionary(c, "secondary_key_json.kv") as d:
        assert len(d) == 7

        assert d.get("my_key", {"user_id": "1"}).value == {"a": 2}
        assert d.get("my_key", {"user_id": "2"}).value == {"a": 3}
        assert d.get("my_key", {"user_id": "3"}).value == {"a": 4}
        assert d.get("my_key", {"user_id": ""}).value == {"a": 6}
        assert d.get("my_other_key", {"user_id": "1"}).value == {"a": 15}
        assert d.get("my_other_key", {"user_id": "2"}) == None
        assert [m for m in d.complete_prefix("user", {"user_id": "3"})] == []
