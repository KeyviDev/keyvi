# -*- coding: utf-8 -*-
# Usage: py.test tests

import json
import os
import tempfile
import keyvi
from test_tools import tmp_dictionary


def test_manifest():
    c = keyvi.IntDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("Leela", 20)
    c.Add("Kif", 2)
    c.SetManifest('{"drink": "slurm"}')
    with tmp_dictionary(c, 'slurm.kv') as d:
        m = json.loads(d.GetManifest())
        assert m['drink'] == "slurm"
