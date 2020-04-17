# -*- coding: utf-8 -*-
# Usage: py.test tests

import sys
import os
import gc

from keyvi.compiler import JsonDictionaryCompiler

root = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(root, "../../tests"))

from test_tools import tmp_dictionary

def memory_usage_ps():
    import subprocess
    out = subprocess.Popen(['ps', 'v', '-p', str(os.getpid())],
    stdout=subprocess.PIPE).communicate()[0].split(b'\n')
    vsz_index = out[0].split().index(b'RSS')
    mem = float(out[1].split()[vsz_index])
    return mem

def test_leak():
    c = JsonDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("something", '["a" : 2]')

    with tmp_dictionary(c, 'near_simple.kv') as d:
        gc.collect()
        memory_usage_on_start = memory_usage_ps()
        for i in range(0, 500000):
            assert not d.get('something_else')
            if i % 100 == 0:
                gc.collect()
                memory_usage_now = memory_usage_ps()
                assert memory_usage_now < memory_usage_on_start + 15000
