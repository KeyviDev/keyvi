# -*- coding: utf-8 -*-
# Usage: py.test tests

import json
from keyvi import compiler
from test_tools import tmp_dictionary


def test_manifest():
    c = compiler.IntDictionaryCompiler({"memory_limit_mb":"10"})
    c.add("Leela", 20)
    c["Kif"] = 2
    c.set_manifest('{"drink": "slurm"}')
    with tmp_dictionary(c, 'slurm.kv') as d:
        m = json.loads(d.manifest())
        assert m['drink'] == "slurm"
