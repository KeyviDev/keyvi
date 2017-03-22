# -*- coding: utf-8 -*-
# Usage: py.test tests

import os
import tempfile
import pykeyvi
from test_tools import tmp_dictionary


def test_manifest():
    c = pykeyvi.IntDictionaryCompiler({"memory_limit_mb":"10"})
    c.Add("Leela", 20)
    c.Add("Kif", 2)
    c.SetManifest({"drink": "slurm"})
    with tmp_dictionary(c, 'slurm.kv') as d:
        m = d.GetManifest()
        assert m['drink'] == "slurm"
